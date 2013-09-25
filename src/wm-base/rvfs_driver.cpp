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

#include <QtCore>
#include <boost/bind.hpp>

#include "rvfs_driver.h"
#include "webmounter.h"
#include "filesystemtools.h"

using namespace RemoteDriver;
using namespace Common;

RESULT RVFSDriver::createFile(const QString& /*path*/, const QString& /*title*/,  const QString& /*id*/, const QString& /*parentId*/)
{
	return eNO_ERROR;
}
/***********************************************************************
***************************** SLOTS ************************************							  
***********************************************************************/
void RVFSDriver::startPlugin(PluginSettings& pluginSettings)
{
	QtConcurrent::run(
		boost::bind(&RemoteDriver::RVFSDriver::connectHandler, _1, _2)
		, this
		, pluginSettings
		);
}
void RVFSDriver::stopPlugin()
{
	QtConcurrent::run(boost::bind(&RemoteDriver::RVFSDriver::disconnectHandler, _1), this);
}
void RVFSDriver::startSync()
{
	QtConcurrent::run(boost::bind(&RemoteDriver::RVFSDriver::syncHandler, _1), this);
}
void RVFSDriver::stopSync()
{
	QtConcurrent::run(boost::bind(&RemoteDriver::RVFSDriver::stopSyncHandler, _1), this);
}

void RVFSDriver::updateState(int progress, DriverState newState)
{
	m_state = newState;
	emit updateView(progress, (int)newState);
}

void RVFSDriver::updateDownloadStatus(RESULT downloadResult, const unsigned int uDownloaded, const unsigned int uNotDownloaded)
{
	if(!downloadResult)
	{
		updateSyncStatus((int)(((float)uDownloaded/uNotDownloaded)*50) + 50);
	}
	else
	{
		notifyUser(Ui::Notification::eCRITICAL, tr("Error"), tr("Downloading failed  !\n"));
		updateSyncStatus(100);
	}
}

unsigned int RVFSDriver::countNotDownloaded()
{
	VFSCache* cache = WebMounter::getCache();
	unsigned int counter = 0;

	for(VFSCache::iterator iter = cache->begin(); 
		iter != cache->end(); 
		++iter)
	{
		if( iter->getPluginName() == m_pluginName && !iter->isDownloaded() && iter->getType() == VFSElement::FILE)
		{
			counter++;
		}
	}

	return counter;
}

unsigned int RVFSDriver::countObjects(VFSElement::VFSElementType type)
{
	VFSCache* cache = WebMounter::getCache();
	unsigned int counter = 0;

	for(VFSCache::iterator iter = cache->begin(); 
		iter != cache->end(); 
		++iter)
	{
		if( iter->getPluginName() == m_pluginName && iter->getType() == type)
		{
			counter++;
		}
	}

	return counter;
}

void RVFSDriver::updateSyncStatus(double currentStatus)
{
	QMutexLocker locker(&m_driverMutex);
	if(m_state != eSyncStopping)
	{
		DriverState state = (currentStatus != 100) ? RemoteDriver::eSync : RemoteDriver::eConnected;
		updateState(currentStatus, state);
	}
}



void RVFSDriver::syncCacheWithFileSystem(const QString& path)
{
	VFSCache* vfsCache = WebMounter::getCache();
	QDir dir(path);
	VFSCache::iterator parent = vfsCache->find(path);

	if(parent == vfsCache->end()) 
		return;

	for(VFSCache::iterator iter = vfsCache->begin(); iter != vfsCache->end(); ++iter)
	{
		if(iter->getParentId() != parent->getId()) continue;

		if(iter->getType() == VFSElement::DIRECTORY)
		{
			if(!dir.exists(iter->getPath())) 
				dir.mkpath(iter->getPath());
		}
		else if(iter->getType() == VFSElement::FILE)
		{
			QFile file(iter->getPath());
			if(!file.exists())
			{
				file.open(QIODevice::WriteOnly);
				file.close();
				vfsCache->setFlag(iter, VFSElement::eFl_None, VFSElement::eFl_Downloaded);
			}
			else if(file.size() == 0 
				&& iter->getFlags() != VFSElement::eFl_None 
				&& iter->getFlags() != VFSElement::eFl_NameDup)
			{
				vfsCache->setFlag(iter, VFSElement::eFl_None, VFSElement::eFl_Downloaded);
			}
		}
	}

	QStringList lstDirs = dir.entryList(QDir::Dirs | QDir::AllDirs | QDir::NoDotAndDotDot);
	QStringList lstFiles = dir.entryList(QDir::Files);

	foreach (QString entry, lstFiles)
	{
		QString entryAbsPath = QFileInfo(dir.absolutePath() + "/" + entry).absoluteFilePath();

		if(vfsCache->find(entryAbsPath) == vfsCache->end())
		{
			QFile::Permissions permissions = QFile::permissions(entryAbsPath);
			permissions |= (QFile::WriteGroup|QFile::WriteOwner|QFile::WriteUser|QFile::WriteOther);
			QFile::setPermissions(entryAbsPath, permissions);

			QFile::remove(entryAbsPath);
		}
	}

	foreach (QString entry, lstDirs)
	{
		QString entryAbsPath = QFileInfo(dir.absolutePath() + "/" + entry).absoluteFilePath();

		if(vfsCache->find(entryAbsPath) == vfsCache->end())
		{
			QDir dir(entryAbsPath);
			FileSystemTools::removeFolder(dir);
		}
		else
		{
			syncCacheWithFileSystem(entryAbsPath);
		}
	}
}

RESULT RVFSDriver::downloadFiles()
{
	QList <QString> urlToDownload;
	QList <QString> pathToDownload;
	VFSCache* vfsCache = WebMounter::getCache();
	VFSCache::iterator iter = vfsCache->begin();

	unsigned int uDownloaded = 0;
	RESULT err = eNO_ERROR;

	int filesCount = countObjects(VFSElement::FILE);
	double progressPortion = (filesCount == 0) ? 0 : (m_progressData.m_maxValue - m_progressData.m_currProgress)/filesCount;

	for(iter = vfsCache->begin(); iter != vfsCache->end(); ++iter)
	{
		{
			QMutexLocker locker(&m_driverMutex);
			if(m_state == eSyncStopping)
			{
				notifyUser(Ui::Notification::eINFO, tr("Info"), tr("Downloading is stopped !\n"));
				return eERROR_CANCEL;
			}
		}

		if(iter->getPluginName() == m_pluginName)
		{
			if(iter->getType() == VFSElement::FILE 
				&& !(iter->getFlags() & VFSElement::eFl_Downloading)
				&& !(iter->getFlags() & VFSElement::eFl_Downloaded))
			{
				urlToDownload.append(iter->getSrcUrl());
				pathToDownload.append(iter->getPath());

				//if(urlToDownload.size() == DOWNLOAD_CHUNK_SIZE)
				//{
				err = downloadFiles(urlToDownload, pathToDownload);

				if(err)
				{
					notifyUser(Ui::Notification::eCRITICAL, tr("Error"), tr("Downloading failed  !\n"));
					updateSyncStatus(m_progressData.m_maxValue);
					return err;
				}
				else
				{
					uDownloaded += urlToDownload.size();
					m_progressData.m_currProgress += urlToDownload.size()*progressPortion;
					updateSyncStatus(m_progressData.m_currProgress);
					urlToDownload.clear();
					pathToDownload.clear();
				}
				//}
			}
			else if(iter->getFlags() & VFSElement::eFl_Downloaded)
			{
				uDownloaded++;
				m_progressData.m_currProgress += progressPortion;
				updateSyncStatus(m_progressData.m_currProgress);
				double duration = filesCount > 0 ? min(1000,	10000/filesCount) : 1000;
				msleep(duration);
			}
		}
	}

	if(urlToDownload.size() > 0)
	{
		return downloadFiles(urlToDownload, pathToDownload);
	}

	return err;
}

void RVFSDriver::notifyUser(Ui::Notification::_Types type, QString title, QString description) const
{
	Ui::NotificationDevice* notifDevice = WebMounter::getNotificationDevice();

	Ui::Notification msg(type, title, description);

	notifDevice->showNotification(msg);
}


/***********************************************************************
*********************** Handlers for slots *****************************							  
***********************************************************************/

void RVFSDriver::connectHandler(PluginSettings& /*pluginSettings*/)
{
	updateState(100, eAuthInProgress);

	updateState(100, eAuthorized);

	updateState(100, eSync);

	updateState(100, eConnected);
}

void RVFSDriver::disconnectHandler()
{
	updateState(100, eNotConnected);
}

void RVFSDriver::syncHandler()
{
	updateState(100, eSync);

	updateState(100, eConnected);
}

void RVFSDriver::stopSyncHandler()
{
	updateState(100, eSyncStopping);

	updateState(100, eConnected);
}

bool RVFSDriver::areFileAttributesValid(const QString&, unsigned long)
{
	return true;
}

void RVFSDriver::updateChildrenPath(const VFSElement& elem)
{
	VFSCache* vfsCache = WebMounter::getCache();
	VFSCache::iterator iter = vfsCache->begin();
	for(iter; iter != vfsCache->end(); ++iter)
	{
		if(iter->getParentId() == elem.getId())
		{
			QString path = elem.getPath() + QString(QDir::separator()) + iter->getName();
			QFileInfo fInfo(path);

			if(iter->getPath() != fInfo.absoluteFilePath())
			{
				VFSElement newElem(iter->getType()
					, fInfo.absoluteFilePath()
					, iter->getName()
					, iter->getEditMetaUrl()
					, iter->getEditMediaUrl()
					, iter->getSrcUrl()
					, iter->getId()
					, iter->getParentId()
					, iter->getModified()
					, iter->getPluginName()
					, iter->getFlags());

				vfsCache->erase(iter);
				vfsCache->insert(newElem);
				iter = vfsCache->begin();
			}
		}
	}
}

void RVFSDriver::mergeToCache(QList<VFSElement>& elements)
{
	QDir qDir;
	QFile qFile;
	VFSCache* vfsCache = WebMounter::getCache();
	VFSCache::iterator iter = vfsCache->begin();

	for(iter; iter != vfsCache->end(); ++iter)
	{
		if(iter->getPluginName() == m_pluginName)
		{
			bool found = false;
			int count = elements.count();
			// Try to find element in new data
			for(int i=0; i<count; i++)
			{
				if(iter->getId() == elements[i].getId())
				{
					found = true;
					if(iter != elements[i])
					{
						if(iter->getType() == VFSElement::DIRECTORY)
						{
							qDir.rename(iter->getPath(), elements[i].getPath());
						}
						break;
					}

					elements.removeAt(i);
					break;
				}
			}

			// Element has been deleted
			if(!found && iter->getId() != ROOT_ID)
			{
				VFSCache::iterator iter1 = iter--;
				vfsCache->erase(iter1);
			}
		}
	}

	m_progressData.m_currProgress += 5;
	updateSyncStatus(m_progressData.m_currProgress);

	// Add newly created elements
	for(int i=0; i<elements.count(); i++)
	{
		vfsCache->insert(elements[i]);
	}
}

RESULT RVFSDriver::sync()
{
	RESULT res = eERROR_GENERAL;
	SettingStorage* settings = WebMounter::getSettingStorage();
	QString pluginStoragePath = settings->getAppStoragePath() + QString(QDir::separator()) + m_pluginName;
	QFileInfo fInfo(pluginStoragePath);
	PluginSettings plSettings;
	settings->getData(plSettings, m_pluginName);
	m_progressData.m_currProgress = 0;

	updateSyncStatus(m_progressData.m_currProgress);

	VFSCache* vfsCache = WebMounter::getCache();

	if( plSettings.m_userName != plSettings.m_prevUserName )
	{
		QDir dir(pluginStoragePath);
		FileSystemTools::removeFolderContent( dir );
		vfsCache->erasePlugin( m_pluginName );
	}

	m_progressData.m_currProgress += 5;
	updateSyncStatus(m_progressData.m_currProgress);

	QList<VFSElement> elements;
	m_progressData.m_maxValue = plSettings.m_fullSync ? 40 : 90;

	res = getElements(elements);
	if(!res)
	{
		mergeToCache(elements);

		m_progressData.m_currProgress += 4;
		updateSyncStatus(m_progressData.m_currProgress);

		QString rootPath = WebMounter::getSettingStorage()->getAppStoragePath() + QString(QDir::separator()) + m_pluginName;
		QFileInfo fInfo(rootPath);
		syncCacheWithFileSystem(fInfo.absoluteFilePath());

		m_progressData.m_currProgress += 4;
		m_progressData.m_maxValue = 100;
		updateSyncStatus(m_progressData.m_currProgress);

		if(plSettings.m_fullSync)
		{
			res = downloadFiles();
		}
	}

	if(res != eNO_ERROR && res != eERROR_CANCEL)
	{
		stopPlugin();
		notifyUser(Ui::Notification::eCRITICAL, tr("Error"), tr("Sync failed !\n"));
		//updateState(100, RemoteDriver::eNotConnected);
	}
	else if(res != eERROR_CANCEL)
	{
		updateSyncStatus(m_progressData.m_maxValue);
	}

	return res;
}

void RVFSDriver::run()
{
	PluginSettings pluginSettings;

	WebMounter::getSettingStorage()->getData(pluginSettings, m_pluginName);

	int sync_period = pluginSettings.m_syncPeriod.toInt();

	forever
	{
		if(m_state == eSyncStopping || m_state == eNotConnected)
		{
			return;
		}

		if(m_state == eAuthorized)
		{
			sync();
			if(m_state == eSyncStopping)
			{
				return;
			}
		}

		{
			QMutexLocker locker(&m_syncMutex);
			m_forceSync.wait(&m_syncMutex, sync_period * 1000);
		}

		if(m_state == eConnected)
		{
			sync();
		}

		if(m_state == eSyncStopping)
		{
			return;
		}
	}
}
