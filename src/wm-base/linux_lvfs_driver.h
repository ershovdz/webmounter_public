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

#ifndef LVFSDRIVER_H
#define LVFSDRIVER_H

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/xattr.h>

//QT headers
#include <QThread>
#include <QMutex>

//WebMounter headers
#include "lvfs_driver.h"
#include "file_proxy.h"
//#include "NotificationDevice.h"
#include "data.h"

//#include "control_panel.h"
#include "common_stuff.h"



namespace LocalDriver
{
	using Common::FileProxy;
	using Ui::NotificationDevice;
	using Ui::Notification;

	class LVFSDriver : public ILVFSDriver
	{
		Q_OBJECT

	private:
		~LVFSDriver(void);
		LVFSDriver(FileProxy * pProxy);
		LVFSDriver(const LVFSDriver &);
		LVFSDriver& operator=(const LVFSDriver &);
		void run();

	public slots:
		virtual void mount(Data::GeneralSettings& generalSettings);
		virtual void unmount();

	private:
		void absPath(char dest[PATH_MAX], const char *path);

		//filesystem calls
		static void setRootDir(const char *path);

		static int fuseGetAttr(const char *path, struct stat *statbuf);
		static int fuseReadLink(const char *path, char *link, size_t size);
		static int fuseMknod(const char *path, mode_t mode, dev_t dev);
		static int fuseMkdir(const char *path, mode_t mode);
		static int fuseUnlink(const char *path);
		static int fuseRmdir(const char *path);
		static int fuseSymlink(const char *path, const char *link);
		static int fuseRename(const char *path, const char *newpath);
		static int fuseLink(const char *path, const char *newpath);
		static int fuseChmod(const char *path, mode_t mode);
		static int fuseChown(const char *path, uid_t uid, gid_t gid);
		static int fuseTruncate(const char *path, off_t newSize);
		static int fuseUtime(const char *path, struct utimbuf *ubuf);
		static int fuseOpen(const char *path, struct fuse_file_info *fileInfo);
		static int fuseRead(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo);
		static int fuseWrite(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo);
		static int fuseStatfs(const char *path, struct statvfs *statInfo);
		static int fuseFlush(const char *path, struct fuse_file_info *fileInfo);
		static int fuseRelease(const char *path, struct fuse_file_info *fileInfo);
		static int fuseFsync(const char *path, int datasync, struct fuse_file_info *fi);
		static int fuseSetxAttr(const char *path, const char *name, const char *value, size_t size, int flags);
		static int fuseGetxAttr(const char *path, const char *name, char *value, size_t size);
		static int fuseListxAttr(const char *path, char *list, size_t size);
		static int fuseRemovexAttr(const char *path, const char *name);
		static int fuseOpenDir(const char *path, struct fuse_file_info *fileInfo);
		static int fuseReadDir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo);
		static int fuseReleaseDir(const char *path, struct fuse_file_info *fileInfo);
		static int fuseFsyncDir(const char *path, int datasync, struct fuse_file_info *fileInfo);
		static void* fuseInit(struct fuse_conn_info *conn);
		static int fuseUtimens(const char *path, const struct timespec ts[2]);

	public:
		static const char *_root;
	private:
		static FileProxy* _pFileProxy;
		//static NotificationDevice* _pNotificationDevice;

		static LVFSDriver* _pDriverInstance;

		struct fuse_operations _driverOperations;
		static QMutex _DriverMutex;

		static QString _rootDirectory;

		static QString _mountPoint;
		static struct fuse* _pFuseObj;
		static enum Operations
		{
			eNONE = 0,
			eCreation
		} _curOperation;
		Ui::NotificationDevice* _notificationDevice;

	public:
		static LVFSDriver* createDriver(FileProxy*);

	private:
		static void startDriver();
		static void stopDriver();
		int fuseMainCustom(int argc, char *argv[],
		const struct fuse_operations *op, void *user_data);
		static int fuseUmount();
	};
}
#endif //LVFSDriver_H
