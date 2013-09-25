//#include "common_stuff.h"

#include "google_connector.h"
#include "../xml/google_xml.h"
#include "data.h"
#include "webmounter.h"

#include <QFile>

#include <fstream>
#include <iostream>
#include <string.h>

namespace Connector
{
	class FileHandle
	{
	public:
		FileHandle(FILE* ptr)
		{
			pFile = ptr;
		}
		~FileHandle()
		{
			if(pFile)
				fclose(pFile);
		}
		FILE* getPtr()
		{
			return pFile;
		}
		operator bool()
		{
			return (bool)pFile;
		}
		bool operator!()
		{
			return !(bool)pFile;
		}
	private:
		FILE* pFile;
	};
	
	
	using std::ifstream;
	using std::ios;
	using Common::eERROR;
	using Common::eNO_ERROR;

	const QString GoogleHTTPConnector::kDocServiceName = "writely";
	const QString GoogleHTTPConnector::kSpreadsheetServiceName = "wise";
	const QString GoogleHTTPConnector::kDocListScope = "http://docs.google.com/feeds";
	//const QString GoogleHTTPConnector::kDocListFeed = "/documents/private/full";
	const QString GoogleHTTPConnector::kDocListFolderFeed = "/folders/private/full/";
	//const QString GoogleHTTPConnector::kDocListAclFeed = "/acl/private/full/";
	const QString GoogleHTTPConnector::kDocumentCategory = "/-/document";
	const QString GoogleHTTPConnector::kSpreadsheetCategory = "/-/spreadsheet";
	const QString GoogleHTTPConnector::kPresentationCategory = "/-/presentation";
	const QString GoogleHTTPConnector::kFolderCategory = "/-/folder";
	//const QString GoogleHTTPConnector::kStarredCategory = "/-/starred";
	//const QString GoogleHTTPConnector::kTrashedCategory = "/-/trashed";
	const QString GoogleHTTPConnector::kDocListFeed_V3 = "/default/private/full";

	void GoogleHTTPConnector::setSettings(const QString& url
		, const QString& login
		, const QString& password
		, const QString& proxy
		, const QString& proxyLoginPwd
		, const QString& pluginName) 
	{
		_login = login;
		_password = password;
		_proxy = proxy;
		_proxy_login_pwd = proxyLoginPwd;

		request_headers_doc.clear();
		request_headers_spr.clear();

		// Standard headers for every request
		request_headers_doc.push_back("User-Agent: WebMounter GData-C++/" + kVersion);
		request_headers_spr.push_back("User-Agent: WebMounter GData-C++/" + kVersion);
	}

	RESULT GoogleHTTPConnector::auth()
	{
		QString responseDoc;
		QString responseSpr;
		char* dataDoc;
		char* dataSpr;
		
		try
		{
			CURL *p_curlDoc = curl_easy_init();
			CURL *p_curlSpr = curl_easy_init();
			struct curl_slist *headersDoc = NULL;
			struct curl_slist *headersSpr = NULL;
			
			if(p_curlDoc && p_curlSpr)
			{
				QString bodyDoc = "Email=" + _login + "&Passwd=" + _password +
					"&accountType=" + "HOSTED_OR_GOOGLE" +
					"&source=WebMounter&service=" + kDocServiceName;
				
				QString bodySpr = "Email=" + _login + "&Passwd=" + _password +
					"&accountType=" + "HOSTED_OR_GOOGLE" +
					"&source=WebMounter&service=" + kSpreadsheetServiceName;

				dataDoc = new char[strlen(bodyDoc.toLocal8Bit().constData()) + 1];
				strcpy(dataDoc, qPrintable(bodyDoc));

				dataSpr = new char[strlen(bodySpr.toLocal8Bit().constData()) + 1];
				strcpy(dataSpr, qPrintable(bodySpr));

				initCurl(p_curlDoc, headersDoc, kClientLoginUrl, write_str, 2, "POST");
				initCurl(p_curlSpr, headersSpr, kClientLoginUrl, write_str, 2, "POST");

				curl_easy_setopt(p_curlDoc, CURLOPT_WRITEDATA, &responseDoc);
				curl_easy_setopt(p_curlDoc, CURLOPT_POSTFIELDS, dataDoc);
				curl_easy_setopt(p_curlDoc, CURLOPT_POSTFIELDSIZE, strlen(dataDoc));

				curl_easy_setopt(p_curlSpr, CURLOPT_WRITEDATA, &responseSpr);
				curl_easy_setopt(p_curlSpr, CURLOPT_POSTFIELDS, dataSpr);
				curl_easy_setopt(p_curlSpr, CURLOPT_POSTFIELDSIZE, strlen(dataSpr));

				CURLcode errorDoc = curl_easy_perform(p_curlDoc);
				long codeDoc;
				curl_easy_getinfo(p_curlDoc, CURLINFO_RESPONSE_CODE, &codeDoc);

				CURLcode errorSpr = curl_easy_perform(p_curlSpr);
				long codeSpr;
				curl_easy_getinfo(p_curlSpr, CURLINFO_RESPONSE_CODE, &codeSpr);

				/* free slist */ 
				curl_slist_free_all(headersDoc);
				curl_slist_free_all(headersSpr);
				curl_easy_cleanup(p_curlDoc);
				curl_easy_cleanup(p_curlSpr);

				if(errorDoc == CURLE_OK && codeDoc == 200 
					&& errorSpr == CURLE_OK && codeSpr == 200)
				{
					QString prefix = "Auth=";  // prefix of the ClientLogin token
					QString tokenDoc = responseDoc.mid(responseDoc.indexOf(QRegExp(prefix)) + prefix.size());
					QString tokenSpr = responseSpr.mid(responseSpr.indexOf(QRegExp(prefix)) + prefix.size());
					auth_token_doc = tokenDoc.mid(0, tokenDoc.size() - 1);  // remove trailing "\n"
					auth_token_spr = tokenSpr.mid(0, tokenSpr.size() - 1);  // remove trailing "\n"

					// Attach Authorization header to every subsequent request
					request_headers_doc.push_back(kClientLoginAuthHeaderPrefix + auth_token_doc);
					request_headers_spr.push_back(kClientLoginAuthHeaderPrefix + auth_token_spr);
					return eNO_ERROR;
				}
				else
				{
					return eERROR;
				}
			}
		}
		catch(...)
		{
		}

		if(dataDoc)
		{
			delete[] dataDoc;
		}

		if(dataSpr)
		{
			delete[] dataSpr;
		}
		return eERROR;
	}

	QString GoogleHTTPConnector::getElements(const QString* nextLink) const 
	{
		QString doc_list_feed;

		if(nextLink == 0) // request for first 100 elements
		{
			doc_list_feed = kDocListScope + kDocListFeed_V3 + "?showfolders=true";
		}
		else // request for next elements
		{
			doc_list_feed = *nextLink + "&showfolders=true";
		}

		QString response;		
		struct curl_slist *headers = NULL;
		CURL* p_curl = curl_easy_init();
		int http_code = 0;

		if(p_curl)
		{
			curl_easy_setopt(p_curl, CURLOPT_HTTPGET, 1L);
			headers = curl_slist_append(headers, "Content-Type: application/atom+xml");
			initCurl(p_curl, headers, doc_list_feed, write_str, 3, "GET");
			curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &response);

			CURLcode _error = curl_easy_perform(p_curl);
			curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &http_code);

			if (_error != CURLE_OK || 400 <= http_code) 
			{
				response = "";
			}

			// clean up
			curl_easy_cleanup(p_curl);
			curl_slist_free_all(headers);			
		}

		return response;
	}

	RESULT GoogleHTTPConnector::checkRemoteFile(VFSCache::iterator iter)
	{
		{   LOCK(_connectorMutex)

			RESULT res = eNO_ERROR;
			struct curl_slist *headers = NULL;
			CURL* p_curl = curl_easy_init();
			long http_code;
			QString url = kDocListScope + kDocListFeed_V3 + "/" + iter->getId();
			QString response;

			if(p_curl)
			{
				headers = curl_slist_append(headers, qPrintable(QString("If-None-Match: " + iter->getModified())));
				initCurl(p_curl, headers, url, write_str, 3, "GET");
				curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &response);

				CURLcode _error = curl_easy_perform(p_curl);

				curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &http_code);
				res = (_error == CURLE_OK 
					&& (http_code == 304 || http_code == 412)) ? eNO_ERROR : eERROR;

				// clean up
				curl_easy_cleanup(p_curl);
				curl_slist_free_all(headers);
			}
			else
			{
				res = eERROR;
			}
			return res;
		}
	}

	RESULT GoogleHTTPConnector::getRemoteFile(const QString& id, QString& xmlResp)
	{
		{   LOCK(_connectorMutex)

			xmlResp = "";
			RESULT res = eNO_ERROR;
			struct curl_slist *headers = NULL;
			CURL* p_curl = curl_easy_init();
			long http_code;
			QString url = kDocListScope + kDocListFeed_V3 + "/" + id;
			
			if(p_curl)
			{
				initCurl(p_curl, headers, url, write_str, 3, "GET");
				curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &xmlResp);

				CURLcode _error = curl_easy_perform(p_curl);

				curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &http_code);
				res = (_error == CURLE_OK && http_code == 200 ) ? eNO_ERROR : eERROR;

				// clean up
				curl_easy_cleanup(p_curl);
				curl_slist_free_all(headers);
			}
			else
			{
				res = eERROR;
			}
			return res;
		}
	}

	void GoogleHTTPConnector::initCurl(CURL* p_curl
										, curl_slist*& headers
										, const QString& url
										, WriteFuncPtr func
										, int protocol
										, const QString& http_method
										, const QString& service) const
	{
		if((_proxy != "") && (_proxy != ":"))
		{
			curl_easy_setopt(p_curl, CURLOPT_PROXY, qPrintable(_proxy));
			curl_easy_setopt(p_curl, CURLOPT_PROXYUSERPWD, qPrintable(_proxy_login_pwd));
		}

		if(http_method == "POST")
		{
			curl_easy_setopt(p_curl, CURLOPT_POST, 1L);
		}
		else if(http_method == "GET")
		{
			curl_easy_setopt(p_curl, CURLOPT_HTTPGET, 1L);
		}
		else if (http_method == "DELETE") 
		{
			curl_easy_setopt(p_curl, CURLOPT_CUSTOMREQUEST, "DELETE");
		}
		else if (http_method == "PUT") 
		{
			curl_easy_setopt(p_curl, CURLOPT_PUT, 1L);
		}

		curl_easy_setopt(p_curl, CURLOPT_URL, qPrintable(url));
		curl_easy_setopt(p_curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, func);
		//curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
		curl_easy_setopt(p_curl, CURLOPT_HEADER, 0L);
		curl_easy_setopt(p_curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
		curl_easy_setopt(p_curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(p_curl, CURLOPT_CONNECTTIMEOUT, 30L);
		

		// Add standard headers
		if(service == "doc")
		{
			for (unsigned int i = 0; i < request_headers_doc.size(); ++i) 
			{
				headers = curl_slist_append(headers, qPrintable(request_headers_doc[i]));
			}
		}
		else if(service == "spr")
		{
			for (unsigned int i = 0; i < request_headers_spr.size(); ++i) 
			{
				headers = curl_slist_append(headers, qPrintable(request_headers_spr[i]));
			}
		}

		if(protocol == 2)
		{
			headers = curl_slist_append(headers, "GData-Version: 2.0");
		}
		else if(protocol == 3)
		{
			headers = curl_slist_append(headers, "GData-Version: 3.0");
		}

		headers = curl_slist_append(headers, "Cache-Control: no-store, no-cache, must-revalidate");

		// attach headers to this request
		curl_easy_setopt(p_curl, CURLOPT_HTTPHEADER, headers);
	}

	RESULT GoogleHTTPConnector::downloadFile(QString& url, QString& path)
	{
		QMutexLocker locker(&_connectorMutex);

		RESULT res = eNO_ERROR;
		struct curl_slist *headers = NULL;
		CURL* p_curl = curl_easy_init();
		long http_code;
		if(p_curl)
		{
			initCurl(p_curl, headers, url, fwrite_b, 3, "GET");
			curl_easy_setopt(p_curl, CURLOPT_HTTPGET, 1L);
			curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &path);

			CURLcode _error = curl_easy_perform(p_curl);

			curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &http_code);
			res = (_error == CURLE_OK && http_code == 200) ? eNO_ERROR : eERROR;

			// clean up
			curl_easy_cleanup(p_curl);
			curl_slist_free_all(headers);
		}
		else
		{
			res = eERROR;
		}

		return res;
	}

	RESULT GoogleHTTPConnector::downloadFiles(QList <QString>& urlList, QList <QString>& pathList)
	{
		QMutexLocker locker(&_connectorMutex);

		RESULT res = eNO_ERROR;
		QList <CURL*> curls_list;
		CURLM* p_mcurl = curl_multi_init();
		QList <curl_slist*> headers_list;
		long http_code;
		
		for(int i = 0; i < urlList.size(); i++)
		{
			CURL* p_curl = curl_easy_init();
			if(p_curl)
			{
				struct curl_slist *headers = NULL;
				
				int offset = pathList.at(i).lastIndexOf(".");
				if(offset != -1)
				{
					if(getDocType(pathList.at(i).mid(offset + 1).toLower()) == "spreadsheet")
					{
						initCurl(p_curl, headers, urlList.at(i), fwrite_b, 3, "GET", "spr");
					}
					else
					{
						initCurl(p_curl, headers, urlList.at(i), fwrite_b, 3, "GET", "doc");
					}
				}
				else
				{
					return eERROR;
				}
				
				//initCurl(p_curl, headers, urlList.at(i), fwrite_b, 3, "GET");
				//if((_proxy != "") && (_proxy != ":"))
				//{
				//	curl_easy_setopt(p_curl, CURLOPT_PROXY, qPrintable(_proxy));
				//	curl_easy_setopt(p_curl, CURLOPT_PROXYUSERPWD, qPrintable(_proxy_login_pwd));
				//}

				//curl_easy_setopt(p_curl, CURLOPT_HTTPGET, 1L);
				//curl_easy_setopt(p_curl, CURLOPT_URL, qPrintable(urlList.at(i)));
				//curl_easy_setopt(p_curl, CURLOPT_VERBOSE, 1L);
				//curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, fwrite_b);
				////curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
				//curl_easy_setopt(p_curl, CURLOPT_HEADER, 0L);
				//curl_easy_setopt(p_curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
				//curl_easy_setopt(p_curl, CURLOPT_FOLLOWLOCATION, 1L);
				//curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYPEER, 0L);
				//curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYHOST, 0L);

				//// Add standard headers
				//for (unsigned int j = 0; j < request_headers_.size(); ++j) 
				//{
				//	headers = curl_slist_append(headers, qPrintable(request_headers_[j]));
				//}

				//headers = curl_slist_append(headers, "GData-Version: 3.0");
				//headers = curl_slist_append(headers, "Cache-Control: no-store, no-cache, must-revalidate");

				//// attach headers to this request
				//curl_easy_setopt(p_curl, CURLOPT_HTTPHEADER, headers);

				curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &pathList.at(i));
				curls_list.append(p_curl);
				headers_list.append(headers);
				curl_multi_add_handle(p_mcurl, p_curl);
			}
			else
			{
				res = eERROR;
				break;
			}
		}

		if(res != eERROR)
		{
			int still_running = 0;
			int result = CURLM_CALL_MULTI_PERFORM;
			do
			{
				result = (int)curl_multi_perform(p_mcurl, &still_running);

				if(still_running <= 0)
				{
					break;
				}
			}
			while(result == CURLM_CALL_MULTI_PERFORM || still_running > 0);

			int msgs_in_queue = 0;
			do
			{
				CURLMsg* msg = curl_multi_info_read(p_mcurl, &msgs_in_queue);
				if(msg)
				{
					curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &http_code);
					if((RESULT)msg->data.result == eERROR
						|| http_code != 200)
					{
						CURLcode _error = curl_easy_perform(msg->easy_handle);
						curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &http_code);
						if(_error != CURLE_OK || http_code != 200)
						{
							res = eERROR;
							break;
						}
					}
				}
			}
			while(msgs_in_queue);
		}

		// clean up
		for(int i = 0; i < curls_list.size(); i++)
		{
			curl_multi_remove_handle(p_mcurl, curls_list.at(i)); 
			curl_easy_cleanup(curls_list.at(i));
			curl_slist_free_all(headers_list.at(i));
		}

		curl_multi_cleanup(p_mcurl);
		return res;
	}

	RESULT GoogleHTTPConnector::createDirectory(const QString& parentid
		, const QString& title
		, QString& response
		, const QString& pluginName)
	{
		{	LOCK(_connectorMutex)

			char *data = 0;
			RESULT res = eNO_ERROR;
			try
			{
				
				struct curl_slist *headers = NULL;
				CURL* p_curl = curl_easy_init();
				long http_code;
				QString url = kDocListScope + kDocListFeed_V3;

				if(parentid != ROOT_ID)
				{
					url += QString("/%1/contents").arg(parentid);
				}

				if(p_curl)
				{
					headers = curl_slist_append(headers, "Content-Type: application/atom+xml");

					initCurl(p_curl, headers, url, write_str, 3, "POST");
					curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &response);

					QString body = 	"<?xml version=\'1.0\' encoding=\'UTF-8\'?>" 
						"<entry xmlns=\"http://www.w3.org/2005/Atom\">"
						"<category scheme=\"http://schemas.google.com/g/2005#kind\" "
						"term=\"http://schemas.google.com/docs/2007#folder\"/>"
						"<title>";
					body += title + "</title></entry>";

					data = new char[strlen(body.toUtf8().constData()) + 1];
					strcpy(data, body.toUtf8().constData());

					curl_easy_setopt(p_curl, CURLOPT_POSTFIELDS, data);
					curl_easy_setopt(p_curl, CURLOPT_POSTFIELDSIZE, strlen(data));

					CURLcode _error = curl_easy_perform(p_curl);

					curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &http_code);
					res = (_error == CURLE_OK && http_code == 201) ? eNO_ERROR : eERROR;

					// clean up
					curl_easy_cleanup(p_curl);
					curl_slist_free_all(headers);
				}
				else
				{
					res = eERROR;
				}
			}
			catch(...)
			{
			}

			if(data)
			{
				delete[] data;
			}
			return res;
		}
	}

	RESULT GoogleHTTPConnector::deleteElement(const QString& url, const QString& etag, QString& response)
	{
		QMutexLocker locker(&_connectorMutex);

		RESULT res = eNO_ERROR;
		struct curl_slist *headers = NULL;
		CURL* p_curl = curl_easy_init();
		long http_code;

		if(p_curl)
		{
			headers = curl_slist_append(headers, qPrintable(QString("If-Match: " + etag)));
			initCurl(p_curl, headers, url, write_str, 3, "DELETE");
			curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &response);

			CURLcode _error = curl_easy_perform(p_curl);

			curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &http_code);
			res = (_error == CURLE_OK && http_code == 200) ? eNO_ERROR : eERROR;

			// clean up
			curl_easy_cleanup(p_curl);
			curl_slist_free_all(headers);
		}
		else
		{
			res = eERROR;
		}

		return res;
	}

	void GoogleHTTPConnector::setUploadHeader(const QString& file_extension
												, const QString& title
												, const QString& etag
												, curl_slist*& headers)
	{
		if (file_extension == "doc") 
		{
			headers = curl_slist_append(headers, "Content-Type: application/msword");
		} 
		else if (file_extension == "docx") 
		{
			headers = curl_slist_append(headers, "Content-Type: application/vnd.openxmlformats-officedocument.wordprocessingml.document");
		} 
		else if (file_extension == "pdf") 
		{
			headers = curl_slist_append(headers, "Content-Type: application/pdf");
		} 
		else if (file_extension == "htm" || file_extension == "html") 
		{
			headers = curl_slist_append(headers, "Content-Type: text/html");
		} 
		else if (file_extension == "ods") 
		{
			headers = curl_slist_append(headers, "Content-Type: application/x-vnd.oasis.opendocument.spreadsheet");
		} 
		else if (file_extension == "odt") 
		{
			headers = curl_slist_append(headers, "Content-Type: application/vnd.oasis.opendocument.text");
		} 
		else if (file_extension == "pps" || file_extension == "ppt") 
		{
			headers = curl_slist_append(headers, "Content-Type: application/vnd.ms-powerpoint");
		} 
		else if (file_extension == "rtf") 
		{
			headers = curl_slist_append(headers, "Content-Type: application/rtf");
		}
		else if (file_extension == "xls") 
		{
			headers = curl_slist_append(headers, "Content-Type: application/vnd.ms-excel");
		}
		else if (file_extension == "xlsx") 
		{
			headers = curl_slist_append(headers, "Content-Type: application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
		} 
		else 
		{
			headers = curl_slist_append(headers, "Content-Type: text/plain");
		}
		
		headers = curl_slist_append(headers, qPrintable(QString("Slug: " + title)));
		if(etag != "")
		{
			headers = curl_slist_append(headers, qPrintable(QString("If-Match: " + etag)));
		}
	}

	QString GoogleHTTPConnector::getDocType(const QString& file_extension)
	{
		//List of supported file extensions
		if(file_extension == "doc"
			|| file_extension == "docx"
			|| file_extension == "txt"
			|| file_extension == "rtf"
			|| file_extension == "html"
			|| file_extension == "htm"
			|| file_extension == "odt")
		{
			return "document";
		}
		else if(file_extension == "xls"
			|| file_extension == "xlsx"
			|| file_extension == "ods")
		{
			return "spreadsheet";
		}
		else if(file_extension == "ppt"
			|| file_extension == "pps")
		{
			return "presentation";
		}
		else if(file_extension == "pdf")
		{
			return "pdf";
		}

		return ""; // this file extension is not supported
	}

	RESULT GoogleHTTPConnector::createFile(const QString& path
		, const QString& title
		, const QString& parentId
		, QString& response)
	{
		RESULT res = eERROR;
		char* data = 0;
		try
		{
			CURL* p_curl = curl_easy_init();
			struct curl_slist *headers = NULL;

			if(p_curl)
			{
				QString file_extension = path.mid(path.lastIndexOf(".") + 1).toLower();			
				QString docType = getDocType(file_extension);

				if(docType != "")
				{
					headers = curl_slist_append(headers, "Content-Type: application/atom+xml");
					QString url = kDocListScope + kDocListFeed_V3;

					if(parentId != ROOT_ID)
					{
						url += QString("/%1/contents").arg(parentId);
					}

					if(docType == "spreadsheet")
					{
						initCurl(p_curl, headers, url, write_str, 3, "POST", "spr");
					}
					else
					{
						initCurl(p_curl, headers, url, write_str, 3, "POST", "doc");
					}

								
					curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &response);

					QString body = QString("<?xml version=\'1.0\' encoding=\'UTF-8\'?>" 
						"<entry xmlns=\"http://www.w3.org/2005/Atom\">"
						"<category scheme=\"http://schemas.google.com/g/2005#kind\" "
						"term=\"http://schemas.google.com/docs/2007#%1\"/>"
						"<title>%2</title></entry>").arg(docType).arg(title + "." + file_extension);

					data = new char[strlen(body.toUtf8().constData()) + 1];
					strcpy(data, body.toUtf8().constData());

					curl_easy_setopt(p_curl, CURLOPT_POSTFIELDS, data);
					curl_easy_setopt(p_curl, CURLOPT_POSTFIELDSIZE, strlen(data));

					CURLcode error = curl_easy_perform(p_curl);
					long code;
					curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &code);
					res = (error == CURLE_OK && code == 201) ? eNO_ERROR : eERROR;

					/* free slist */ 
					curl_slist_free_all(headers);
					curl_easy_cleanup(p_curl);
				}
				else
				{
					return eERROR;
				}
			}
		}
		catch(...)
		{
		}

		if(data)
		{
			delete[] data;
		}
		return res;
	}

	RESULT GoogleHTTPConnector::renameElement(VFSCache::iterator iter
		, const QString& newTitle
		, QString& response)
	{
		RESULT res = eERROR;
		char* data = 0;
		QString url = iter->getEditMetaUrl();
		QString docType = "";

		if(url == "")
		{
			return res;
		}

		try
		{
			CURL* p_curl = curl_easy_init();
			struct curl_slist *headers = NULL;

			if(p_curl)
			{

				if(iter->getType() == VFSElement::FILE)
				{
					docType = getDocType(iter->getPath().mid(iter->getPath().lastIndexOf(".") + 1).toLower());
				}
				else if(iter->getType() == VFSElement::DIRECTORY)
				{
					docType = "folder";
				}

				if(docType != "")
				{
					headers = curl_slist_append(headers, "Content-Type: application/atom+xml");
					headers = curl_slist_append(headers, qPrintable(QString("If-Match: " + iter->getModified())));
					
					initCurl(p_curl, headers, url, write_str, 3, "PUT");			
					curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &response);
					
					QString body = QString("<?xml version='1.0' encoding='UTF-8'?>"
						"<entry xmlns=\"http://www.w3.org/2005/Atom\" "
						"xmlns:docs=\"http://schemas.google.com/docs/2007\" "
						"xmlns:gd=\"http://schemas.google.com/g/2005\" gd:etag=%1>"
						"<category scheme=\"http://schemas.google.com/g/2005#kind\" "
						"term=\"http://schemas.google.com/docs/2007#%2\"/>"
						"<title>%3</title>"
						"</entry>").arg(iter->getModified()).arg(docType).arg(newTitle);
					

					//data = new char[strlen(body.toLocal8Bit().constData()) + 1];
					//strcpy_s(data, strlen(body.toLocal8Bit().constData()) + 1, body.toLocal8Bit().constData());

					sPutData userdata;
					QByteArray arr = body.toUtf8();
					userdata.data = arr.constData();
					userdata.len = strlen(userdata.data);
					
					curl_easy_setopt(p_curl, CURLOPT_UPLOAD, 1L);
					curl_easy_setopt(p_curl, CURLOPT_READFUNCTION, read_str);
					curl_easy_setopt(p_curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)userdata.len);
					curl_easy_setopt(p_curl, CURLOPT_READDATA, &userdata);
					
					//curl_easy_setopt(p_curl, CURLOPT_POSTFIELDS, data);
					//curl_easy_setopt(p_curl, CURLOPT_POSTFIELDSIZE, strlen(data));

					CURLcode error = curl_easy_perform(p_curl);
					long code;
					curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &code);
					res = (error == CURLE_OK && code == 200) ? eNO_ERROR : eERROR;

					/* free slist */ 
					curl_slist_free_all(headers);
					curl_easy_cleanup(p_curl);
				}
				else
				{
					return eERROR;
				}
			}
		}
		catch(...)
		{
		}

		if(data)
		{
			delete[] data;
		}
		return res;
	}

	RESULT GoogleHTTPConnector::moveElement(VFSCache::iterator iter
											, const QString& newParentSrc
											, QString& response)
	{
		RESULT res = eERROR;
		char* data = 0;
		QString url = newParentSrc;

		if(url == "")
		{
			return res;
		}

		try
		{
			CURL* p_curl = curl_easy_init();
			struct curl_slist *headers = NULL;

			if(p_curl)
			{
				headers = curl_slist_append(headers, "Content-Type: application/atom+xml");

				initCurl(p_curl, headers, url, write_str, 3, "PUT");			
				curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &response);

				QString body = QString("<?xml version='1.0' encoding='UTF-8'?>"
					"<entry xmlns=\"http://www.w3.org/2005/Atom\">"
					"<id>%1</id>"
					"</entry>").arg(kDocListScope + kDocListFeed_V3 + "/" + iter->getId());

				//data = new char[strlen(body.toUtf8().constData()) + 1];
				//strcpy_s(data, strlen(body.toUtf8().constData()) + 1, body.toUtf8().constData());

				/*curl_easy_setopt(p_curl, CURLOPT_POSTFIELDS, data);
				curl_easy_setopt(p_curl, CURLOPT_POSTFIELDSIZE, strlen(data));*/


				sPutData userdata;
				QByteArray arr = body.toUtf8();
				userdata.data = arr.constData();
				userdata.len = strlen(userdata.data);

				curl_easy_setopt(p_curl, CURLOPT_UPLOAD, 1L);
				curl_easy_setopt(p_curl, CURLOPT_READFUNCTION, read_str);
				curl_easy_setopt(p_curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)userdata.len);
				curl_easy_setopt(p_curl, CURLOPT_READDATA, &userdata);


				CURLcode error = curl_easy_perform(p_curl);
				long code;
				curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &code);
				res = (error == CURLE_OK && code == 201) ? eNO_ERROR : eERROR;

				/* free slist */ 
				curl_slist_free_all(headers);
				curl_easy_cleanup(p_curl);

			}
		}
		catch(...)
		{
		}

		if(data)
		{
			delete[] data;
		}
		return res;
	}


	RESULT GoogleHTTPConnector::uploadFile(const QString& path, const QString& title, const QString& parentId, QString& response)
	{
		RESULT res = eERROR;
		int file_size = 0;
		char *memblock = 0; 
		QString url = kDocListScope + kDocListFeed_V3;
		struct curl_slist *headers = 0;
		CURL* p_curl = 0;

		try
		{
			p_curl = curl_easy_init();
			long code;

			if(p_curl)
			{
				if(parentId != ROOT_ID)
				{
					url += QString("/%1/contents").arg(parentId);
				}

				QString file_extension = path.mid(path.lastIndexOf(".") + 1).toLower();			
				setUploadHeader(file_extension, title, "", headers);

				ifstream file(qPrintable(path), ios::binary | ios::ate);

				if (!file) 
				{
					curl_slist_free_all(headers);
					curl_easy_cleanup(p_curl);
					return eERROR;
				} 
				else 
				{
					int file_size = static_cast<int>(file.tellg());
					memblock = new char[file_size];
					file.seekg(0, ios::beg);
					file.read(memblock, file_size);
					file.close();

					curl_easy_setopt(p_curl, CURLOPT_POSTFIELDS, memblock);
					curl_easy_setopt(p_curl, CURLOPT_POSTFIELDSIZE, file_size);
				}

				if(getDocType(file_extension) == "spreadsheet")
				{
					initCurl(p_curl, headers, url, write_str, 3, "POST", "spr");
				}
				else
				{
					initCurl(p_curl, headers, url, write_str, 3, "POST", "doc");
				}

				curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &response);

				CURLcode error = curl_easy_perform(p_curl);
				curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &code);
				res = (error == CURLE_OK && code == 201) ? eNO_ERROR : eERROR;
			}
		}
		catch(...)
		{
		}

		try
		{
			/* free slist */ 
			curl_slist_free_all(headers);
			curl_easy_cleanup(p_curl);

			if (memblock != 0) 
			{
				delete[] memblock;
			}
		}
		catch(...)
		{

		}
		return res;
	}
	
	RESULT GoogleHTTPConnector::uploadFile(VFSCache::iterator iter, QString& response)
	{
		RESULT res = eERROR;
		QString url = iter->getEditMediaUrl();
		int file_size = 0;

		try
		{
			if(url == "")
			{
				return res;
			}

			CURL* p_curl = curl_easy_init();
			if(p_curl)
			{
				struct curl_slist *headers = NULL;

				QString file_extension = iter->getPath().mid(iter->getPath().lastIndexOf(".") + 1).toLower();			
				setUploadHeader(file_extension, iter->getName(),iter->getModified(), headers);

				if(getDocType(file_extension) == "spreadsheet")
				{
					initCurl(p_curl, headers, url, write_str, 3, "PUT", "spr");
				}
				else
				{
					initCurl(p_curl, headers, url, write_str, 3, "PUT", "doc");
				}

				curl_easy_setopt(p_curl, CURLOPT_UPLOAD, 1L);
				curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &response);

				//Reading file for uploading
				ifstream file(qPrintable(iter->getPath()), ios::binary | ios::ate);
				
				if (!file) 
				{
					curl_slist_free_all(headers);
					curl_easy_cleanup(p_curl);
					return eERROR;
				} 
				else 
				{
					file_size = static_cast<int>(file.tellg());
					file.seekg(0, ios::beg);
					file.close();
					bool open = file.is_open();

					FileHandle handle(fopen(qPrintable(iter->getPath()), "rb"));
					if(!handle)
					{
						curl_slist_free_all(headers);
						curl_easy_cleanup(p_curl);
						return eERROR;
					}

					curl_easy_setopt(p_curl, CURLOPT_READFUNCTION, read_b);
					curl_easy_setopt(p_curl, CURLOPT_READDATA, handle.getPtr());
					curl_easy_setopt(p_curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_size);

					CURLcode error = curl_easy_perform(p_curl);
					long code;
					curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &code);
					res = (error == CURLE_OK && code == 200) ? eNO_ERROR : eERROR;

					/* free slist */ 
					curl_slist_free_all(headers);
					curl_easy_cleanup(p_curl);
				}
			}
		}
		catch(...)
		{
		}

		return res;
	}
	
	////////////////////////////////////////////////////////////////////////////////
	// Private members
	////////////////////////////////////////////////////////////////////////////////

	// This is the writer call back function used by curl
	size_t GoogleHTTPConnector::write_str(void *data
		, size_t size
		, size_t nmemb
		, void *buffer) 
	{
		int result = 0;

		if (buffer != NULL) 
		{
			((QString*)buffer)->append(QString::fromUtf8((char*)data, nmemb));
			result = size * nmemb;  // How much did we write?
		}
		return result;
	}

	size_t GoogleHTTPConnector::read_str(void *ptr
		, size_t size
		, size_t nmemb
		, void *stream) 
	{
		size_t res = 0;
		if(stream)
		{
			sPutData* userdata = (sPutData*)stream;

			size_t curl_size = nmemb * size;
			res = (userdata->len < curl_size) ? userdata->len : curl_size;
			memcpy(ptr, userdata->data, res);
			userdata->len -= res;
			userdata->data += res;
 		}

		return res;
	}
	
	size_t GoogleHTTPConnector::read_b(void *ptr, size_t size, size_t nmemb, void *stream)
	{
		size_t retcode;
 
		retcode = fread(ptr, size, nmemb, (FILE*)stream);
		//ptr = stream;

		return retcode;
	}
	size_t GoogleHTTPConnector::fwrite_b(void *ptr, size_t size, size_t count, void *path) 
	{
		//QMutexLocker locker(&_connectorMutex); 		
		if(path)
		{
			QFile file(*(QString*)path);
			bool res = file.open(QIODevice::WriteOnly | QIODevice::Append);
			size_t result = file.write((char*)ptr, size*count);
			file.close();
			return result;
		}
		return -1;
	}
};