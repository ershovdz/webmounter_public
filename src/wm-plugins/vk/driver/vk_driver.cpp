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

#include "webmounter.h"
#include "../driver/vk_driver.h"
#include "reg_exp.h"

namespace RemoteDriver
{
	using namespace Common;
	using namespace Connector;

	VkRVFSDriver::VkRVFSDriver(const QString& pluginName)
	{
        m_state = RemoteDriver::eNotConnected;
        m_httpConnector = new VkHTTPConnector();
        m_pluginName = pluginName;

		RESULT res = WebMounter::getCache()->restoreCache();
		if(res == eNO_ERROR)
		{
            QString rootPath = WebMounter::getSettingStorage()->getAppStoragePath() + QString(QDir::separator()) + m_pluginName;
			QFileInfo fInfo(rootPath);
			//syncCacheWithFileSystem(fInfo.absoluteFilePath());
		}
	}

	VkRVFSDriver::~VkRVFSDriver(void)
	{
        delete m_httpConnector;
	}

	RESULT VkRVFSDriver::downloadFiles(QList <QString>& urlList, QList <QString>& pathList)
	{
		VFSCache* vfsCache = WebMounter::getCache();
		int initialListSize = urlList.size();

		VFSCache::iterator iter = vfsCache->end();
		for(int i = 0; i < pathList.size(); ++i)
		{
			iter = vfsCache->find(pathList.at(i));
			if(iter != vfsCache->end()
				&& !(iter->getFlags() & VFSElement::eFl_Downloading)
				&& !(iter->getFlags() & VFSElement::eFl_Downloaded))
			{	
				vfsCache->setFlag(iter, VFSElement::eFl_Downloading, VFSElement::eFl_None);

				QFile::Permissions permissions = QFile::permissions(pathList.at(i));
				permissions |= (QFile::WriteGroup|QFile::WriteOwner|QFile::WriteUser|QFile::WriteOther);
				QFile::setPermissions(pathList.at(i), permissions);
				QFile::remove(pathList.at(i));
			}
			else
			{
				urlList.removeAt(i);
				pathList.removeAt(i);
				i = 0;
			}
		}

		if(urlList.size() == 0) // there is nothing to download
		{
			if(initialListSize == 1
				&& iter != vfsCache->end() 
				&& iter->getFlags() & VFSElement::eFl_Downloaded) // download request has been recieved for one file, but file is already downloaded
			{	
				return eNO_ERROR;
			}
			else
			{
				return eERROR_GENERAL;
			}
		}

        RESULT res = m_httpConnector->downloadFiles(urlList, pathList);
		if(res != eNO_ERROR)
		{
			unsigned int retryDownloadCounter = 0;
			while(res != eNO_ERROR && retryDownloadCounter < MAX_DOWNLOAD_RETRY)
			{
                res = m_httpConnector->downloadFiles(urlList, pathList);
				retryDownloadCounter++;
			}
		}

		for(int i = 0; i < pathList.size(); i++)
		{
			QFileInfo fInfo(pathList.at(i));
			VFSCache::iterator iter = vfsCache->find(fInfo.absoluteFilePath());

			if(iter != vfsCache->end())
			{
				if(res != eNO_ERROR)
				{
					vfsCache->setFlag(iter, VFSElement::eFl_None, VFSElement::eFl_Downloading);
				}
				else
				{
					QFile::Permissions permissions = QFile::permissions(pathList.at(i));
					permissions &= ~(QFile::WriteGroup|QFile::WriteOwner|QFile::WriteUser|QFile::WriteOther);
					QFile::setPermissions(pathList.at(i), permissions);
					vfsCache->setFlag(iter, VFSElement::eFl_Downloaded, VFSElement::eFl_Downloading);
				}
			}
		}

		return res;
	}

	RESULT VkRVFSDriver::uploadFile(const QString& path, const QString& title, const QString& id, const QString& parentId)
	{
		RESULT err = eERROR_GENERAL;
		int count = 0;

		QFileInfo fInfo(path);
		QString suffix = fInfo.suffix();
		suffix = suffix.toLower();

		if(getState() != eConnected)
		{
			QDir dir;
			dir.remove(fInfo.absoluteFilePath());

			notifyUser(Ui::Notification::eINFO, QObject::tr("Info"),QObject::tr("Plugin is not connected !\nPlugin has to be in connected state.\n"));
			WebMounter::getProxy()->fileUploaded(fInfo.absoluteFilePath(), eNO_ERROR);
			return eNO_ERROR;
		}

		if(suffix != "jpg" && suffix != "jpeg" && suffix != "png" && suffix != "gif")
		{
			notifyUser(Ui::Notification::eINFO, RemoteDriver::VkRVFSDriver::tr("Info"), RemoteDriver::VkRVFSDriver::tr("This file extension is not supported !\n"));
			WebMounter::getProxy()->fileUploaded(fInfo.absoluteFilePath(), eERROR_GENERAL);
			return eERROR_GENERAL;
		}
		else if(id != ROOT_ID)
		{
			notifyUser(Ui::Notification::eINFO, tr("Info"), tr("This file will be changed locally!\n"));
			WebMounter::getProxy()->fileUploaded(fInfo.absoluteFilePath(), eERROR_GENERAL);
			return eERROR_GENERAL;
		}
		else if(parentId == ROOT_ID)
		{
			QDir dir;
			dir.remove(fInfo.absoluteFilePath());

			notifyUser(Ui::Notification::eINFO, tr("Info"), tr("You can not upload photos into the root directory!\n"));
			WebMounter::getProxy()->fileUploaded(fInfo.absoluteFilePath(), eERROR_NOT_SUPPORTED);
			return eERROR_GENERAL;
		}
		else if (fInfo.size() == 0)
		{
			WebMounter::getProxy()->fileUploaded(fInfo.absoluteFilePath(), eNO_ERROR);
			return eNO_ERROR;
		}

		VFSCache::iterator iter = WebMounter::getCache()->begin();
		for(iter; iter != WebMounter::getCache()->end(); ++iter)
		{
            if((iter->getPluginName() == m_pluginName)
				&&(iter->getFlags()&VFSElement::eFl_SelfMade)
				&&(iter->getType() == VFSElement::FILE)
				&&(iter->getParentId() == parentId)
				)
			{
				count++;
			}
		}


		QString xmlResp;
		unsigned int retryUploadCounter = 0;

        err = m_httpConnector->uploadFile(path, title, parentId, xmlResp);
		while(err != eNO_ERROR && retryUploadCounter < MAX_UPLOAD_RETRY)
		{
            err = m_httpConnector->uploadFile(path, title, parentId, xmlResp);
			retryUploadCounter++;
		}

		VFSElement elem;
		if(!err && xmlResp != "")
		{
			VFSCache* vfsCache = WebMounter::getCache();
			parsePhotoEntry(xmlResp, elem);

			fInfo.setFile(fInfo.absolutePath() + QString(QDir::separator()) + elem.getName());

			elem.setPath(fInfo.absoluteFilePath());

			elem.setFlags(VFSElement::eFl_SelfMade | VFSElement::eFl_Downloaded);

			vfsCache->insert(elem);

			QDir dir;
			dir.rename(path, elem.getPath());

			QFile::Permissions permissions = QFile::permissions(elem.getPath());
			permissions &= ~(QFile::WriteGroup|QFile::WriteOwner|QFile::WriteUser|QFile::WriteOther);
			QFile::setPermissions(elem.getPath(), permissions);
		}
		else
		{
			notifyUser(Ui::Notification::eCRITICAL
				, RemoteDriver::VkRVFSDriver::tr("Error")
				, RemoteDriver::VkRVFSDriver::tr("File upload failed (") + elem.getName() + QString(")"));

		}

		fInfo.setFile(path);
		WebMounter::getProxy()->fileUploaded(fInfo.absoluteFilePath(), err);


		if(err)
		{
			QDir dir;
			dir.remove(path);
		}

		return err;
	}

	RESULT VkRVFSDriver::modifyFile(const QString&)
	{
		return eERROR_GENERAL;
	}

	RESULT VkRVFSDriver::renameElement(const QString& path, const QString& newTitle)
	{
		RESULT res = eERROR_GENERAL;
		QFileInfo qInfo(path);
		VFSCache* cache = WebMounter::getCache();
		VFSCache::iterator elem = cache->find(qInfo.absoluteFilePath());

		if(elem != cache->end())
		{
			if(elem->getType() == VFSElement::FILE)
			{
				return res; // Renaming of photos is not supported by Vkontakte
			}

            res = m_httpConnector->renameAlbum(elem->getId(), newTitle);
			if(res == eNO_ERROR)
			{
				qInfo = qInfo.dir().absolutePath() + QString(QDir::separator()) + newTitle;
				VFSElement newElem(elem->getType()
					, qInfo.absoluteFilePath()
					, newTitle
					, elem->getEditMetaUrl()
					, elem->getEditMediaUrl()
					, elem->getSrcUrl()
					, elem->getId()
					, elem->getParentId()
					, elem->getModified()
					, elem->getPluginName()
					, elem->getFlags());

				cache->erase(elem);
				cache->insert(newElem);

				if(newElem.getType() == VFSElement::DIRECTORY)
				{
					updateChildrenPath(newElem);
				}
			}
		}

		return res;
	}

	RESULT VkRVFSDriver::deleteDirectory(const QString& path)
	{
		RESULT res = eNO_ERROR;
		QDir qDirFrom(path);
		VFSCache* cache = WebMounter::getCache();
		VFSCache::iterator elem = cache->find(qDirFrom.absolutePath());
		if(elem != cache->end() && elem->getType() == VFSElement::DIRECTORY)
		{
            res = m_httpConnector->deleteAlbum(elem->getId());
			if(res == eNO_ERROR)
			{
				cache->erase(elem);
			}
		}
		return res;
	}

	RESULT VkRVFSDriver::deleteFile(const QString& path)
	{
		RESULT res = eERROR_GENERAL;
		QFileInfo qInfo(path);
		VFSCache* cache = WebMounter::getCache();
		VFSCache::iterator elem = cache->find(qInfo.absoluteFilePath());

		if(elem != cache->end())
		{
			QString response;
            res = res = m_httpConnector->deleteFile(elem->getId());
			if(res == eNO_ERROR)
			{
				WebMounter::getProxy()->fileDeleted(elem->getPath(), res);
				cache->erase(elem);
			}
		}

		return res;
	}

	RESULT VkRVFSDriver::moveElement(const QString& path, const QString& newParentId)
	{
		RESULT res = eERROR_GENERAL;
		QFileInfo qInfo(path);
		VFSCache* cache = WebMounter::getCache();
		VFSCache::iterator elem = cache->find(qInfo.absoluteFilePath());

		if(elem != cache->end())
		{
			if(elem->getType() == VFSElement::DIRECTORY)
			{
				return res; // Moving of albums is not supported
			}

            res = m_httpConnector->moveFile(elem->getId(), elem->getParentId(), newParentId);
			if(res == eNO_ERROR)
			{
				VFSElement newElem(elem->getType()
					, elem->getPath()
					, elem->getName()
					, elem->getEditMetaUrl()
					, elem->getEditMediaUrl()
					, elem->getSrcUrl()
					, elem->getId()
					, newParentId
					, elem->getModified()
					, elem->getPluginName());

				VFSCache::iterator iter = cache->begin();
				for(iter; iter != cache->end(); ++iter)
				{
					if(iter->getId() == newParentId)
						break;
				}

				QString path = iter->getPath() + QString(QDir::separator()) + newElem.getName();
				QFileInfo fInfo(path);
				newElem.setPath(fInfo.absoluteFilePath());

				cache->erase(elem);
				cache->insert(newElem);
			}
		}

		return res;
	}

	RESULT VkRVFSDriver::createDirectory(const QString& /*path*/, const QString& parentId, const QString& title)
	{
		RESULT res = eERROR_GENERAL;
		int count = 0;
		VFSCache::iterator iter = WebMounter::getCache()->begin();
		for(iter; iter != WebMounter::getCache()->end(); ++iter)
		{
            if((iter->getPluginName() == m_pluginName)
				&&(iter->getFlags()&VFSElement::eFl_SelfMade)
				&&(iter->getType() == VFSElement::DIRECTORY))
			{
				count++;
			}
		}

		if(parentId != ROOT_ID)
		{
			return eERROR_GENERAL; // Creation of sub-albums is not supported
		}

		QString xmlResp;
        res = m_httpConnector->createDirectory(title, xmlResp);
		if(!res)
		{
			VFSCache* vfsCache = WebMounter::getCache();
			VFSElement elem;
			parseAlbumEntry(xmlResp, elem);
			elem.setParentId(ROOT_ID);

            QString pluginStoragePath = WebMounter::getSettingStorage()->getAppStoragePath() + QString(QDir::separator()) + m_pluginName;
			QString path = pluginStoragePath + QString(QDir::separator()) + elem.getName();
			QFileInfo fInfo(path);
			elem.setPath(fInfo.absoluteFilePath());

			elem.setFlags(VFSElement::eFl_SelfMade);

			vfsCache->insert(elem);
		}

		return res;
	}

	RESULT VkRVFSDriver::getAlbums(QList<VFSElement>& elements, int& errorCode)
	{
        double albumsMaxProgress = m_progressData.m_currProgress + (m_progressData.m_maxValue - m_progressData.m_currProgress)/3;

		QString xmlResp;
        RESULT err = m_httpConnector->getAlbums(xmlResp, errorCode);
		if(!err)
		{
			{
                QMutexLocker locker(&m_driverMutex);
                if(m_state == eSyncStopping)
				{
					notifyUser(Ui::Notification::eINFO, tr("Info"), tr("Syncing is stopped !\n"));
					return eERROR_CANCEL;
				}
			}

            m_progressData.m_currProgress += 5;
            updateSyncStatus(m_progressData.m_currProgress);

			QString pattern = QString::fromAscii("<album>(.*)</album>");
			QRegExp rx(pattern);
			rx.setCaseSensitivity(Qt::CaseSensitive);
			rx.setMinimal(true);
			rx.setPatternSyntax(QRegExp::RegExp);

			int pos = 0;
			while((pos = rx.indexIn(xmlResp, pos)) != -1)
			{
				VFSElement elem;
				QString cap(rx.cap(1));

				parseAlbumEntry(cap, elem);

				elem.setDownloaded(false);
				elem.setParentId(ROOT_ID);

                QString pluginStoragePath = WebMounter::getSettingStorage()->getAppStoragePath() + QString(QDir::separator()) + m_pluginName;
				QString path = pluginStoragePath + QString(QDir::separator()) + elem.getName();
				QFileInfo fInfo(path);
				elem.setPath(fInfo.absoluteFilePath());

				elements.append(elem);

				pos += rx.matchedLength();

                m_progressData.m_currProgress += 1;
                updateSyncStatus(m_progressData.m_currProgress);
			}
		}

		if(err == eNO_ERROR)
		{
			handleNameDuplicates(elements);

            m_progressData.m_currProgress = albumsMaxProgress;
			updateSyncStatus(albumsMaxProgress);
		}

		return err;
	}

	int VkRVFSDriver::findParentIndex(const QList<VFSElement>& elemList, const VFSElement& elem)
	{
		int res = -1;
		for(int i=0; i<elemList.count(); i++)
		{
			if(elemList[i].getId() == elem.getParentId())
			{
				res = i;
				break;
			}
		}

		return res;
	}

	RESULT VkRVFSDriver::getPhotos(QList<VFSElement>& elements)
	{
		RESULT err = eERROR_GENERAL;
		int offset = 0;
		forever
		{
			{
                QMutexLocker locker(&m_driverMutex);
                if(m_state == eSyncStopping)
				{
					notifyUser(Ui::Notification::eINFO, tr("Info"), tr("Syncing is stopped !\n"));
					return eERROR_CANCEL;
				}
			}

			int count = 0; 
			QString xmlResp = "";
            err = m_httpConnector->getPhotos(offset, xmlResp);
			if(!err)
			{
				count = RegExp::getByPattern("<count>(.*)</count>", xmlResp).toInt();
                double progressPortion = (count == 0) ? 0 : (m_progressData.m_maxValue - m_progressData.m_currProgress)/count;

				QString pattern = QString::fromAscii("<photo>(.*)</photo>");
				QRegExp rx(pattern);
				rx.setCaseSensitivity(Qt::CaseSensitive);
				rx.setMinimal(true);
				rx.setPatternSyntax(QRegExp::RegExp);

				int pos = 0;
				while((pos = rx.indexIn(xmlResp, pos)) != -1)
				{
					VFSElement elem;
					QString cap(rx.cap(1));

					elem.setDownloaded(false);
					parsePhotoEntry(cap, elem);

					QString path;
					int i = findParentIndex(elements, elem);
					if(i < 0)
					{
						elem.setParentId(ROOT_ID);
                        QString pluginStoragePath = WebMounter::getSettingStorage()->getAppStoragePath() + QString(QDir::separator()) + m_pluginName;
						path = pluginStoragePath + QString(QDir::separator()) + elem.getName();
					}
					else 
					{
						path = elements[i].getPath() + QString(QDir::separator()) + elem.getName();
					}

					QFileInfo fInfo(path);
					elem.setPath(fInfo.absoluteFilePath());

					elements.append(elem);

					pos += rx.matchedLength();

                    m_progressData.m_currProgress += progressPortion;
                    updateSyncStatus(m_progressData.m_currProgress);
				}
			}

			offset +=100;
			if(offset >= count)
				break;
		}

		if(err == eNO_ERROR)
		{
			handleNameDuplicates(elements);
		}

		return err;
	}

	RESULT VkRVFSDriver::getElements(QList<VFSElement>& elements)
	{
		int errorCode;
		RESULT res = getAlbums(elements, errorCode);
		if(!res)
		{
			res = getPhotos(elements);
		}

		return res;
	}

	void VkRVFSDriver::parseAlbumEntry(QString& xmlEntry, VFSElement& elem)
	{
		elem.setType(VFSElement::DIRECTORY);
        elem.setPluginName(m_pluginName);

		QString title = QString::fromAscii("<title>(.*)</title>");
		QRegExp rx(title);
		rx.setCaseSensitivity(Qt::CaseSensitive);
		rx.setMinimal(true);
		rx.setPatternSyntax(QRegExp::RegExp);

		rx.indexIn(xmlEntry);
		elem.setName(rx.cap(1));

		QString url = QString::fromAscii("http://vkontakte.ru/album");

		rx.setPattern("<owner_id>(.*)</owner_id>");
		rx.indexIn(xmlEntry);
		url += rx.cap(1);

		rx.setPattern("<aid>(.*)</aid>");
		rx.indexIn(xmlEntry);
		elem.setId(rx.cap(1));
		url += QString::fromAscii("_") + rx.cap(1);

		elem.setSrcUrl(url);
	}

	void VkRVFSDriver::parsePhotoEntry(QString& xmlEntry, VFSElement& elem)
	{
		elem.setType(VFSElement::FILE);
        elem.setPluginName(m_pluginName);

		QString pid = QString::fromAscii("<pid>(.*)</pid>");
		QRegExp rx(pid);
		rx.setCaseSensitivity(Qt::CaseSensitive);
		rx.setMinimal(true);
		rx.setPatternSyntax(QRegExp::RegExp);

		rx.indexIn(xmlEntry);
		elem.setId(rx.cap(1));

		rx.setPattern("<aid>(.*)</aid>");
		rx.indexIn(xmlEntry);
		elem.setParentId(rx.cap(1));

		QString url;

		rx.setPattern("<src_xxxbig>(.*)</src_xxxbig>");
		rx.indexIn(xmlEntry);
		elem.setSrcUrl(rx.cap(1));
		url = elem.getSrcUrl();

		if( url.isEmpty() )
		{
			rx.setPattern("<src_xxbig>(.*)</src_xxbig>");
			rx.indexIn(xmlEntry);
			elem.setSrcUrl(rx.cap(1));
			url = elem.getSrcUrl();
		}

		if( url.isEmpty() )
		{
			rx.setPattern("<src_xbig>(.*)</src_xbig>");
			rx.indexIn(xmlEntry);
			elem.setSrcUrl(rx.cap(1));
			url = elem.getSrcUrl();
		}

		if( url.isEmpty() )
		{
			rx.setPattern("<src_big>(.*)</src_big>");
			rx.indexIn(xmlEntry);
			elem.setSrcUrl(rx.cap(1));
			url = elem.getSrcUrl();
		}

		int pos = url.lastIndexOf("/");
		elem.setName(url.right(url.length() - pos - 1));
	}

	void VkRVFSDriver::notifyUser(Ui::Notification::_Types type, QString title, QString description) const
	{
		Ui::NotificationDevice* notifDevice = WebMounter::getNotificationDevice();

		Ui::Notification msg(type, title, description);

		notifDevice->showNotification(msg);
	}

	void VkRVFSDriver::updateDownloadStatus(RESULT downloadResult, const unsigned int uDownloaded, const unsigned int uNotDownloaded)
	{
		if(!downloadResult)
		{			
			{
                QMutexLocker locker(&m_driverMutex);

                if(m_state != eSyncStopping)
				{
					updateState((int)(((float)uDownloaded/uNotDownloaded)*50) + 50, RemoteDriver::eSync);
				}
			}
		}
		else
		{
			notifyUser(Ui::Notification::eCRITICAL, tr("Error"), tr("Downloading failed  !\n"));

			{
                QMutexLocker locker(&m_driverMutex);
                if(m_state != eSyncStopping)
				{
					updateState(100, RemoteDriver::eNotConnected);
				}
			}
		}
	}

	void VkRVFSDriver::connectHandler(PluginSettings& pluginSettings)
	{
        QMutexLocker locker(&m_driverMutex);

        if(m_state == RemoteDriver::eNotConnected)
		{
			GeneralSettings generalSettings; 

			WebMounter::getSettingStorage()->getData(generalSettings);
			WebMounter::getSettingStorage()->addSettings(pluginSettings);

            m_httpConnector->setSettings(
                pluginSettings.m_userName
                , pluginSettings.m_userPassword
                , generalSettings.m_proxyAddress
                , generalSettings.m_proxyLogin + ":" + generalSettings.m_proxyPassword
                , pluginSettings.m_isOAuthUsing
                , pluginSettings.m_oAuthToken
				);

            if(m_state != eSyncStopping)
			{
				updateState(0, eAuthInProgress);
			}

			/*if(pluginSettings.isOAuthUsing && pluginSettings.oAuthToken != "")
			{
			updateState(100, eAuthorized);

			start(); // run sync thread				
			}
			else
			{
			updateState(0, eAuthInProgress);
			}*/
		}
	}

	void VkRVFSDriver::connectHandlerStage2(RESULT error, PluginSettings pluginSettings)
	{
        if(error == eNO_ERROR && m_state == eAuthInProgress)
		{
			GeneralSettings generalSettings; 

			WebMounter::getSettingStorage()->getData(generalSettings);
			WebMounter::getSettingStorage()->addSettings(pluginSettings);

            m_httpConnector->setSettings(
                pluginSettings.m_userName
                , pluginSettings.m_userPassword
                , generalSettings.m_proxyAddress
                , generalSettings.m_proxyLogin + ":" + generalSettings.m_proxyPassword
                , pluginSettings.m_isOAuthUsing
                , pluginSettings.m_oAuthToken
				);

            if(m_state != eSyncStopping)
			{
				updateState(100, eAuthorized);
			}

			start(); // run sync thread
		}
		else
		{
			updateState(100, eNotConnected);

			notifyUser(Ui::Notification::eCRITICAL, tr("Error"), tr("Authorization failed !\n"));
		}
	}

	void VkRVFSDriver::disconnectHandler()
	{
        m_driverMutex.lock();

		if(isRunning()) //if sync thread is running 
		{
			updateState(40, eSyncStopping);
            m_driverMutex.unlock();

            m_forceSync.wakeAll(); // we have to wake up the sync thread for safely termination

			while(isRunning()) // wait thread termination 
			{
				sleep(1);
			}

			//WebMounter::getCache()->restoreCache(); // restore cache from DB
		}
		else
		{
            m_driverMutex.unlock();
		}

		updateState(100, eNotConnected);
	}

	void VkRVFSDriver::syncHandler()
	{
        QMutexLocker locker(&m_driverMutex);

        if(m_state == eConnected)
		{
            m_forceSync.wakeAll();
		}
	}

	void VkRVFSDriver::stopSyncHandler()
	{
		if(isRunning()) //if sync thread is running 
		{
			PluginSettings pluginSettings;
            WebMounter::getSettingStorage()->getData(pluginSettings, m_pluginName);

            updateState(m_progressData.m_currProgress, eSyncStopping);

            m_forceSync.wakeAll();

			while(isRunning())
			{
				sleep(1);
			}
		}
		updateState(100, eConnected);
		start(); // start sync thread again
	}

	QString VkRVFSDriver::addPathSuffix(ElementType type, const QString& path, const QString& suffix)
	{
		QString res;
		int ind = path.lastIndexOf(".");
		if(ind > 0 && type == VFSElement::FILE)
		{
			res = path.left(ind) + "_" + suffix + path.right(path.length() - ind);
		}
		else
		{
			res = path + "_" + suffix;
		}

		return res;
	}

	void VkRVFSDriver::markNameDuplicates(QList<VFSElement>& elemList)
	{
		for(int i=0; i<elemList.count(); i++)
		{
			for(int k=0; k<elemList.count(); k++)
			{
				if(!(elemList[i].getFlags()&VFSElement::eFl_NameDup) 
					&& (i!=k) && (elemList[i].getName() == elemList[k].getName()) 
					&& (elemList[i].getParentId() == elemList[k].getParentId()))
				{
					elemList[i].setFlags(VFSElement::eFl_NameDup);
				}
			}
		}
	}

	void VkRVFSDriver::handleNameDuplicates(QList<VFSElement>& elemList)
	{
		for(int i=0; i<elemList.count(); i++)
		{
			for(int k=0; k<elemList.count(); k++)
			{
				if(!(elemList[i].getFlags()&VFSElement::eFl_NameDup) 
					&& (i!=k) && (elemList[i].getName() == elemList[k].getName()) 
					&& (elemList[i].getParentId() == elemList[k].getParentId()))
				{
					elemList[i].setFlags(VFSElement::eFl_NameDup);
					elemList[i].setPath(addPathSuffix(elemList[i].getType(), elemList[i].getPath(), elemList[i].getId()));
				}
			}
		}

		for(int i=0; i<elemList.count(); i++)
		{
			for(int k=0; k<elemList.count(); k++)
			{
				if((i!=k) && (elemList[i].getPath() == elemList[k].getPath()))
				{
					elemList[i].setFlags(VFSElement::eFl_NameDup);
					elemList[i].setPath(addPathSuffix(elemList[i].getType(), elemList[i].getPath(), elemList[i].getId()));
					for(int j=0; j<elemList.count(); j++)
					{
						if(elemList[j].getParentId() == elemList[i].getId())
						{
							QString path = elemList[i].getPath() + QString(QDir::separator()) + elemList[j].getName();
							QFileInfo fInfo(path);
							elemList[j].setPath(fInfo.absoluteFilePath());
						}
					}
				}
			}
		}
	}

	bool VkRVFSDriver::areFileAttributesValid(const QString& /*path*/, unsigned long /*attributes*/)
	{
		return true;///////(attributes & FILE_ATTRIBUTE_READONLY);
	}
}
