#ifndef GOOGLE_DRIVER_H
#define GOOGLE_DRIVER_H

//#include "common_stuff.h"

#include "data.h"
#include "rvfs_driver.h"
#include "../connector/google_connector.h"
#include "../xml/google_xml.h"

#include "notification_device.h"

#include <QDir>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <queue>
#include "view.h"


namespace RemoteDriver
{
	
	class GoogleRVFSDriver : public RVFSDriver
	{
		Q_OBJECT

	public:
		GoogleRVFSDriver(const QString& pluginName);
		GoogleRVFSDriver(const QString& pluginName
			, const QString& rootPath
			, const QString& url
			, const QString& login
			, const QString& password
			, const QString& proxy = ""//"proxy.te.mera.ru:8080"
			, const QString& proxyLoginPwd = "");

	public:
		~GoogleRVFSDriver(void);

	public:
		virtual RESULT downloadFiles() {return RVFSDriver::downloadFiles();};
		virtual RESULT downloadFiles(QList <QString>& urlList, QList <QString>& pathList);
		virtual RESULT uploadFile(const QString& path, const QString& title, const QString& id, const QString& parentid);
		virtual RESULT modifyFile(const QString&);
		virtual RESULT renameElement( const QString& path, const QString& newTitle);
		virtual RESULT deleteDirectory( const QString& path);
		virtual RESULT deleteFile( const QString& path );
		virtual RESULT moveElement( const QString& path, const QString& newParentId);
		virtual RESULT createDirectory(const QString& path,  const QString& parentid, const QString& title);
		virtual RESULT createFile(const QString& path, const QString& title,  const QString& id, const QString& parentId);
		virtual RESULT getElements();
		virtual RESULT sync();
		
	//protected:
		//
		RESULT deleteNotDirty();
		unsigned int countNotDownloaded();
		void notifyUser(Ui::Notification::_Types type, QString title, QString description) const;

		virtual void connectHandler(PluginSettings& pluginSettings);
		virtual void disconnectHandler();
		virtual void syncHandler();
		virtual void stopSyncHandler();

		//int removeFolder(QDir& dir);
		//void syncCacheWithFileSystem(const QString& path);
		//virtual RESULT checkKey(const PluginSettings& pluginSettings);
		void run();
	
	private:
		RESULT getFileEtag(const QString& id, QString& etag);
		RESULT deleteElement( const QString& path);
		Connector::GoogleHTTPConnector _httpConnector;
		Xml::GoogleAtomParser _atomParser;

		QWaitCondition _forceSync;
		QMutex _syncMutex;
		Ui::View* _driverView;
	};
}

#endif
