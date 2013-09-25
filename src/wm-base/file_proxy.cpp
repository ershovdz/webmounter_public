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

#include "file_proxy.h"
#include "vfs_cache.h"
#include "webmounter.h"

#include <QDir>
#include <QFileInfo>
#include <QObject>

#include <boost/bind.hpp>

namespace Common
{

	//RVFSDriver* FileProxy::_pRVFSDriver = 0;
	FileProxy* FileProxy::m_fileProxyInstance = 0;
	QMutex FileProxy::m_fileProxyMutex;
	set<QString> FileProxy::m_currentFiles;

	unsigned int FileProxy::m_uploaded = 0;
	unsigned int FileProxy::m_notUploaded = 0;
	QList<QString> FileProxy::m_uploadQueue;
	//JmmRVFSDriver rvfsDriver;

	unsigned int FileProxy::m_notDeleted = 0;
	unsigned int FileProxy::m_deleted = 0;
	QList<QString> FileProxy::m_deleteQueue;

	FileProxy::FileProxy(void)
	{
	}


	FileProxy::~FileProxy(void)
	{
	}

	/*FileProxy::FileProxy(RVFSDriver* pDriver)
	{
	if(pDriver && pDriver != _pRVFSDriver)
	{
	if(_pRVFSDriver != 0) delete _pRVFSDriver;
	_pRVFSDriver = pDriver;
	}
	}*/

	FileProxy* FileProxy::CreateFileProxy()
	{
		//QMutexLocker locker(&m_fileProxyMutex);
		if(m_fileProxyInstance)
		{
			delete m_fileProxyInstance;
		}
		m_fileProxyInstance = new FileProxy();
		return m_fileProxyInstance;
	}

	RESULT FileProxy::CreateFileW(QString path)
	{
		QMutexLocker locker(&m_fileProxyMutex);

		VFSCache* cache = WebMounter::getCache();
		QFileInfo qFileInfo(path);

		if(qFileInfo.baseName().mid(0, 2).toLower() == "~$"
			|| qFileInfo.baseName().mid(0, 4).toLower() == "~wrd"
			|| qFileInfo.baseName().mid(0, 4).toLower() == "~wrl") // temp file of MS Office - do nothing
		{
			return eNO_ERROR;
		}

		RVFSDriver* driver = extractPlugin(path);

		if(!driver)
		{
			return eERROR_GENERAL;
		}

		VFSCache::iterator file		 = cache->find(qFileInfo.absoluteFilePath());
		VFSCache::iterator parentDir = cache->find(qFileInfo.dir().absolutePath());

		QFileInfo fInfo(path);
		if(fInfo.size())
		{
			IncreaseNotUploadedCounter();
			m_uploadQueue.append(fInfo.absoluteFilePath());

			if(file != cache->end())
			{
				QtConcurrent::run(boost::bind(&RemoteDriver::RVFSDriver::uploadFile, _1, _2, _3, _4, _5), driver, path, qFileInfo.baseName(), file->getId(), parentDir->getId());
				//return driver->uploadFile(path, qFileInfo.baseName(), file->getId(), parentDir->getId());
			}
			else
			{
				QtConcurrent::run(boost::bind(&RemoteDriver::RVFSDriver::uploadFile, _1, _2, _3, _4, _5), driver, path, qFileInfo.baseName(), QString(ROOT_ID), parentDir->getId());
				//return driver->uploadFile(path, qFileInfo.baseName(), ROOT_ID, parentDir->getId());
			}
		}
		else
		{
			return driver->createFile(path, qFileInfo.baseName(), QString(ROOT_ID), parentDir->getId());
		}


		return eNO_ERROR;
	}

	unsigned int FileProxy::CheckFile(QString path)
	{
		//QMutexLocker locker(&m_fileProxyMutex);  // probably it's useless locker ???
		QFileInfo fInfo(path);
		VFSCache* cache = WebMounter::getCache();
		VFSCache::iterator iter = cache->find(fInfo.absoluteFilePath());

		if(fInfo.baseName().mid(0, 2).toLower() == "~$") // temp file for MS Office - do nothing
		{
			return Data::VFSElement::eFl_Downloaded;
		}

		if(iter != cache->end())
		{
			return iter->getFlags();
		}

		return Data::VFSElement::eFl_All;
	}

	RESULT FileProxy::UnCheckFile(QString path)
	{
		QMutexLocker locker(&m_fileProxyMutex);
		QFileInfo fInfo(path);

		if(m_currentFiles.find(fInfo.absoluteFilePath()) != m_currentFiles.end()) 
		{
			//_currentFiles.erase(_currentFiles.find(fInfo.absoluteFilePath()));
			return eNO_ERROR;
		}
		else
		{
			//Sleep(2000);
			return eERROR_GENERAL;
		}
	}

	RESULT FileProxy::ReadFile(QString path)
	{
		//QMutexLocker locker(&m_fileProxyMutex);  // probably it's useless locker ???
		QList <QString> urlList;
		QList <QString> pathList;

		QFileInfo fInfo(path);
		VFSCache* cache = WebMounter::getCache();

		VFSCache::iterator iter = cache->find(fInfo.absoluteFilePath());

		if(fInfo.baseName().mid(0, 2).toLower() == "~$"
			|| fInfo.baseName().mid(0, 4).toLower() == "~wrd"
			|| fInfo.baseName().mid(0, 4).toLower() == "~wrl") // temp file for MS Office - do nothing
		{
			return eNO_ERROR;
		}

		if(iter != cache->end())
		{
			if(iter->getFlags() &  VFSElement::eFl_Downloading)
			{
				return eERROR_GENERAL;
			}
			else if(iter->getFlags() &  VFSElement::eFl_Downloaded)
			{
				return eNO_ERROR;
			}

			RVFSDriver* driver = extractPlugin(path);

			if(!driver)
			{
				return eERROR_GENERAL;
			}

			if(driver->getState() != RemoteDriver::eConnected)
			{
				notifyUser(Ui::Notification::eINFO, QObject::tr("Info"), QString(iter->getPluginName() + QObject::tr("Plugin is not connected !\nPlugin has to be in connected state.\n")));
				return eERROR_WRONG_STATE;
			}

			urlList.append(iter->getSrcUrl());
			pathList.append(iter->getPath());

			return driver->downloadFiles(urlList, pathList);
		}
		return eERROR_GENERAL;
	}

	RESULT FileProxy::CreateDirectoryW(QString path)
	{
		QDir dir(path);
		VFSCache* cache = WebMounter::getCache();

		if(cache->find(dir.absolutePath()) !=  cache->end())
		{
			return eNO_ERROR; //directory already exists. Let's return no error.
		}

		QString dirName = dir.dirName();
		QString dirPath = dir.absolutePath();
		QString parentId = ROOT_ID;

		if(dir.cdUp())
		{
			VFSCache::iterator iter = cache->find(dir.absolutePath());
			if(iter != cache->end())
			{
				parentId = iter->getId();
			}
		}

		RVFSDriver* driver = extractPlugin(path);

		if(!driver)
		{
			return eERROR_GENERAL;
		}

		if(driver->getState() != RemoteDriver::eConnected)
		{
			notifyUser(Ui::Notification::eINFO, QObject::tr("Info"),QObject::tr("Plugin is not connected !\nPlugin has to be in connected state.\n"));
			return eERROR_WRONG_STATE;
		}

		return driver->createDirectory(dirPath, parentId, dirName);  
	}

	RESULT FileProxy::MoveElement(QString from, QString to, bool bFirstRequest)
	{
		QMutexLocker locker(&m_fileProxyMutex);

		VFSCache* cache = WebMounter::getCache();

		QFile qFile(from);
		QFileInfo fInfoFrom(from);
		QFileInfo fInfoTo(to);

		QDir qDirFrom = fInfoFrom.dir();
		QDir qDirTo = fInfoTo.dir();

		if(fInfoFrom.baseName().mid(0, 2).toLower() == "~$"
			|| fInfoTo.baseName().mid(0, 4).toLower() == "~wrl"
			|| fInfoTo.baseName().mid(0, 4).toLower() == "~wrd") // temp file for MS Office - do nothing
		{
			return eNO_ERROR;
		}

		//Hack for MS Word. If it's a first request,
		//i.e. BEFORE actual (physical) moving file - just ignore this request
		if(fInfoFrom.baseName().mid(0, 4).toLower() == "~wrd" && bFirstRequest)
		{
			return eNO_ERROR;
		}
		else if(fInfoFrom.baseName().mid(0, 4).toLower() != "~wrd" && bFirstRequest == false)
		{
			return eNO_ERROR;
		}

		VFSCache::iterator it_dir_from = cache->find(qDirFrom.absolutePath());
		VFSCache::iterator it_dir_to = cache->find(qDirTo.absolutePath());
		VFSCache::iterator it_file_from = cache->find(fInfoFrom.absoluteFilePath());
		VFSCache::iterator it_file_to = cache->find(fInfoTo.absoluteFilePath());

		if(from != to && it_dir_from == it_dir_to) //rename or replace element
		{
			RVFSDriver* driver = extractPlugin(from);

			if(!driver)
			{
				return eERROR_GENERAL;
			}

			if(driver->getState() != RemoteDriver::eConnected)
			{
				notifyUser(Ui::Notification::eINFO, QObject::tr("Info"),QObject::tr("Plugin is not connected !\nPlugin has to be in connected state.\n"));
				return eERROR_WRONG_STATE;
			}

			if(fInfoFrom.baseName().mid(0, 4).toLower() == "~wrd"
				&& it_file_to != cache->end())
			{
				IncreaseNotUploadedCounter();
				m_uploadQueue.append(fInfoFrom.absoluteFilePath());

				QtConcurrent::run(boost::bind(&RemoteDriver::RVFSDriver::uploadFile, _1, _2, _3, _4, _5)
					, driver
					, fInfoFrom.absoluteFilePath() // path to replacing element
					, fInfoTo.baseName()		 // name of existing element
					, it_file_to->getId()
					, it_dir_to->getId());
				return eNO_ERROR;
			}
			else
			{
				return driver->renameElement(it_file_from->getPath(), fInfoTo.fileName());
			}
		}
		else if(//it_dir_from != cache->end() 
			it_dir_to != cache->end() 
			&& it_file_from != cache->end()) //From the inside, on the inside.
		{
			RVFSDriver* plugin_from = extractPlugin(from);
			RVFSDriver* plugin_to = extractPlugin(to);

			if(!plugin_from || !plugin_to)
			{
				return eERROR_GENERAL;
			}

			if(plugin_to != plugin_from)
			{
				notifyUser(Ui::Notification::eCRITICAL, QObject::tr("Info"), QObject::tr("Moving is possible within a single plugin only\n"));
				return eERROR_GENERAL;
			}

			if(plugin_to->getState() != RemoteDriver::eConnected)
			{
				notifyUser(Ui::Notification::eINFO, QObject::tr("Info"), QObject::tr("Plugin is not connected !\nPlugin has to be in connected state.\n"));
				return eERROR_WRONG_STATE;
			}

			return plugin_to->moveElement(it_file_from->getPath(), it_dir_to->getId());
		}
		return eERROR_GENERAL;
	}

	RESULT FileProxy::RemoveFile(QString path)
	{
		RESULT res = eERROR_GENERAL;
		RVFSDriver* driver = extractPlugin(path);
		if(driver)
		{
			if(driver->getState() != RemoteDriver::eConnected)
			{
				notifyUser(Ui::Notification::eINFO, QObject::tr("Info"),QObject::tr("Plugin is not connected !\nPlugin has to be in connected state.\n"));
				return eERROR_WRONG_STATE;
			}

			QFileInfo fInfo(path);
			increaseNotDeletedCounter();
			m_deleteQueue.append(fInfo.absoluteFilePath());

			QFile::Permissions permissions = QFile::permissions(fInfo.absoluteFilePath());
			permissions |= (QFile::WriteGroup|QFile::WriteOwner|QFile::WriteUser|QFile::WriteOther);
			QFile::setPermissions(fInfo.absoluteFilePath(), permissions);

			QtConcurrent::run(boost::bind(&RemoteDriver::RVFSDriver::deleteFile, _1, _2), driver, fInfo.absoluteFilePath());
			res = eNO_ERROR;
		}
		return res;
	}

	void FileProxy::notifyUser(Ui::Notification::_Types type, QString title, QString description)
	{
		Ui::NotificationDevice* notifDevice = WebMounter::getNotificationDevice();

		Ui::Notification msg(type, title, description);

		notifDevice->showNotification(msg);
	}

	RESULT FileProxy::RemoveDir(QString& path)
	{
		RESULT res = eERROR_GENERAL;
		RVFSDriver* driver = extractPlugin(path);
		if(driver)
		{
			if(driver->getState() != RemoteDriver::eConnected)
			{
				notifyUser(Ui::Notification::eINFO, QObject::tr("Info"),QObject::tr("Plugin is not connected !\nPlugin has to be in connected state.\n"));
				return eERROR_WRONG_STATE;
			}

			QFileInfo fInfo(path);
			res = driver->deleteDirectory( fInfo.absoluteFilePath() );
		}
		return res;
	}

	RVFSDriver* FileProxy::extractPlugin(const QString& path) const
	{
		QDir dirFrom(path);

		QDir rootDir(WebMounter::getSettingStorage()->getAppStoragePath());

		QString pluginName = "";

		while(dirFrom != rootDir)
		{
			pluginName = dirFrom.dirName();

			if(!dirFrom.cdUp())
			{
				break;
			}
		}

		if(pluginName != "")
		{
			return WebMounter::getPlugin(pluginName);
		}
		else
		{
			notifyUser(Ui::Notification::eINTERACTIVE, QString("Info"), QString("Select a specific plugin, at first\n"));
			return 0; // Looks like it was root path
		}
	}

	bool FileProxy::CheckFileAttributes(const QString& path, unsigned long attributes)
	{
		RVFSDriver* driver = extractPlugin(path);
		return driver->areFileAttributesValid(path, attributes);
	}

	void FileProxy::fileUploaded(QString filePath, RESULT result)
	{
		QList<QString>::iterator iter;
		for(iter = m_uploadQueue.begin(); iter != m_uploadQueue.end(); iter++)
		{
			if(*iter == filePath)
			{
				m_uploadQueue.erase(iter);
				break;
			}
		}

		if(m_uploadQueue.empty())
		{
			m_uploaded = 0;
			m_notUploaded = 0;

			RVFSDriver* driver = extractPlugin(filePath);
			if(driver != NULL && driver->getState() == RemoteDriver::eConnected)
			{
				if(result == eNO_ERROR)
				{
					notifyUser(Ui::Notification::eINFO
						, QObject::tr("Info")
						, QObject::tr("Upload completed !"));
				}
				else if(result != eERROR_NOT_SUPPORTED)
				{
					notifyUser(Ui::Notification::eINFO
						, QObject::tr("Info")
						, QObject::tr("Upload completed, but some files has been not uploaded !"));
				}		
			}
		}
		else
		{
			IncreaseUploadedCounter();
			notifyUser(Ui::Notification::eINFO
				, QObject::tr("Info")
				, QObject::tr("Uploading progress (") 
				+ QString::number(GetUploadedCounter()) 
				+ QObject::tr(" of ") 
				+ QString::number(GetNotUploadedCounter())
				+ " ) !");
		}
	}

	void FileProxy::fileDeleted(const QString& filePath, RESULT result)
	{
		QList<QString>::iterator iter;
		for(iter = m_deleteQueue.begin(); iter != m_deleteQueue.end(); iter++)
		{
			if(*iter == filePath)
			{
				m_deleteQueue.erase(iter);
				// 				if(result == eNO_ERROR)
				// 				{
				// 					VFSCache* cache = WebMounter::getCache();
				// 					VFSCache::iterator elem = cache->find(filePath);
				// 					if(elem != cache->end())
				// 						cache->erase(elem);
				// 				}
				break;
			}
		}

		if(m_deleteQueue.empty())
		{
			m_deleted = 0;
			m_notDeleted = 0;

			if(result == eNO_ERROR)
			{
				notifyUser(Ui::Notification::eINFO
					, QObject::tr("Info")
					, QObject::tr("Deletion completed !"));
			}
			else
			{
				notifyUser(Ui::Notification::eINFO
					, QObject::tr("Info")
					, QObject::tr("Deletion completed, but some files has been not deleted !"));
			}		
		}
		else
		{
			increaseDeletedCounter();
			notifyUser(Ui::Notification::eINFO
				, QObject::tr("Info")
				, QObject::tr("Deleting progress (") 
				+ QString::number(getDeletedCounter()) 
				+ QObject::tr(" of ") 
				+ QString::number(getNotDeletedCounter())
				+ " ) !");
		}
	}
}
