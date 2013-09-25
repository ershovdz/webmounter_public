#ifndef JMM_CONNECTOR_H
#define JMM_CONNECTOR_H

#include <QMutex>
#include <QThread>

#include <curl/curl.h>

#include "common_stuff.h"
#include "vfs_cache.h"

#if defined(WEBMOUNTER_JMM_LIBRARY)
#  define WEBMOUNTER_JMM_EXPORT Q_DECL_EXPORT
#else
#  define WEBMOUNTER_JMM_EXPORT Q_DECL_IMPORT
#endif

namespace Connector
{
	using namespace Data;
	using namespace Common;

	class WEBMOUNTER_JMM_EXPORT JmmHTTPConnector//: public QThread
	{
	public:
		JmmHTTPConnector(){ _curl = 0; };

		void setSettings(const QString& url
			, const QString& login
			, const QString& password
			, const QString& proxy = "" //"proxy.te.mera.ru:8080"
			, const QString& proxyLoginPwd = ""//"loing:password"
			, const QString& pluginName = "");
	
	public:
		~JmmHTTPConnector();
		
		RESULT auth();
		static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp);
		static size_t write_str(void *ptr, size_t size, size_t count, void *response);
		static size_t fwrite_ch(void *ptr, size_t size, size_t count, void *path);
		static size_t fwrite_b(void *ptr, size_t size, size_t count, void *path);
		RESULT getTreeElements(const QString& path, const QString& pluginName);
		RESULT downloadFile(const QString& url, const QString& path);
		RESULT uploadFile(const QString& path, const QString& title, const QString& id, const QString& parentId, QString& response, const QString& pluginName);
		RESULT deleteFile(const QString& id, QString& response, const QString& pluginName);
		RESULT deleteDirectory(const QString& id, QString& response, const QString& pluginName);
		RESULT createDirectory(const QString& parentid, const QString& title, QString& response, const QString& pluginName);
		RESULT moveElement(const QString& id, const QString& oldParentId, const QString& newParentId, ElementType type, QString& response, const QString& pluginName);
		RESULT renameElement(const QString& id, ElementType type, const QString& newTitle, QString& response, const QString& pluginName);

	private:
		CURL* _curl;
		QString _url;
		QString _login;
		QString _password;
		QString _proxy;
		QString _proxy_login_pwd;
		QMutex _connectorMutex;
		struct curl_slist *_chunk;
	};
}
#endif
