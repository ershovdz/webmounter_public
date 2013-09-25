#ifndef JMM_DRIVER_H
#define JMM_DRIVER_H

#include "common_stuff.h"

#include "data.h"
#include "rvfs_driver.h"
#include "../connector/jmm_connector.h"
#include "notification_device.h"

#include <QDir>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <queue>

#if defined(WEBMOUNTER_JMM_LIBRARY)
#  define WEBMOUNTER_JMM_EXPORT Q_DECL_EXPORT
#else
#  define WEBMOUNTER_JMM_EXPORT Q_DECL_IMPORT
#endif

namespace RemoteDriver
{
	
	class WEBMOUNTER_JMM_EXPORT JmmRVFSDriver :
		public RVFSDriver
	{
		Q_OBJECT

	public:
		JmmRVFSDriver(const QString& pluginName);
		JmmRVFSDriver(const QString& pluginName
			, const QString& rootPath
			, const QString& url
			, const QString& login
			, const QString& password
			, const QString& proxy = ""//"proxy.te.mera.ru:8080"
			, const QString& proxyLoginPwd = "");

	public:
		~JmmRVFSDriver(void);

	public:
		virtual RESULT downloadFiles(QList <QString>& urlList, QList <QString>& pathList);
		virtual RESULT uploadFile(const QString& path, const QString& title, const QString& id, const QString& parentid);
		virtual RESULT modifyFile(const QString&);
		virtual RESULT renameElement( const QString& path, const QString& newTitle);
		virtual RESULT deleteDirectory( const QString& path);
		virtual RESULT deleteFile( const QString& id );
		virtual RESULT moveElement( const QString& path, const QString& newParentId);
		virtual RESULT createDirectory(const QString& path,  const QString& parentid, const QString& title);
		virtual RESULT createFile(const QString& path, const QString& title,  const QString& id, const QString& parentId);
		virtual RESULT getElements();
		virtual RESULT sync();
		
	protected:
		RESULT deleteNotDirty();
                unsigned int countNotDownloaded();
		void notifyUser(Ui::Notification::_Types type, QString title, QString description) const;

		virtual void connectHandler(PluginSettings& pluginSettings);
		virtual void disconnectHandler();
		virtual void syncHandler();
		virtual void stopSyncHandler();

//		int removeFolder(QDir& dir);
		//void syncCacheWithFileSystem(const QString& path);
		virtual RESULT checkKey(const PluginSettings& pluginSettings);
		void run();
	
	private:
		Connector::JmmHTTPConnector _httpConnector;
		
		QWaitCondition _forceSync;
		QMutex _syncMutex;
		QMutex _driverMutex;
		
		//JmmRVFSDriver* _driverInstance;
		//const QString _pluginName;

		//Ui::View* _driverView;

		//const unsigned int queueMaxLength;
		//queue<RemoteDriverCommand> cmdQueue;
		//QMutex mutex;
		//QWaitCondition queueIsNotEmpty;
	};
}

#endif
