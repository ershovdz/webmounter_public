#include "common_stuff.h"
#include "google_driver.h"
#include "webmounter.h"

#include <QFile>
#include <QTime>
#include <QDir>

namespace RemoteDriver
{
	using namespace Common;
	using namespace Data;

	GoogleRVFSDriver::GoogleRVFSDriver(const QString& pluginName) : _atomParser(pluginName)
	{
		_state = RemoteDriver::eNotConnected;
		_pluginName = pluginName; 

		RESULT res = WebMounter::getCache()->restoreCache();
		if(res == eNO_ERROR)
		{
			QString rootPath = WebMounter::getSettingStorage()->getAppStoragePath() + QString(QDir::separator()) + _pluginName;
			QFileInfo fInfo(rootPath);
		}
	}

	GoogleRVFSDriver::GoogleRVFSDriver(const QString& pluginName
		, const QString& rootPath
		, const QString& url
		, const QString& login
		, const QString& password
		, const QString& proxy//"proxy.te.mera.ru:8080"
		, const QString& proxyLoginPwd) : _atomParser(pluginName)//:  _rootPath(rootPath)
	{
		_state = RemoteDriver::eNotConnected;
		_pluginName = pluginName; 
		RESULT res = WebMounter::getCache()->restoreCache();
	}

	GoogleRVFSDriver::~GoogleRVFSDriver(void)
	{

	}

	RESULT GoogleRVFSDriver::createFile(const QString& path, const QString& title,  const QString& id, const QString& parentid)
	{
		QString xmlResp;
		QFileInfo fInfo(path);

		QString suffix = fInfo.suffix();
		suffix = suffix.toLower();

		if(_atomParser.checkExtension(suffix) == eERROR)
		{
			notifyUser(Ui::Notification::eINFO, tr("Info"), tr("This file extension is not supported !\n"));
			return eERROR;
		}

		if(suffix == "pdf") // it is not allowed to create pdf files, only upload an existing file
		{
			return eNO_ERROR;
		}

		if(_httpConnector.createFile(path, title, parentid, xmlResp) == eNO_ERROR)
		{
			VFSElement element;
			QString etag;
			if(_atomParser.parseEntry(xmlResp, element) == eNO_ERROR
				&& getFileEtag(element.getId(), etag) == eNO_ERROR)
			{
				element.setModified(etag);
				VFSCache* vfsCache = WebMounter::getCache();
				VFSCache::iterator iter = vfsCache->begin();
				for(iter; iter != vfsCache->end(); ++iter)
				{
					if(iter->getId() == element.getParentId())
						break;
				}

				if(iter != vfsCache->end())
				{
					QString path = iter->getPath() + QString(QDir::separator()) + element.getName();
					QFileInfo fInfo(path);
					element.setPath(fInfo.absoluteFilePath());

					WebMounter::getCache()->insert(element);
					return eNO_ERROR;
				}
			}
		}
		return eERROR;
	}

	RESULT GoogleRVFSDriver::getFileEtag(const QString& id, QString& etag)
	{
		QString xmlResp;
		VFSElement elem;
		int retry = 0;

		do
		{
			sleep(2);
			if(_httpConnector.getRemoteFile(id, xmlResp) == eNO_ERROR
				&& _atomParser.parseEntry(xmlResp, elem) == eNO_ERROR)
			{
				if(etag != elem.getModified())
				{
					etag = elem.getModified();
				}
				else
				{
					return eNO_ERROR;
				}
			}
			retry++;
		}
		while(retry < 5);
		return eERROR;
	}

	RESULT GoogleRVFSDriver::uploadFile(const QString& path, const QString& title, const QString& id, const QString& parentid)
	{
		QString xmlResp;
		QFileInfo fInfo(path);
		RESULT err = eERROR;
		QString etag = "";
		VFSElement elem;
		QString suffix = fInfo.suffix();
		suffix = suffix.toLower();

		if(_atomParser.checkExtension(suffix) == eERROR 
			&& suffix != "tmp")
		{
			notifyUser(Ui::Notification::eINFO, tr("Info"), tr("This file extension is not supported !\n"));
			return eERROR;
		}

		VFSCache* vfsCache = WebMounter::getCache();
		VFSCache::iterator iter = vfsCache->begin();
		for(iter; iter != vfsCache->end(); ++iter)
		{
			if(iter->getId() == id)
				break;
		}

		// uploading common file type: document, spreadsheet or presentation
		// Before uploading we create the file on the server in the CreateFile request.
		// So this is the second request and the file has been already inserted into the cache
		if(iter != vfsCache->end() && id != ROOT_ID) 
		{
			if(_httpConnector.checkRemoteFile(iter) == eERROR)
			{
				notifyUser(Ui::Notification::eERROR, tr("Error")
					, tr("The file ") 
					+ iter->getName() 
					+ tr(" is modified by another user on Google server. This file will be changed locally. \n"));

				WebMounter::getProxy()->fileUploaded(fInfo.absoluteFilePath(), eNO_ERROR);
				return eNO_ERROR;
			}

			err = _httpConnector.uploadFile(iter, xmlResp);
			unsigned int retryUploadCounter = 0;

			while(err == eERROR && retryUploadCounter < MAX_UPLOAD_RETRY)
			{
				err = _httpConnector.uploadFile(iter, xmlResp);
				retryUploadCounter++;
			}
		}
		else // Uploading pdf file or smth like that.
			 // We do not create file in the CreateFile request, so this is the first request
		{
			err = _httpConnector.uploadFile(path, title, parentid, xmlResp);
			unsigned int retryUploadCounter = 0;

			while(err == eERROR && retryUploadCounter < MAX_UPLOAD_RETRY)
			{
				err = _httpConnector.uploadFile(path, title, parentid, xmlResp);
				retryUploadCounter++;
			}
		}
		
		if(!err && xmlResp != "")
		{
			if(_atomParser.parseEntry(xmlResp, elem) == eNO_ERROR
				&& getFileEtag(elem.getId(), etag) == eNO_ERROR)
			{
				elem.setModified(etag);
				VFSCache::iterator parent = vfsCache->begin();
				for(parent = vfsCache->begin(); parent != vfsCache->end(); ++parent)
				{
					if(parent->getId() == elem.getParentId())
						break;
				}

				if(parent != vfsCache->end())
				{
					QString path = parent->getPath() + QString(QDir::separator()) + elem.getName();
					QFileInfo fCurInfo(path);
					elem.setPath(fCurInfo.absoluteFilePath());
					elem.setDownloaded(true);

					WebMounter::getProxy()->fileUploaded(fInfo.absoluteFilePath(), err);

					vfsCache->insert(elem, true, true); // modified field has been changed, so we need to update cache and DB. 
					return eNO_ERROR;
				}
			}
			err = eERROR;
		}

		WebMounter::getProxy()->fileUploaded(fInfo.absoluteFilePath(), err);
		return err;
	};

	RESULT GoogleRVFSDriver::modifyFile(const QString&)
	{
		return eNO_ERROR;
	};

	RESULT GoogleRVFSDriver::renameElement( const QString& path, const QString& newTitle)
	{
		QString xmlResp;

		if(!isRunning())
		{
			notifyUser(Ui::Notification::eCRITICAL, tr("Error"), tr("Plugin is not connected !\nPlease connect plugin at first.\n"));
			return eERROR;
		}

		VFSCache* vfsCache = WebMounter::getCache();
		QFileInfo qInfo(path);
		VFSCache::iterator iter = vfsCache->find(qInfo.absoluteFilePath());

		if(iter != vfsCache->end())
		{
			if(_httpConnector.renameElement(iter, newTitle, xmlResp) == eNO_ERROR)
			{
				VFSElement element;
				if(_atomParser.parseEntry(xmlResp, element) == eNO_ERROR)
				{
					VFSCache::iterator parent = vfsCache->begin();
					for(parent = vfsCache->begin(); parent != vfsCache->end(); ++parent)
					{
						if(parent->getId() == element.getParentId())
						break;
					}
					
					if(parent != vfsCache->end())
					{
						QString path = parent->getPath() + QString(QDir::separator()) + element.getName();
						QFileInfo fInfo(path);
						element.setPath(fInfo.absoluteFilePath());
						
						vfsCache->insert(element);
						return eNO_ERROR;
					}
				}
			}
		}
		
		return eERROR;
	};

	RESULT GoogleRVFSDriver::deleteDirectory( const QString& path)
	{
		return deleteElement(path);
	}
	RESULT GoogleRVFSDriver::deleteFile( const QString& path )
	{
		return deleteElement(path);
	}

	RESULT GoogleRVFSDriver::deleteElement( const QString& path)
	{
		QString response;

		if(!isRunning())
		{
			notifyUser(Ui::Notification::eCRITICAL, tr("Error"), tr("Plugin is not connected !\nPlease connect plugin at first.\n"));
			return eERROR;
		}
		
		QFileInfo qInfo(path);
		VFSCache* vfsCache = WebMounter::getCache();
		VFSCache::iterator iter = vfsCache->find(qInfo.absoluteFilePath());

		if(iter != vfsCache->end())
		{
			if(_httpConnector.deleteElement(iter->getEditMetaUrl(), iter->getModified(), response) == eNO_ERROR)
			{
				vfsCache->erase(iter);
				WebMounter::getProxy()->fileDeleted(iter->getPath(), eNO_ERROR);
				return eNO_ERROR;
			}
			WebMounter::getProxy()->fileDeleted(iter->getPath(), eERROR);
		}
		return eERROR;
	};

	RESULT GoogleRVFSDriver::moveElement(const QString& path, const QString& newParentId)
	{
		QString response;

		if(!isRunning())
		{
			notifyUser(Ui::Notification::eCRITICAL, QString(tr("Error")), QString(tr("Plugin is not connected !\nPlease connect plugin at first.\n")));
			return eERROR;
		}

		VFSCache* vfsCache = WebMounter::getCache();
		VFSCache::iterator parent = vfsCache->begin();
		QFileInfo qInfo(path);
		VFSCache::iterator element = vfsCache->find(qInfo.absoluteFilePath());
		for(parent; parent != vfsCache->end(); ++parent)
		{
			if(parent->getId() == newParentId)
			break;
		}
		
		if(parent != vfsCache->end() && element != vfsCache->end())
		{
			if(element->getType() == VFSElement::DIRECTORY)
			{
				notifyUser(Ui::Notification::eCRITICAL, QString(tr("Error")), QString(tr("Moving directory is not allowed !\n")));
				return eERROR;
			}

			if(_httpConnector.moveElement(element, parent->getSrcUrl(), response) == eNO_ERROR)
			{
				VFSElement element;
				if(_atomParser.parseEntry(response, element) == eNO_ERROR)
				{
					QString path = parent->getPath() + QString(QDir::separator()) + element.getName();
					QFileInfo fInfo(path);
					element.setPath(fInfo.absoluteFilePath());
						
					vfsCache->insert(element);
					return eNO_ERROR;
				}
			}
		}
		return eERROR;
	};

	RESULT GoogleRVFSDriver::createDirectory(const QString& path, const QString& parentid, const QString& title)
	{
		VFSElement element;
		QString response;
		QString id;

		if(!isRunning())
		{
			notifyUser(Ui::Notification::eCRITICAL, QString(tr("Error")), QString(tr("Plugin is not connected !\nPlease connect plugin at first.\n")));
			return eERROR;
		}

		//====HACK START !!!
		SettingStorage* settings = WebMounter::getSettingStorage();
		QDir pluginRootDir(settings->getAppStoragePath() + QDir::separator() + _pluginName);
		QDir createDir(path);
		if(pluginRootDir == createDir)
		{
			return eERROR;
		}	
		//====HACK END !!!

		if(_httpConnector.createDirectory(parentid, title, response, _pluginName) == eNO_ERROR)
		{
			if(_atomParser.parseEntry(response, element) == eNO_ERROR)
			{
				VFSCache* vfsCache = WebMounter::getCache();
				VFSCache::iterator iter = vfsCache->begin();
				for(iter; iter != vfsCache->end(); ++iter)
				{
					if(iter->getId() == element.getParentId())
					break;
				}
				
				if(iter != vfsCache->end())
				{
					QString path = iter->getPath() + QString(QDir::separator()) + element.getName();
					QFileInfo fInfo(path);
					element.setPath(fInfo.absoluteFilePath());

					WebMounter::getCache()->insert(element);
					return eNO_ERROR;
				}
			}
		}
				
		return eERROR;
	};

	
	RESULT GoogleRVFSDriver::getElements()
	{
		try
		{
			//QList<VFSElement> elements;
			//return _atomParser.parseDocList(_httpConnector.getDocList(), elements);
			return eNO_ERROR;
		}
		catch(...)
		{
		}
		return eERROR;
	}

	RESULT GoogleRVFSDriver::sync()
	{
		RESULT res = eERROR;
		SettingStorage* settings = WebMounter::getSettingStorage();
		QString pluginStoragePath = settings->getAppStoragePath() + QString(QDir::separator()) + _pluginName;
		QFileInfo fInfo(pluginStoragePath);
		unsigned int uNotDownloaded = 0;
		QString nextLink;
		

		{	LOCK(_driverMutex)
			
			if(_state != eSyncStopping)
			{
				updateState(0, RemoteDriver::eSync);
			}
		}
		
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
			, _pluginName);

		// Hack start
		// Something wrong with Glib::ustring. locale_from_utf8 function sometimes throws an exception
		int counter = 0;
		while(counter < 10 && res == eERROR)
		{
			elements.clear();
			elements.append(elem);
			
			res = _atomParser.parseDocList(_httpConnector.getElements(), elements);
			counter++;
		}
		//Hack end

		if(!res)
		{
			while((nextLink = _atomParser.NextLinkHref()) != "")
			{
				res = _atomParser.parseDocList(_httpConnector.getElements(&nextLink), elements);
				if(res == eERROR)
				{
					stopPlugin();
					notifyUser(Ui::Notification::eCRITICAL, tr("Error"), tr("Sync failed !\n"));
				}
			}
			
			QDir qDir;
			QFile qFile;

			uNotDownloaded = elements.count();
			VFSCache* vfsCache = WebMounter::getCache();
			VFSCache::iterator iter = vfsCache->begin();
			for(iter; iter != vfsCache->end(); ++iter)
			{
				if(iter->getPluginName() == _pluginName)
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
									bool err = qDir.rename(iter->getPath(), elements[i].getPath());
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

			// Add newly created elements
			for(int i=0; i<elements.count(); i++)
			{
				vfsCache->insert(elements[i]);
			}

			QString rootPath = WebMounter::getSettingStorage()->getAppStoragePath() + QString(QDir::separator()) + _pluginName;
			QFileInfo fInfo(rootPath);
			syncCacheWithFileSystem(fInfo.absoluteFilePath());

			PluginSettings plSettings;
			settings->getData(plSettings, _pluginName);

			if(plSettings.bFullSync)
			{
				res = downloadFiles();
			}
		}

		if(res == eERROR)
		{
			stopPlugin();
			notifyUser(Ui::Notification::eCRITICAL, tr("Error"), tr("Sync failed !\n"));
			//updateState(100, RemoteDriver::eNotConnected);
		}
		else
		{
			_driverMutex.lock();
			if(_state != eSyncStopping)
			{
				updateState(100, eConnected);
			}
			_driverMutex.unlock();
		}
		
		return res;
	}
	
	/***********************************************************************
	*********************** Handlers for slots *********************************							  
	***********************************************************************/

	void GoogleRVFSDriver::connectHandler(PluginSettings& pluginSettings)
	{
		QMutexLocker locker(&_driverMutex);

		if(_state == RemoteDriver::eNotConnected)
		{
			GeneralSettings generalSettings; 

			WebMounter::getSettingStorage()->getData(generalSettings);
			WebMounter::getSettingStorage()->addSettings(pluginSettings);

			_httpConnector.setSettings(
				pluginSettings.serverUrl
				, pluginSettings.userName
				, pluginSettings.userPassword
				, generalSettings.proxyAddress
				, generalSettings.proxyLogin + ":" + generalSettings.proxyPassword
				, _pluginName
				);

			if(_state != eSyncStopping)
			{
				updateState(0, eAuthInProgress);
			}

			if(_httpConnector.auth() == eNO_ERROR)
			{
				if(_state != eSyncStopping)
				{
					updateState(100, eAuthorized);
				}

				start(); // run sync thread
			}
			else
			{
				//disconnectHandler();
				updateState(100, eNotConnected);

				notifyUser(Ui::Notification::eCRITICAL, QString(tr("Error")), QString(tr("Authorization failed !\n"
					"Please check proxy settings on Configuration tab and check settings on corresponding plugin tab...\n")));
			}
		}
	}

	void GoogleRVFSDriver::disconnectHandler()
	{
		_driverMutex.lock();

		if(isRunning()) //if sync thread is running 
		{
			updateState(40, eSyncStopping);
			_driverMutex.unlock();

			_forceSync.wakeAll(); // we have to wake up the sync thread for safely termination

			while(isRunning()) // wait thread termination 
			{
				sleep(1);
			}

			//WebMounter::getCache()->restoreCache(); // restore cache from DB
		}
		else
		{
			_driverMutex.unlock();	
		}

		updateState(100, eNotConnected);
	}

	void GoogleRVFSDriver::syncHandler()
	{
		QMutexLocker locker(&_driverMutex);

		if(_state == eConnected)
		{
			_forceSync.wakeAll();
		}
	}

	void GoogleRVFSDriver::stopSyncHandler()
	{
		if(isRunning()) //if sync thread is running 
		{
			PluginSettings pluginSettings;
			WebMounter::getSettingStorage()->getData(pluginSettings, _pluginName);

			updateState(40, eSyncStopping);

			_forceSync.wakeAll();

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
	
	/***********************************************************************
	*********************** Private functions**********************************							  
	***********************************************************************/
	
	void GoogleRVFSDriver::notifyUser(Ui::Notification::_Types type, QString title, QString description) const
	{
		Ui::NotificationDevice* notifDevice = WebMounter::getNotificationDevice();

		Ui::Notification msg(type, title, description);

		notifDevice->showNotification(msg);
	}
	
	void GoogleRVFSDriver::run()
	{
		PluginSettings pluginSettings;

		WebMounter::getSettingStorage()->getData(pluginSettings, _pluginName);

		int sync_period = pluginSettings.syncPeriod.toInt();

		forever
		{
			if(_state == eSyncStopping || _state == eNotConnected)
			{
				return;
			}

			if(_state == eAuthorized)
			{
				sync();
				if(_state == eSyncStopping)
				{
					return;
				}
			}

			_syncMutex.lock();

			bool result = _forceSync.wait(&_syncMutex, sync_period * 1000);

			_syncMutex.unlock();

			if(_state == eConnected)
			{
				sync();
			}

			if(_state == eSyncStopping)
			{
				return;
			}
		}
	}
	
	//RESULT GoogleRVFSDriver::downloadFiles()
	//{
	//	QList <QString> urlToDownload;
	//	QList <QString> pathToDownload;
	//	VFSCache* vfsCache = WebMounter::getCache();
	//	VFSCache::iterator iter = vfsCache->begin();
	//	UINT uDownloaded = 0;
	//	RESULT err = eERROR;
	//	for(iter = vfsCache->begin(); iter != vfsCache->end(); ++iter)
	//	{
	//		{ 	LOCK(_driverMutex)
	//			
	//			if(_state == eSyncStopping)
	//			{
	//				notifyUser(Ui::Notification::eINFO, tr("Info"), tr("Synchronization is stopped !\n"));
	//				//updateState(100, RemoteDriver::eConnected);
	//				return eNO_ERROR;
	//			}
	//		}

	//		if(iter->getPluginName() == _pluginName)
	//		{
	//			if(iter->getType() == VFSElement::FILE 
	//				&& !(iter->getFlags() & VFSElement::eFl_Downloading)
	//				&& !(iter->getFlags() & VFSElement::eFl_Downloaded))
	//			{
	//				urlToDownload.append(iter->getSrcUrl());
	//				pathToDownload.append(iter->getPath());
	//				
	//				if(urlToDownload.size() == DOWNLOAD_CHUNK_SIZE)
	//				{
	//					err = downloadFiles(urlToDownload, pathToDownload);
	//					
	//					if(err)
	//					{
	//						updateDownloadStatus(err, uDownloaded, countNotDownloaded());
	//						return err;
	//					}
	//					else
	//					{
	//						uDownloaded += urlToDownload.size();
	//						updateDownloadStatus(err, uDownloaded, countNotDownloaded());
	//						urlToDownload.clear();
	//						pathToDownload.clear();
	//					}
	//				}
	//			}
	//		}
	//	}

	//	if(urlToDownload.size() > 0)
	//	{
	//		return downloadFiles(urlToDownload, pathToDownload);
	//	}

	//	return err;
	//}
	
	RESULT GoogleRVFSDriver::downloadFiles(QList <QString>& urlList, QList <QString>& pathList)
	{
		VFSCache* vfsCache = WebMounter::getCache();
		int initialListSize = urlList.size();
		
		{ 	LOCK(_driverMutex)
		
			VFSCache::iterator iter = vfsCache->end();
			for(int i = 0; i < pathList.size(); ++i)
			{
				iter = vfsCache->find(pathList.at(i));
				if(iter != vfsCache->end()
					&& !(iter->getFlags() & VFSElement::eFl_Downloading)
					&& !(iter->getFlags() & VFSElement::eFl_Downloaded))
				{	
					vfsCache->setFlag(iter, VFSElement::eFl_Downloading);
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
					return eERROR;
				}
			}
		}
		
		RESULT res = _httpConnector.downloadFiles(urlList, pathList);
		if(res == eERROR)
		{
			unsigned int retryDownloadCounter = 0;
			while(res == eERROR && retryDownloadCounter < MAX_DOWNLOAD_RETRY)
			{
				res = _httpConnector.downloadFiles(urlList, pathList);
				retryDownloadCounter++;
			}
		}
		
		{ 	LOCK(_driverMutex)
		
			for(int i = 0; i < pathList.size(); i++)
			{
				QFileInfo fInfo(pathList.at(i));
				VFSCache::iterator iter = vfsCache->find(fInfo.absoluteFilePath());

				if(iter != vfsCache->end())
				{
					if(res == eERROR)
					{
						vfsCache->setFlag(iter, VFSElement::eFl_None, VFSElement::eFl_Downloading);
					}
					else
					{
						vfsCache->setFlag(iter, VFSElement::eFl_Downloaded, VFSElement::eFl_Downloading);
					}
				}
			}
		}
		return res;
	}
	
	unsigned int GoogleRVFSDriver::countNotDownloaded()
	{
		VFSCache* cache = WebMounter::getCache();
		unsigned int counter = 0;

		for(VFSCache::iterator iter = cache->begin(); 
			iter != cache->end(); 
			++iter)
		{
			if( iter->getPluginName() == _pluginName && !iter->isDownloaded() && iter->getType() == VFSElement::FILE)
			{
				counter++;
			}
		}

		return counter;
	}

};