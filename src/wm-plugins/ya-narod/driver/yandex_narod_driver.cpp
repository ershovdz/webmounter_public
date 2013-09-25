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

#include <QRegExp>

#include "yandex_narod_driver.h"
#include "webmounter.h"
#include "reg_exp.h"

namespace RemoteDriver
{
	using namespace Common;
	using namespace Connector;

	YandexNarodRVFSDriver::YandexNarodRVFSDriver(const QString& pluginName)
	{
        m_state = RemoteDriver::eNotConnected;
        m_httpConnector = new YandexNarodHTTPConnector;
        m_pluginName = pluginName;

		RESULT res = WebMounter::getCache()->restoreCache();
		if(res == eNO_ERROR)
		{
            QString rootPath = WebMounter::getSettingStorage()->getAppStoragePath() + QString(QDir::separator()) + m_pluginName;
			QFileInfo fInfo(rootPath);
			//syncCacheWithFileSystem(fInfo.absoluteFilePath());
		}
	}

	YandexNarodRVFSDriver::~YandexNarodRVFSDriver(void)
	{
        delete m_httpConnector;
	}

	RESULT YandexNarodRVFSDriver::downloadFiles(QList <QString>& urlList, QList <QString>& pathList)
	{
		VFSCache* vfsCache = WebMounter::getCache();
		int initialListSize = urlList.size();

        { 	LOCK(m_driverMutex)

            VFSCache::iterator iter = vfsCache->end();
            for(int i = 0; i < pathList.size(); ++i)
            {
                iter = vfsCache->find(pathList.at(i));
                if(iter != vfsCache->end()
                        && !(iter->getFlags() & VFSElement::eFl_Downloading)
                        && !(iter->getFlags() & VFSElement::eFl_Downloaded))
                {
                    vfsCache->setFlag(iter, VFSElement::eFl_Downloading, VFSElement::eFl_None);
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

        { 	LOCK(m_driverMutex)

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
		}
		return res;
	}

	RESULT YandexNarodRVFSDriver::uploadFile(const QString& path, const QString& title,  const QString& /*Id*/, const QString& parentId)
	{
		QString xmlResp;
		QString id = ROOT_ID;
		QFileInfo fInfo(path);

        RESULT err = m_httpConnector->uploadFile(path, title, parentId, xmlResp);
		unsigned int retryUploadCounter = 0;

		while(err != eNO_ERROR && retryUploadCounter < MAX_UPLOAD_RETRY)
		{
            err = m_httpConnector->uploadFile(path, title, parentId, xmlResp);
			retryUploadCounter++;
		}

		VFSElement elem;
		if(!err && xmlResp != "")
		{
			id = RegExp::getByPattern("\"fids\": \"(.*)\"", xmlResp);
			if(id != "")
			{
				// Trying to get new element from server to retrieve the data-token
				QList<VFSElement> elements;
				err = getElements(elements); 
				if(err == eNO_ERROR)
				{
					int i=0;
					for(i=0; i<elements.count(); i++)
					{
						if(elements[i].getId() == id)
							break;
					}

					QString hash = RegExp::getByPattern("\"hash\": \"(.*)\",", xmlResp);

					VFSCache* vfsCache = WebMounter::getCache();
					elem = VFSElement(VFSElement::FILE
						, fInfo.absoluteFilePath()
						, title
						, ""
						, (i<elements.count() ? elements[i].getEditMediaUrl() : "")
						, "http://narod.ru/disk/" + hash + "/" + fInfo.fileName() + ".html"
						, id
						, parentId
						, ""
                        , m_pluginName);

					vfsCache->insert(elem);

					QFile::Permissions permissions = QFile::permissions(elem.getPath());
					permissions &= ~(QFile::WriteGroup|QFile::WriteOwner|QFile::WriteUser|QFile::WriteOther);
					QFile::setPermissions(elem.getPath(), permissions);
				}
			}
		}
		else
		{
			notifyUser(Ui::Notification::eCRITICAL
				, tr("Error")
				, tr("File upload failed (") + elem.getName() + QString(")"));
		}

		WebMounter::getProxy()->fileUploaded(fInfo.absoluteFilePath(), err);
		return err;
	}

	RESULT YandexNarodRVFSDriver::modifyFile(const QString&)
	{
		return eERROR_NOT_SUPPORTED;
	}

	RESULT YandexNarodRVFSDriver::renameElement(const QString& /*path*/, const QString& /*newTitle*/)
	{
		RESULT res = eERROR_NOT_SUPPORTED;
		return res;
	}

	RESULT YandexNarodRVFSDriver::deleteDirectory(const QString& /*path*/)
	{
		RESULT res = eERROR_NOT_SUPPORTED;
		return res;
	}

	RESULT YandexNarodRVFSDriver::deleteFile(const QString& path)
	{
		RESULT res = eERROR_GENERAL;
		QFileInfo qInfo(path);
		VFSCache* cache = WebMounter::getCache();
		VFSCache::iterator elem = cache->find(qInfo.absoluteFilePath());

		if(elem != cache->end())
		{
			QString response;
            res = m_httpConnector->deleteFile(elem->getId(), elem->getEditMediaUrl());
			if(res == eNO_ERROR)
			{
				WebMounter::getProxy()->fileDeleted(elem->getPath(), res);
				cache->erase(elem);
			}
		}

		return res;
	}

	RESULT YandexNarodRVFSDriver::moveElement(const QString& /*path*/, const QString& /*newParentId*/)
	{
		RESULT res = eERROR_NOT_SUPPORTED;
		return res;
	}

	RESULT YandexNarodRVFSDriver::createDirectory(const QString& /*path*/, const QString& /*parentId*/, const QString& /*title*/)
	{
		RESULT err = eERROR_NOT_SUPPORTED;
		return err;
	}

	RESULT YandexNarodRVFSDriver::getElements(QList<VFSElement>& elements)
	{
		RESULT err = eERROR_GENERAL;
		QString xmlResp = "";
        err = m_httpConnector->getFiles(xmlResp);

		if(!err)
		{
			xmlResp.replace("<wbr/>", "");
			int cpos=0;
			QRegExp rx("class=\"\\S+icon\\s(\\S+)\"[^<]+<img[^<]+</i[^<]+</td[^<]+<td[^<]+<input[^v]+value=\"(\\d+)\"[^<]+</td[^<]+<td[^<]+<span\\sclass='b-fname'><a\\shref=\"(\\S+)\">([^<]+)");
			cpos = rx.indexIn(xmlResp);
			while (cpos>0)
			{
				VFSElement elem;
				elem.setType(VFSElement::FILE);
                elem.setPluginName(m_pluginName);

				elem.setName(rx.cap(4));
				elem.setId(rx.cap(2));
				elem.setSrcUrl(rx.cap(3));

				elem.setParentId(ROOT_ID);
                QString pluginStoragePath = WebMounter::getSettingStorage()->getAppStoragePath() + QString(QDir::separator()) + m_pluginName;
				QString path = pluginStoragePath + QString(QDir::separator()) + elem.getName();
				QFileInfo fInfo(path);
				elem.setPath(fInfo.absoluteFilePath());

				QString pattern = QString("input type=\"checkbox\" name=\"fid\" value=\"%1\" data-token=\"(.*)\"").arg(elem.getId());
				elem.setEditMediaUrl(RegExp::getByPattern(pattern, xmlResp));

				elements.append(elem);

				cpos = rx.indexIn(xmlResp, cpos+1);
			}
		}

		return err;
	}

	RESULT YandexNarodRVFSDriver::getElements()
	{
		RESULT res = eERROR_GENERAL;
		return res;
	}

	RESULT YandexNarodRVFSDriver::sync()
	{
		RESULT res = eERROR_GENERAL;
		SettingStorage* settings = WebMounter::getSettingStorage();
        QString pluginStoragePath = settings->getAppStoragePath() + QString(QDir::separator()) + m_pluginName;
		QFileInfo fInfo(pluginStoragePath);
		unsigned int uNotDownloaded = 0;
		PluginSettings plSettings;
        settings->getData(plSettings, m_pluginName);

        m_driverMutex.lock();
        if(m_state != eSyncStopping)
		{
			updateState(0, RemoteDriver::eSync);
		}
        m_driverMutex.unlock();

		QList<VFSElement> elements;

		VFSElement elem = VFSElement(VFSElement::DIRECTORY
			, fInfo.absoluteFilePath()
			, "ROOT"
			, ""
			, ""
			, ""
			, ROOT_ID
			, ROOT_ID
			, ""
            , m_pluginName);

		elements.append(elem);

		res = getElements(elements);
		if(!res)
		{
			QDir qDir;
			QFile qFile;

			uNotDownloaded = elements.count();
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
					if(!found)
					{
						VFSCache::iterator iter1 = iter--;
						vfsCache->erase(iter1);
					}
				}
			}

            { 	LOCK(m_driverMutex);
            if(m_state != eSyncStopping)
			{
				updateState(30, RemoteDriver::eSync);
			}
			}

			// Add newly created elements
			for(int i=0; i<elements.count(); i++)
			{
				vfsCache->insert(elements[i]);
			}

            { 	LOCK(m_driverMutex);
            if(m_state != eSyncStopping)
			{
				updateState(40, RemoteDriver::eSync);
			}
			}

            QString rootPath = WebMounter::getSettingStorage()->getAppStoragePath() + QString(QDir::separator()) + m_pluginName;
			QFileInfo fInfo(rootPath);

			syncCacheWithFileSystem(fInfo.absoluteFilePath());

            { 	LOCK(m_driverMutex);
            if(m_state != eSyncStopping)
			{
				updateState(50, RemoteDriver::eSync);
			}
			}

            if(plSettings.m_fullSync)
			{
				res = downloadFiles();
			}
		}

		if(res != eNO_ERROR)
		{
			stopPlugin();
			notifyUser(Ui::Notification::eCRITICAL, tr("Error"), tr("Sync failed !\n"));
			//updateState(100, RemoteDriver::eNotConnected);
		}
		else
		{
            m_driverMutex.lock();
            if(m_state != eSyncStopping)
			{
				updateState(100, eConnected);
			}
            m_driverMutex.unlock();
		}

		return res;
	}

	void YandexNarodRVFSDriver::run()
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

            m_syncMutex.lock();

            m_forceSync.wait(&m_syncMutex, sync_period * 1000);

            m_syncMutex.unlock();

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

	void YandexNarodRVFSDriver::connectHandler(PluginSettings& pluginSettings)
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
				);

            if(m_state != eSyncStopping)
			{
				updateState(0, eAuthInProgress);
			}

            if(m_httpConnector->auth() == eNO_ERROR)
			{
                if(m_state != eSyncStopping)
				{
					updateState(100, eAuthorized);
				}

				start(); // run sync thread
			}
			else
			{
				updateState(100, eNotConnected);

				notifyUser(Ui::Notification::eCRITICAL, tr("Error"), tr("Authorization failed !\n"
					"Please check proxy settings on Configuration tab and check settings on corresponding plugin tab...\n"));
			}
		}
	}

	void YandexNarodRVFSDriver::disconnectHandler()
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

	void YandexNarodRVFSDriver::syncHandler()
	{
        QMutexLocker locker(&m_driverMutex);

        if(m_state == eConnected)
		{
            m_forceSync.wakeAll();
		}
	}

	void YandexNarodRVFSDriver::stopSyncHandler()
	{
		if(isRunning()) //if sync thread is running 
		{
			PluginSettings pluginSettings;
            WebMounter::getSettingStorage()->getData(pluginSettings, m_pluginName);

			updateState(40, eSyncStopping);

            m_forceSync.wakeAll();

			while(isRunning())
			{
				sleep(1);
			}

			updateState(80, eSyncStopping);

			//WebMounter::getCache()->restoreCache(); // restore cache from DB
		}
		updateState(100, eConnected);
		start(); // start sync thread again
	}

	bool YandexNarodRVFSDriver::areFileAttributesValid(const QString& /*path*/, unsigned long /*attributes*/)
	{
		return true;///////////(attributes & FILE_ATTRIBUTE_READONLY);
	}
}
