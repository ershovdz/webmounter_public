/* Copyright (c) 2013, Alexander Ershov
 *
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 * Contact e-mail: Alexander Ershov <ershav@yandex.ru>
 */

#include <fuse.h>
#include "linux_lvfs_driver.h"

#include "webmounter.h"
#include <QString>
#include <sys/time.h>
#include "notification_device.h"


namespace LocalDriver
{

	using namespace Common;
	using namespace Data;

	#define RETURN_ERRNO(x) (x) == 0 ? 0 : -errno

	QString LVFSDriver::_rootDirectory;
	QString LVFSDriver::_mountPoint;

	static bool g_UseStdErr = false;
	static bool g_DebugMode = false;

	LVFSDriver* LVFSDriver::_pDriverInstance = NULL;

	QMutex LVFSDriver::_DriverMutex;
	FileProxy* LVFSDriver::_pFileProxy = NULL;
	struct fuse* LVFSDriver::_pFuseObj = NULL;
	const char* LVFSDriver::_root = NULL;
	LVFSDriver::Operations LVFSDriver::_curOperation = eNONE;

	LVFSDriver::~LVFSDriver(void)
	{
		_notificationDevice = NULL;
		stopDriver();
	}

	LVFSDriver::LVFSDriver(FileProxy * pProxy)
	{
		_pFileProxy = pProxy;
	}

	void LVFSDriver::mount(Data::GeneralSettings& generalSettings)
	{
		if( !isRunning() )
		{
			_rootDirectory = generalSettings.appStoragePath;
			_mountPoint = QDir::homePath() + QDir::separator() + QString::fromUtf8("WebMounter") + QDir::separator();
			QDir dir;
			dir.mkpath(_mountPoint);

			startDriver();
		}
	}

	void LVFSDriver::unmount()
	{
		stopDriver();
	}


	void LVFSDriver::setRootDir(const char *path)
	{
		printf("setting FS root to: %s\n", path);
		_pDriverInstance->_root = path;
	}

	int LVFSDriver::fuseGetAttr(const char *path, struct stat *statbuf)
	{
		char fullPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);
		printf("getattr(%s)\n", fullPath);
		return RETURN_ERRNO(lstat(fullPath, statbuf));
	}

	int LVFSDriver::fuseReadLink(const char *path, char *link, size_t size)
	{
		printf("readlink(path=%s, link=%s, size=%d)\n", path, link, (int)size);
		char fullPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);
		return RETURN_ERRNO(readlink(fullPath, link, size));
	}

	int LVFSDriver::fuseMknod(const char *path, mode_t mode, dev_t dev)
	{
		printf("mknod(path=%s, mode=%d)\n", path, mode);
		char fullPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);
		//handles creating FIFOs, regular files, etc...
		int res =  mknod(fullPath, mode, dev);
		if(!res)
		{
			_curOperation = eCreation;
		}
		return RETURN_ERRNO(res);
	}

	int LVFSDriver::fuseMkdir(const char *path, mode_t mode)
	{
		printf("**mkdir(path=%s, mode=%d)\n", path, (int)mode);
		char fullPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);

		if(_pDriverInstance->_pFileProxy->CreateDirectoryW(QString::fromUtf8(fullPath)))
		{
			return -1;
		}

		return RETURN_ERRNO(mkdir(fullPath, mode));
	}

	int LVFSDriver::fuseUnlink(const char *path)
	{
		printf("unlink(path=%s\n)", path);
		char fullPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);
                if(_pFileProxy->RemoveFile(QString::fromUtf8(fullPath)) != eNO_ERROR)
		{
			return -1;
		}
		return RETURN_ERRNO(unlink(fullPath));
	}

	int LVFSDriver::fuseRmdir(const char *path)
	{
		printf("rmkdir(path=%s\n)", path);
		char fullPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);
		QString pathQ(QString::fromUtf8(fullPath));
		if(_pDriverInstance->_pFileProxy->RemoveDir(pathQ))
		{
			return -1;
		}
		return RETURN_ERRNO(rmdir(fullPath));
	}

	int LVFSDriver::fuseSymlink(const char *path, const char *link)
	{
		printf("symlink(path=%s, link=%s)\n", path, link);
		char fullPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);
		return RETURN_ERRNO(symlink(fullPath, link));
	}

	int LVFSDriver::fuseRename(const char *path, const char *newpath)
	{
		printf("rename(path=%s, newPath=%s)\n", path, newpath);
		char fullPath[PATH_MAX];
		char newFullPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);
		_pDriverInstance->absPath(newFullPath, newpath);

		if(_pFileProxy->MoveElement(QString::fromUtf8(fullPath), QString::fromUtf8(newFullPath)))
		{
			return -1;
		}
		return RETURN_ERRNO(rename(fullPath, newFullPath));
	}

	int LVFSDriver::fuseLink(const char *path, const char *newpath)
	{
		printf("link(path=%s, newPath=%s)\n", path, newpath);
		char fullPath[PATH_MAX];
		char fullNewPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);
		_pDriverInstance->absPath(fullNewPath, newpath);
		return RETURN_ERRNO(link(fullPath, fullNewPath));
	}

	int LVFSDriver::fuseChmod(const char *path, mode_t mode)
	{
		printf("chmod(path=%s, mode=%d)\n", path, mode);
		char fullPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);
		return RETURN_ERRNO(chmod(fullPath, mode));
	}

	int LVFSDriver::fuseChown(const char *path, uid_t uid, gid_t gid)
	{
		printf("chown(path=%s, uid=%d, gid=%d)\n", path, (int)uid, (int)gid);
		char fullPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);
		return RETURN_ERRNO(chown(fullPath, uid, gid));
	}

	int LVFSDriver::fuseTruncate(const char *path, off_t newSize)
	{
		printf("truncate(path=%s, newSize=%d\n", path, (int)newSize);
		char fullPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);
		return RETURN_ERRNO(truncate(fullPath, newSize));
	}

	int LVFSDriver::fuseUtime(const char *path, struct utimbuf *ubuf)
	{
		printf("utime(path=%s)\n", path);
		char fullPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);
		return RETURN_ERRNO(utime(fullPath, ubuf));
	}

	int LVFSDriver::fuseOpen(const char *path, struct fuse_file_info *fileInfo)
	{
		char fullPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);
		//fileInfo->fh = open(fullPath, fileInfo->flags);
		//close(fileInfo->fh);
		return 0;
	}

	int LVFSDriver::fuseRead(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
	{
		char fullPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);

		uint fileState = _pFileProxy->CheckFile(QString::fromUtf8(fullPath));

		if(!(fileState & VFSElement::eFl_Downloading)
			&& !(fileState & VFSElement::eFl_Downloaded))
			{
			if(_pFileProxy->ReadFile(QString::fromUtf8(fullPath)))
			{
				return -1;
			}
		}

		fileInfo->fh = open(fullPath, fileInfo->flags);
		printf("read(path=%s, size=%d, offset=%d)\n", path, (int)size, (int)offset);
		int res = pread(fileInfo->fh, buf, size, offset);
		close(fileInfo->fh);
		return res;
	}

	int LVFSDriver::fuseWrite(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
	{
		char fullPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);
		printf("write(path=%s, size=%d, offset=%d)\n", fullPath, (int)size, (int)offset);
		fileInfo->fh = open(fullPath, O_WRONLY);
		int err = pwrite(fileInfo->fh, buf, size, offset);
		close(fileInfo->fh);
		if(err == -1)
			err = -errno;
		return err;
	}

	int LVFSDriver::fuseStatfs(const char *path, struct statvfs *statInfo)
	{
		printf("statfs(path=%s)\n", path);
		char fullPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);
		return RETURN_ERRNO(statvfs(fullPath, statInfo));
	}

	int LVFSDriver::fuseFlush(const char *path, struct fuse_file_info *fileInfo)
	{
		return 0;
	}

	int LVFSDriver::fuseRelease(const char *path, struct fuse_file_info *fileInfo)
	{
		int err = 0;
		if(_curOperation == eCreation)
		{
			char fullPath[PATH_MAX];
			_pDriverInstance->absPath(fullPath, path);
			RESULT res = _pFileProxy->CreateFileW(QString::fromUtf8(fullPath));
			printf("res = %d\n", res);
                        if(res != eNO_ERROR)
				err = -1;

			_curOperation = eNONE;
		}
		return err;
	}

	int LVFSDriver::fuseFsync(const char *path, int datasync, struct fuse_file_info *fi)
	{
		if(datasync) {
			//sync data only
			return RETURN_ERRNO(fdatasync(fi->fh));
		} else {
			//sync data + file metadata
			return RETURN_ERRNO(fsync(fi->fh));
		}
	}

	int LVFSDriver::fuseSetxAttr(const char *path, const char *name, const char *value, size_t size, int flags)
	{
		char fullPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);
		return RETURN_ERRNO(lsetxattr(fullPath, name, value, size, flags));
	}

	int LVFSDriver::fuseGetxAttr(const char *path, const char *name, char *value, size_t size)
	{
		printf("getxattr(path=%s, name=%s, size=%d\n", path, name, (int)size);
		char fullPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);
		return RETURN_ERRNO(getxattr(fullPath, name, value, size));
	}

	int LVFSDriver::fuseListxAttr(const char *path, char *list, size_t size)
	{
		printf("listxattr(path=%s, size=%d)\n", path, (int)size);
		char fullPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);
		return RETURN_ERRNO(llistxattr(fullPath, list, size));
	}

	int LVFSDriver::fuseRemovexAttr(const char *path, const char *name)
	{
		printf("removexattry(path=%s, name=%s)\n", path, name);
		char fullPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);
		return RETURN_ERRNO(lremovexattr(fullPath, name));
	}

	int LVFSDriver::fuseOpenDir(const char *path, struct fuse_file_info *fileInfo)
	{
		char fullPath[PATH_MAX];
		_pDriverInstance->absPath(fullPath, path);
		DIR *dir = opendir(fullPath);
		fileInfo->fh = (uint64_t)dir;
		return NULL == dir ? -errno : 0;
	}

	int LVFSDriver::fuseReadDir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo)
	{
		DIR *dir = (DIR*)fileInfo->fh;
		struct dirent *de = readdir(dir);
		if(NULL == de) {
			return -errno;
		} else {
			do {
				if(filler(buf, de->d_name, NULL, 0) != 0) {
					return -ENOMEM;
				}
			} while(NULL != (de = readdir(dir)));
		}
		return 0;
	}

	int LVFSDriver::fuseReleaseDir(const char *path, struct fuse_file_info *fileInfo)
	{
		printf("releasedir(path=%s)\n", path);
		closedir((DIR*)fileInfo->fh);
		return 0;
	}

	int LVFSDriver::fuseFsyncDir(const char *path, int datasync, struct fuse_file_info *fileInfo)
	{
		return 0;
	}

	void* LVFSDriver::fuseInit(struct fuse_conn_info *conn)
	{
		int res = 0;
		return (void*)(&res);
	}

	int LVFSDriver::fuseUtimens(const char *path, const struct timespec ts[2])
	{
		int res;
		struct timeval tv[2];

		tv[0].tv_sec = ts[0].tv_sec;
		tv[0].tv_usec = ts[0].tv_nsec / 1000;
		tv[1].tv_sec = ts[1].tv_sec;
		tv[1].tv_usec = ts[1].tv_nsec / 1000;

		res = utimes(path, tv);
		if (res == -1)
			return -errno;

		return 0;
	}
	void LVFSDriver::run()
	{
		try
		{
			_notificationDevice = Common::WebMounter::getNotificationDevice();

			int status;
			unsigned long command;

			_driverOperations.getattr = LVFSDriver::fuseGetAttr;
			_driverOperations.readlink = LVFSDriver::fuseReadLink;
			_driverOperations.getdir = NULL;
			_driverOperations.lock = NULL;
			_driverOperations.create = NULL;
			_driverOperations.mknod = LVFSDriver::fuseMknod;
			_driverOperations.mkdir = LVFSDriver::fuseMkdir;
			_driverOperations.unlink = LVFSDriver::fuseUnlink;
			_driverOperations.rmdir = LVFSDriver::fuseRmdir;
			_driverOperations.symlink = LVFSDriver::fuseSymlink;
			_driverOperations.rename = LVFSDriver::fuseRename;
			_driverOperations.link = LVFSDriver::fuseLink;
			_driverOperations.chmod = LVFSDriver::fuseChmod;
			_driverOperations.chown = LVFSDriver::fuseChown;
			_driverOperations.truncate = LVFSDriver::fuseTruncate;
			_driverOperations.utime = LVFSDriver::fuseUtime;
			_driverOperations.open = LVFSDriver::fuseOpen;
			_driverOperations.read = LVFSDriver::fuseRead;
			_driverOperations.write = LVFSDriver::fuseWrite;
			_driverOperations.statfs = LVFSDriver::fuseStatfs;
			_driverOperations.flush = LVFSDriver::fuseFlush;
			_driverOperations.release = LVFSDriver::fuseRelease;
			_driverOperations.fsync = LVFSDriver::fuseFsync;
			_driverOperations.setxattr = LVFSDriver::fuseSetxAttr;
			_driverOperations.getxattr = LVFSDriver::fuseGetxAttr;
			_driverOperations.listxattr = LVFSDriver::fuseListxAttr;
			_driverOperations.removexattr = LVFSDriver::fuseRemovexAttr;
			_driverOperations.opendir = LVFSDriver::fuseOpenDir;
			_driverOperations.readdir = LVFSDriver::fuseReadDir;
			_driverOperations.releasedir = LVFSDriver::fuseReleaseDir;
			_driverOperations.fsyncdir = LVFSDriver::fuseFsyncDir;
			_driverOperations.init = LVFSDriver::fuseInit;
			_driverOperations.access = NULL;
			_driverOperations.utimens = LVFSDriver::fuseUtimens;
			_driverOperations.fgetattr = NULL;

			int argc = 4;
			printf("mount point =%s\n", _mountPoint.toUtf8().data());
			char* mnt = new char[_mountPoint.length()];
			strcpy(mnt, _mountPoint.toUtf8().data());
			char* argv[] = {"webmounter", mnt, "-f", "-s"};

			//realpath(...) returns the canonicalized absolute pathname
			setRootDir(realpath(_rootDirectory.toStdString().c_str(), NULL));

			emit mounted();

			if(_notificationDevice)
			{
				Notification msg(Notification::eINFO, tr("Info"), tr("Disk is mounted\n"));
				_notificationDevice->showNotification(msg);
			}

			LVFSDriver::fuseUmount();

			status = fuseMainCustom(argc, argv, &_driverOperations, NULL);

			emit unmounted();

			if(_notificationDevice)
			{
				Notification msg(Notification::eINFO, tr("Info"), tr("Disk is unmounted\n"));
				_notificationDevice->showNotification(msg);
			}

			delete[] mnt;
			/*			switch (status)
			{
			case DOKAN_ERROR:
				{
					Notification msg(Notification::eCRITICAL, tr("Error"), tr("Error\n"));
					notificationDevice->showNotification(msg);
					DbgPrint(L"Error\n");
					break;
				}
			case DOKAN_DRIVE_LETTER_ERROR:
				{
					Notification msg(Notification::eCRITICAL, tr("Error"), tr("Bad Drive letter\n"));
					//emit showDriverMessage(msg);
					notificationDevice->showNotification(msg);
					DbgPrint(L"Bad Drive letter\n");
					break;
				}
			case DOKAN_DRIVER_INSTALL_ERROR:
				{
					Notification msg(Notification::eCRITICAL, tr("Error"), tr("Can't install driver\n"));
					notificationDevice->showNotification(msg);
					DbgPrint(L"Can't install driver\n");
					break;
				}
			case DOKAN_START_ERROR:
				{
					Notification msg(Notification::eCRITICAL, tr("Error"), tr("Driver something wrong\n"));
					notificationDevice->showNotification(msg);
					DbgPrint(L"Driver something wrong\n");
					break;
				}
			case DOKAN_MOUNT_ERROR:
				{
					Notification msg(Notification::eCRITICAL, tr("Error"), tr("Can't assign a drive letter\n"));
					notificationDevice->showNotification(msg);
					DbgPrint(L"Can't assign a drive letter\n");
					break;

			case DOKAN_SUCCESS:
				{
					Notification msg(Notification::eINFO, tr("Info"), tr("Disk is unmounted\n"));
					notificationDevice->showNotification(msg);
					DbgPrint(L"Disk is unmounted\n");
					break;
				}
			default:
				{
					Notification msg(Notification::eCRITICAL, tr("Error"), tr("Unknown error\n"));
					notificationDevice->showNotification(msg);
					DbgPrint(L"Unknown error: %d\n", status);
					break;
				}
			}

			free(_pDriverOptions);
			free(_pDriverOperations);
*/
		}
		catch(...)
		{
		}
	}

	LVFSDriver* LVFSDriver::createDriver(FileProxy * pProxy)
	{
		QMutexLocker locker(&_DriverMutex);
		if(!_pDriverInstance)
		{
			_pDriverInstance = new LVFSDriver(pProxy);
		}
		return _pDriverInstance;
	}

	void LVFSDriver::startDriver()
	{
		if(_pDriverInstance)
		{
			_pDriverInstance->start();
		}
	}
	void LVFSDriver::stopDriver()
	{
		if(_pDriverInstance->isRunning())
		{
			if(LVFSDriver::fuseUmount())
			{
				Ui::Notification msg(Ui::Notification::eERROR, tr("Error"), tr("Device is busy. Can not be unmounted\n"));
				Ui::NotificationDevice* notificationDevice = Common::WebMounter::getNotificationDevice();
				notificationDevice->showNotification(msg);
			}
		}
	}


	int LVFSDriver::fuseMainCustom(int argc, char *argv[],
								   const struct fuse_operations *op, void *user_data)
	{
		char *mountpoint;
		int multithreaded;
		int res;

		_pDriverInstance->_pFuseObj = fuse_setup(argc, argv, op, sizeof(*(op)), &mountpoint,
												 &multithreaded, user_data);
		if (_pDriverInstance->_pFuseObj == NULL)
			return 1;

		res = fuse_loop(_pDriverInstance->_pFuseObj);
		return 0;
	}

	void LVFSDriver::absPath(char dest[PATH_MAX], const char *path)
	{
		strcpy(dest, _pDriverInstance->_root);
		strncat(dest, path, PATH_MAX);
	}

	int LVFSDriver::fuseUmount()
	{
		QString cmd("fusermount " + _mountPoint + " -u");
		return system(cmd.toUtf8().data());
	}
};

