#ifndef GOOGLE_CONNECTOR_H
#define GOOGLE_CONNECTOR_H

#include <QMutex>
#include <QThread>
#include <vector>

#include "../xml/google_xml.h"
#include <curl/curl.h>

#include "common_stuff.h"
#include "vfs_cache.h"

namespace Connector
{
	using namespace Data;
	using std::vector;

	typedef size_t (*WriteFuncPtr)(void *, size_t, size_t, void *);
	
	struct PostData 
	{
		char *data;
		QString filename;
	};

	const QString kVersion = "1.0";
	const QString kClientLoginUrl =
		"https://www.google.com/accounts/ClientLogin";
	const QString kClientLoginAuthHeaderPrefix =
		"Authorization: GoogleLogin auth=";

	class GoogleHTTPConnector//: public QThread
	{
	public:
		GoogleHTTPConnector(){};
		~GoogleHTTPConnector() {};

		void setSettings(const QString& url
			, const QString& login
			, const QString& password
			, const QString& proxy = "" //"proxy.te.mera.ru:8080"
			, const QString& proxyLoginPwd = ""//"loing:password"
			, const QString& pluginName = "");

	public:
		static size_t read_b(void *ptr, size_t size, size_t nmemb, void *stream);
		static size_t read_str(void *ptr, size_t size, size_t count, void *buffer);
		static size_t write_str(void *ptr, size_t size, size_t count, void *response);
		static size_t fwrite_b(void *ptr, size_t size, size_t count, void *path);
		
		RESULT auth();
		RESULT checkRemoteFile(VFSCache::iterator iter);
		RESULT getRemoteFile(const QString& id, QString& xmlResp);
		RESULT createFile(const QString& path
							, const QString& title
							, const QString& parentId
							, QString& response);
		QString getElements(const QString* nextLink = 0) const;
		RESULT downloadFiles(QList <QString>& urlList, QList <QString>& pathList);
		RESULT downloadFile(QString& url, QString& path);
		RESULT deleteElement(const QString& url, const QString& etag, QString& response);
		RESULT getTreeElements(const QString& path, const QString& pluginName);
		RESULT uploadFile(const QString& path, const QString& title, const QString& parentId, QString& response);
		RESULT uploadFile(VFSCache::iterator iter, QString& response);
		RESULT deleteFile(const QString& id, QString& response, const QString& pluginName);
		RESULT deleteDirectory(const QString& id, QString& response, const QString& pluginName);
		RESULT createDirectory(const QString& parentid, const QString& title, QString& response, const QString& pluginName);
		RESULT moveElement(VFSCache::iterator iter
							, const QString& newParentSrc
							, QString& response);

		RESULT renameElement(VFSCache::iterator iter
								, const QString& newTitle
								, QString& response);
	private:
		QString getDocType(const QString& file_extension);
		void setUploadHeader(const QString& file_extension
								, const QString& title
								, const QString& etag
								, curl_slist*& headers);

		void initCurl(CURL* p_curl
						, curl_slist*& headers
						, const QString& url
						, WriteFuncPtr func
						, int protocol = 2
						, const QString& http_method = "GET"
						, const QString& service = "doc") const;
		
	private:
		struct sPutData
		{
			const char *data;
			size_t len;
		};

	private:
		QString auth_token_doc;
		QString auth_token_spr;
		QString service_name_;
		QString application_name_;
		vector<QString> request_headers_doc;
		vector<QString> request_headers_spr;
		
		QString _url;
		QString _login;
		QString _password;
		QString _proxy;
		QString _proxy_login_pwd;
		QMutex _connectorMutex;
		struct curl_slist *_chunk;

		static const QString kDocServiceName;
		static const QString kSpreadsheetServiceName;
		static const QString kDocListScope;
		static const QString kDocListFeed;
		static const QString kDocListFeed_V3;
		static const QString kDocListFolderFeed;
		static const QString kDocListAclFeed;
		static const QString kDocumentCategory;
		static const QString kSpreadsheetCategory;
		static const QString kPresentationCategory;
		static const QString kFolderCategory;
		static const QString kStarredCategory;
		static const QString kTrashedCategory;
		static const QString kDownloadDocFeed;
	};
}
#endif // GOOGLE_CONNECTOR_H