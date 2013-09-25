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
#include <QtXml>

#include "yandex_disk_driver.h"
#include "webmounter.h"

namespace RemoteDriver
{
	using namespace Common;
	using namespace Connector;

	YaDiskRVFSDriver::YaDiskRVFSDriver(const QString& pluginName)
	{
        m_state = RemoteDriver::eNotConnected;
        m_httpConnector = new YaDiskHTTPConnector;
        m_pluginName = pluginName;

		RESULT res = WebMounter::getCache()->restoreCache();
		if(res == eNO_ERROR)
		{
            QString rootPath = WebMounter::getSettingStorage()->getAppStoragePath() + QString(QDir::separator()) + m_pluginName;
			QFileInfo fInfo(rootPath);
			//syncCacheWithFileSystem(fInfo.absoluteFilePath());
		}
	}

	YaDiskRVFSDriver::~YaDiskRVFSDriver(void)
	{
        delete m_httpConnector;
	}

	RESULT YaDiskRVFSDriver::getElements() 
	{ 
		return eNO_ERROR;
	}

	RESULT YaDiskRVFSDriver::downloadFiles(QList <QString>& urlList, QList <QString>& pathList)
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

	RESULT YaDiskRVFSDriver::uploadFile(const QString& path, const QString& title,  const QString& /*Id*/, const QString& parentId)
	{
		QString xmlResp;
		QString id = ROOT_ID;
		QFileInfo fInfo(path);
		if (fInfo.size() == 0)
		{
			return eNO_ERROR;
		}

        RESULT err = m_httpConnector->uploadFile(path, title, parentId, xmlResp);
		unsigned int retryUploadCounter = 0;

		while(err != eNO_ERROR && retryUploadCounter < MAX_UPLOAD_RETRY)
		{
            err = m_httpConnector->uploadFile(path, title, parentId, xmlResp);
			retryUploadCounter++;
		}

		VFSElement elem;
		if(!err)
		{
			QFileInfo fInfo(path);
			id = fInfo.fileName();

			VFSCache* vfsCache = WebMounter::getCache();
			elem = VFSElement(VFSElement::FILE
				, fInfo.absoluteFilePath()
				, title
				, ""
				, ""
				, id
				, id
				, parentId
				, ""
                , m_pluginName);

			vfsCache->insert(elem);

			QFile::Permissions permissions = QFile::permissions(elem.getPath());
			permissions &= ~(QFile::WriteGroup|QFile::WriteOwner|QFile::WriteUser|QFile::WriteOther);
			QFile::setPermissions(elem.getPath(), permissions);
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

	RESULT YaDiskRVFSDriver::modifyFile(const QString&)
	{
		return eERROR_GENERAL;
	}

	RESULT YaDiskRVFSDriver::renameElement(const QString& path, const QString& newTitle)
	{
		RESULT res = eERROR_GENERAL;
		QFileInfo qInfo(path);
		VFSCache* cache = WebMounter::getCache();
		VFSCache::iterator elem = cache->find(qInfo.absoluteFilePath());

		if(elem != cache->end())
		{
			QString response;
            res = m_httpConnector->renameElement(elem->getId(), elem->getType(), newTitle, response);
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
					, elem->getPluginName());

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

	RESULT YaDiskRVFSDriver::deleteDirectory(const QString& path)
	{
		/*RESULT res = eERROR_GENERAL;
		QDir qDirFrom(path);
		VFSCache* cache = WebMounter::getCache();
		VFSCache::iterator elem = cache->find(qDirFrom.absolutePath());

		if(elem != cache->end() && elem->getType() == VFSElement::DIRECTORY)
		{
		QString response;
        res = m_httpConnector->deleteFile("album/" + elem->getId() + "/", response);
		if(res == eNO_ERROR)
		{
		cache->erase(elem);
		}
		}*/


		return deleteFile(path);
	}

	RESULT YaDiskRVFSDriver::deleteFile(const QString& path)
	{
		RESULT res = eERROR_GENERAL;
		QFileInfo qInfo(path);
		VFSCache* cache = WebMounter::getCache();
		VFSCache::iterator elem = cache->find(qInfo.absoluteFilePath());

		if(elem != cache->end())
		{
			QString response;
            res = m_httpConnector->deleteFile(elem->getId(), response);
			if(res == eNO_ERROR)
			{
				WebMounter::getProxy()->fileDeleted(elem->getPath(), res);
				cache->erase(elem);
			}
		}

		return res;
	}

	RESULT YaDiskRVFSDriver::moveElement(const QString& path, const QString& newParentId)
	{
		RESULT res = eERROR_GENERAL;
		QFileInfo qInfo(path);
		VFSCache* cache = WebMounter::getCache();
		VFSCache::iterator elem = cache->find(qInfo.absoluteFilePath());

		if(elem != cache->end())
		{
			QString response;
            res = m_httpConnector->moveElement(elem->getId(), elem->getParentId(), newParentId, elem->getType(), response);
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

	RESULT YaDiskRVFSDriver::createDirectory(const QString& path, const QString& parentId, const QString& title)
	{
		QString xmlResp;
        RESULT err = m_httpConnector->createDirectory(title, parentId, xmlResp);
		if(!err)
		{
			VFSElement elem;
			elem.setType(VFSElement::DIRECTORY);
            elem.setPluginName(m_pluginName);

			elem.setName(title);

			elem.setId(title);

			elem.setModified(QDateTime::currentDateTime().toString());

			elem.setSrcUrl(title);

			elem.setPath(path);

			WebMounter::getCache()->insert(elem);
		}

		return err;
	}

	QString YaDiskRVFSDriver::getElementPath(QList<VFSElement>& elements, VFSElement& element)
	{
		QString parentId = element.getParentId();
		QString rootPath = QFileInfo(Common::WebMounter::getSettingStorage()->getAppStoragePath() 
			+ QDir::separator() 
            + m_pluginName).absoluteFilePath();

		if(ROOT_ID == element.getId())
		{
			return element.getPath();
		}
		if(ROOT_ID == parentId)
		{
			return QFileInfo(rootPath + QDir::separator() + element.getName())
				.absoluteFilePath();	
		}
		else
		{
			for (int i = 0; i < elements.count(); ++i) 
			{
				if(elements[i].getId() == parentId)
				{
					return QFileInfo(getElementPath(elements, elements[i]) + QDir::separator() + element.getName())
						.absoluteFilePath();		
				}
			}

		}
		return "";
	}

	RESULT YaDiskRVFSDriver::getElements(QList<VFSElement>& elements)
	{
		RESULT err = eERROR_GENERAL;
		for(int i=0; i<elements.count()&&elements[i].getType()==VFSElement::DIRECTORY; i++)
		{
			QString xmlResp = "";
			QString path = elements[i].getSrcUrl();
            err = m_httpConnector->getTreeElements(path, xmlResp);
			if(err || xmlResp.length() == 0)
			{
				continue;
			}

			QDomDocument xml;
			xml.setContent(xmlResp);
			QDomElement  root         = xml.firstChildElement(); // <multistatus> root element
			QDomNode  responseElement = root.firstChildElement(); // <response>

			while(!responseElement.isNull())
			{
				VFSElement elem;

				parseEntry(responseElement, elem);
				if(elem.getSrcUrl() != elements[i].getSrcUrl()) // skip duplicates
				{
					elem.setParentId(elements[i].getId());
					QString path = elements[i].getPath() + QString(QDir::separator()) + elem.getName();
					QFileInfo fInfo(path);
					elem.setPath(fInfo.absoluteFilePath());

					elem.setDownloaded(false);
					elements.append(elem);
				}

				responseElement = responseElement.nextSibling(); //next element
			}
		}

		if(err == eNO_ERROR)
		{
			//Populate path for elemenets
			for(int k=0, count = elements.count(); k < count; k++)
			{
				elements[k].setPath(QFileInfo(getElementPath(elements, elements[k])).absoluteFilePath());
			}

			handleNameDuplicates(elements);
		}

		return err;
	}

	int YaDiskRVFSDriver::findParentIndex(const QList<VFSElement>& elemList, const VFSElement& elem)
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

	void YaDiskRVFSDriver::parseEntry(QDomNode& xmlEntry, VFSElement& elem)
	{
		QString temp;

		int a = xmlEntry.toElement().elementsByTagName("d:resourcetype").item(0).childNodes().count();
		if(a > 0)
		{
			elem.setType(VFSElement::DIRECTORY);
		}
		else
		{
			elem.setType(VFSElement::FILE);
		}

        elem.setPluginName(m_pluginName);

		elem.setName(xmlEntry.toElement().elementsByTagName("d:displayname").item(0).toElement().text());

		QString tmp = xmlEntry.toElement().elementsByTagName("d:href").item(0).toElement().text();
		if(tmp.left(1) == "/" || tmp.left(1) == "\\")
		{
			tmp = tmp.mid(1);
		}

		if(tmp.right(1) == "/" || tmp.right(1) == "\\")
		{
			tmp = tmp.mid(0, tmp.length() - 1);
		}

		elem.setId(tmp.lastIndexOf("/") != -1 ? tmp.mid(tmp.lastIndexOf("/") + 1) : tmp);

		elem.setModified(xmlEntry.toElement().elementsByTagName("d:getlastmodified").item(0).toElement().text());

		elem.setSrcUrl(tmp);
	}

	RESULT YaDiskRVFSDriver::sync()
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

		// Handle albums
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

	void YaDiskRVFSDriver::run()
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

	void YaDiskRVFSDriver::connectHandler(PluginSettings& pluginSettings)
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

	void YaDiskRVFSDriver::connectHandlerStage2(RESULT error, PluginSettings pluginSettings)
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

			notifyUser(Ui::Notification::eCRITICAL, tr("Error"), tr("Authorization failed !\n"
				"Please check proxy settings on Configuration tab and check settings on corresponding plugin tab...\n"));
		}
	}

	void YaDiskRVFSDriver::disconnectHandler()
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

	void YaDiskRVFSDriver::syncHandler()
	{
        QMutexLocker locker(&m_driverMutex);

        if(m_state == eConnected)
		{
            m_forceSync.wakeAll();
		}
	}

	void YaDiskRVFSDriver::stopSyncHandler()
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

	QString YaDiskRVFSDriver::addPathSuffix(ElementType type, const QString& path, const QString& suffix)
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

	void YaDiskRVFSDriver::markNameDuplicates(QList<VFSElement>& elemList)
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

	void YaDiskRVFSDriver::handleNameDuplicates(QList<VFSElement>& elemList)
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

	bool YaDiskRVFSDriver::areFileAttributesValid(const QString& /*path*/, unsigned long /*attributes*/)
	{
		return true;///////////(attributes & FILE_ATTRIBUTE_READONLY);
	}
}
