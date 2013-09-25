#include "common_stuff.h"

#include "jmm_connector.h"
#include "../xml/jmm_xml.h"
#include "data.h"
#include "webmounter.h"

#include <QFile>
#include <QDir>

namespace Connector
{
	using namespace Common;

	void JmmHTTPConnector::setSettings(const QString& url
		, const QString& login
		, const QString& password
		, const QString& proxy
		, const QString& proxyLoginPwd
		, const QString& pluginName)
	{
		_url = url;
		_login = login;
		_password = password;
		_proxy = proxy;
		_proxy_login_pwd = proxyLoginPwd;

		_curl = curl_easy_init();

		if(_proxy != ":")
		{
			curl_easy_setopt(_curl, CURLOPT_PROXY, qPrintable(_proxy));
			curl_easy_setopt(_curl, CURLOPT_PROXYUSERPWD, qPrintable(_proxy_login_pwd));
		}

		SettingStorage* settings = Common::WebMounter::getSettingStorage();
		QString cookie_file		 = settings->getAppSettingStoragePath() + QDir::separator() + pluginName + QDir::separator() + "cookie.txt";

		QDir dir;

		if(!dir.exists(settings->getAppSettingStoragePath() + QDir::separator() + pluginName))
		{
			dir.mkpath(settings->getAppSettingStoragePath() + QDir::separator() + pluginName);
		}

		// Read cookies from a previous session, as stored in MyCookieFileName.
		curl_easy_setopt( _curl, CURLOPT_COOKIEFILE, qPrintable(cookie_file) );
		// Save cookies from *this* session in MyCookieFileName
		curl_easy_setopt( _curl, CURLOPT_COOKIEJAR, qPrintable(cookie_file) );

		_chunk = NULL;
		_chunk = curl_slist_append(_chunk, "Cache-Control: no-store, no-cache, must-revalidate");

		curl_easy_setopt( _curl, CURLOPT_HTTPHEADER, _chunk);
	}

	JmmHTTPConnector::~JmmHTTPConnector()
	{
		if(_curl)  curl_easy_cleanup(_curl);
		//if(_chunk) curl_slist_free_all(_chunk);
	}
	
	RESULT JmmHTTPConnector::auth()
	{
		QMutexLocker locker(&_connectorMutex);

		try
		{
			if(_curl) 
			{
				QString response;
				QString url = QString("%1/index2.php?option=com_jmediamanager&no_html=1&controller=server&login=%2"
					"&password=%3&func=auth&no_html=1")
					.arg(_url).arg(_login).arg(_password);

				curl_easy_setopt(_curl, CURLOPT_URL, qPrintable(url));
				curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, write_str);
				curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &response);
				CURLcode _error = curl_easy_perform(_curl);

				if(!_error)
				{
					Xml::JmmXmlParser::parseAuthResponse(response);

					if(response.length() > 32 || response.length() < 20 )
					{
						return eERROR;
					}
					else
					{
						return eNO_ERROR;
					}
				}
			}
		}
		catch(...)
		{

		}
		return eERROR;
	}
	size_t JmmHTTPConnector::read_callback(void *ptr, size_t size, size_t nmemb, void *userp) 
	{
		return 0;
	}
	size_t JmmHTTPConnector::write_str(void *ptr, size_t size, size_t count, void *response) 
	{
		try
		{
			if(response && ptr)
			{
				*((QString*)response) = QString::fromUtf8((char*)ptr, count);
				return size*count;
			}
		}
		catch(...)
		{}
		return -1;
	}
	size_t JmmHTTPConnector::fwrite_ch(void *ptr, size_t size, size_t count, void *path) 
	{
		if(path)
		{
			QFile file(*(QString*)path);
			file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
			size_t result = file.write((char*)ptr, size*count);
			return result;
		}
		return -1;
		//((wstring*)stream)->append((wchar_t*)ptr, 0, size*count);
	}

	size_t JmmHTTPConnector::fwrite_b(void *ptr, size_t size, size_t count, void *path) 
	{
		//QMutexLocker locker(&_connectorMutex);
		if(path)
		{
			QFile file(*(QString*)path);
			file.open(QIODevice::WriteOnly | QIODevice::Append);
			size_t result = file.write((char*)ptr, size*count);
			return result;
		}
		return -1;
	}

	RESULT JmmHTTPConnector::getTreeElements(const QString& path, const QString& pluginName)
	{
		try
		{
			if(_curl) 
			{
				QString url = QString("%1/index2.php?option=com_jmediamanager&no_html=1&controller=server&login=%2"
					"&func=getXMLTree&no_html=1&plugin=%3")
					.arg(_url).arg(_login).arg(pluginName);

				curl_easy_setopt(_curl, CURLOPT_URL, qPrintable(url));
				curl_easy_setopt(_curl, CURLOPT_HTTPPOST, 0);
				curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, fwrite_ch);
				curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &path);

				CURLcode code = curl_easy_perform(_curl);
				return (RESULT)code;
			}
		}
		catch(...)
		{
		}
		return eERROR;
	}
	RESULT JmmHTTPConnector::downloadFile(const QString& url, const QString& path)
	{
		QMutexLocker locker(&_connectorMutex);
		
		try
		{
			if(_curl) 
			{
				/*if(_proxy != "")
				{
					curl_easy_setopt(_curl, CURLOPT_PROXY, qPrintable(_proxy));
					curl_easy_setopt(_curl, CURLOPT_PROXYUSERPWD, qPrintable(_proxy_login_pwd));
				}*/

				curl_easy_setopt(_curl, CURLOPT_URL, qPrintable(url));
				curl_easy_setopt(_curl, CURLOPT_READFUNCTION, read_callback);
				curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, fwrite_b);
				curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &path);

				CURLcode _error = curl_easy_perform(_curl);
				
				curl_easy_setopt(_curl, CURLOPT_URL, 0);
				curl_easy_setopt(_curl, CURLOPT_HTTPPOST, 0);
				curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, 0);
				curl_easy_setopt(_curl, CURLOPT_WRITEDATA, 0);

				return (RESULT)_error;
			}
		}
		catch(...)
		{	
		}
		return eERROR;
	}
	RESULT JmmHTTPConnector::uploadFile(const QString& path
										, const QString& title
										, const QString& id
										, const QString& catId
										, QString& response
										, const QString& pluginName)
	{
		QMutexLocker locker(&_connectorMutex);
		CURL* p_curl = curl_easy_init();
		if(_proxy != ":")
		{
			curl_easy_setopt(p_curl, CURLOPT_PROXY, qPrintable(_proxy));
			curl_easy_setopt(p_curl, CURLOPT_PROXYUSERPWD, qPrintable(_proxy_login_pwd));
		}

		SettingStorage* settings = Common::WebMounter::getSettingStorage();
		QString cookie_file		 = settings->getAppSettingStoragePath() + QDir::separator() + pluginName + QDir::separator() + "cookie.txt";

		// Read cookies from a previous session, as stored in MyCookieFileName.
		curl_easy_setopt( p_curl, CURLOPT_COOKIEFILE, qPrintable(cookie_file) );
		// Save cookies from *this* session in MyCookieFileName
		curl_easy_setopt( p_curl, CURLOPT_COOKIEJAR, qPrintable(cookie_file) );

		curl_slist * chunk = NULL;
		chunk = curl_slist_append(_chunk, "Cache-Control: no-store, no-cache, must-revalidate");

		curl_easy_setopt( p_curl, CURLOPT_HTTPHEADER, chunk);
		
		struct curl_httppost *post = NULL;
		struct curl_httppost *last = NULL;

		if(p_curl) 
		{
			if(pluginName == "Joomla.Article")
			{
				QString content;
				QFile file(path);
				if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
				{
					return eERROR;
				}
				else
				{
					QTextStream in(&file);
					in.setCodec( QTextCodec::codecForHtml(file.readAll(), QTextCodec::codecForLocale()));
					file.seek(0);
					in.seek(0);
					while (!in.atEnd()) 
					{
						content += in.readLine();
					}

					file.close();
				}
				curl_formadd(&post, &last, CURLFORM_COPYNAME, "text", CURLFORM_COPYCONTENTS, content.toUtf8().constData(), CURLFORM_END);
			}
			else
			{
				curl_formadd(&post, &last, CURLFORM_COPYNAME, "upload_field", CURLFORM_FILE, path.toLocal8Bit().constData(), CURLFORM_END);
			}

			curl_formadd(&post, &last, CURLFORM_COPYNAME, "title", CURLFORM_COPYCONTENTS, title.toUtf8().constData(), CURLFORM_END);
			
			QString url = QString(
				"%1/index2.php?option=com_jmediamanager&no_html=1&controller=server&login=%2&catid=%3&func=file_upload&id=%4&plugin=%5")
				.arg(_url)
				.arg(_login)
				.arg(catId)
				.arg(id)
				.arg(pluginName);

			curl_easy_setopt(p_curl, CURLOPT_URL, qPrintable(url));
			curl_easy_setopt(p_curl, CURLOPT_HTTPPOST, post);
			curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, write_str);
			curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &response);

			CURLcode _error = curl_easy_perform(p_curl);

			curl_formfree(post);
			curl_easy_setopt(p_curl, CURLOPT_URL, 0);
			curl_easy_setopt(p_curl, CURLOPT_HTTPPOST, 0);
			curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, 0);
			curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, 0);

			curl_easy_cleanup(p_curl);
			
			return (RESULT)_error;
		}
		return eERROR;
	}
	RESULT JmmHTTPConnector::deleteFile(const QString& photoid, QString& response, const QString& pluginName)
	{
		QMutexLocker locker(&_connectorMutex); 
		try
		{
			if(_curl) 
			{
				QString url = QString( 
					"%1/index2.php?option=com_jmediamanager&no_html=1&controller=server&login=%2"
					"&param1=%3&func=edit&type=deletefoto&plugin=%4")
					.arg(_url)
					.arg(_login)
					.arg(photoid)
					.arg(pluginName);
				curl_easy_setopt(_curl,CURLOPT_URL, qPrintable(url));
				curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, write_str);
				curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &response);
				CURLcode _error = curl_easy_perform(_curl);

				curl_easy_setopt(_curl,CURLOPT_URL, 0);
				curl_easy_setopt(_curl, CURLOPT_HTTPPOST, 0);
				curl_easy_setopt(_curl, CURLOPT_WRITEDATA, 0);
				return (RESULT)_error;
			}
		}
		catch(...)
		{
		}

		return eERROR;
	}
	RESULT JmmHTTPConnector::renameElement(const QString& id, ElementType type, const QString& newTitle, QString& response, const QString& pluginName)
	{
		try
		{
			struct curl_httppost *post = NULL;
			struct curl_httppost *last = NULL;


			if(_curl) 
			{
				QString url;

				if(type == VFSElement::DIRECTORY)
				{
					curl_formadd(&post, &last, CURLFORM_COPYNAME, "param2", CURLFORM_COPYCONTENTS, newTitle.toUtf8().constData(), CURLFORM_END);

					url = QString(
						"%1/index2.php?option=com_jmediamanager&no_html=1&controller=server&login=%2"
						"&param1=%3&func=edit&type=renamecat&plugin=%4")
						.arg(_url)
						.arg(_login)
						.arg(id)
						.arg(pluginName);
				}
				else if(type == VFSElement::FILE)
				{
					curl_formadd(&post, &last, CURLFORM_COPYNAME, "param3", CURLFORM_COPYCONTENTS, newTitle.toUtf8().constData(), CURLFORM_END);

					url = QString(
						"%1/index2.php?option=com_jmediamanager&no_html=1&controller=server&login=%2"
						"&param1=%3&param2=imgtitle&func=edit&type=editfoto&plugin=%4")
						.arg(_url)
						.arg(_login)
						.arg(id)
						.arg(pluginName);
				}
				curl_easy_setopt(_curl,CURLOPT_URL, qPrintable(url));
				curl_easy_setopt(_curl, CURLOPT_HTTPPOST, post);
				curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, write_str);
				curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &response);
				CURLcode _error = curl_easy_perform(_curl);
				
				curl_formfree(post);
				curl_easy_setopt(_curl,CURLOPT_URL, 0);
				curl_easy_setopt(_curl, CURLOPT_HTTPPOST, 0);
				curl_easy_setopt(_curl, CURLOPT_WRITEDATA, 0);
				return (RESULT)_error;
			}
		}
		catch(...)
		{
		}

		return eERROR;
	}

	RESULT JmmHTTPConnector::deleteDirectory(const QString& dirid, QString& response, const QString& pluginName)
	{
		try
		{
			if(_curl) 
			{
				QString url = QString(
					"%1/index2.php?option=com_jmediamanager&no_html=1&controller=server&login=%2"
					"&param1=%3&func=edit&type=deletecat&plugin=%4")
					.arg(_url)
					.arg(_login)
					.arg(dirid)
					.arg(pluginName);
				curl_easy_setopt(_curl,CURLOPT_URL, qPrintable(url));
				curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, write_str);
				curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &response);
				bool _error = curl_easy_perform(_curl);

				curl_easy_setopt(_curl,CURLOPT_URL, 0);
				curl_easy_setopt(_curl, CURLOPT_HTTPPOST, 0);
				curl_easy_setopt(_curl, CURLOPT_WRITEDATA, 0);

				return (RESULT)_error;
			}
		}
		catch(...)
		{
		}

		return eERROR;
	}

	RESULT JmmHTTPConnector::createDirectory(const QString& parentid, const QString& title, QString& response, const QString& pluginName)
	{
		try
		{
			struct curl_httppost *post = NULL;
			struct curl_httppost *last = NULL;

			if(_curl) 
			{
				curl_formadd(&post, &last, CURLFORM_COPYNAME, "param2", CURLFORM_COPYCONTENTS, title.toUtf8().constData(), CURLFORM_END);
				QString url = QString( 
					"%1/index2.php?option=com_jmediamanager&no_html=1&controller=server&login=%2"
					"&param1=%3&param6=1&func=edit&type=createcat&plugin=%4")
					.arg(_url)
					.arg( _login)
					.arg(parentid)
					.arg(pluginName);

				curl_easy_setopt(_curl,CURLOPT_URL, qPrintable(url));
				curl_easy_setopt(_curl, CURLOPT_HTTPPOST, post);
				curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, write_str);
				curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &response);
				CURLcode _error = curl_easy_perform(_curl);

				curl_formfree(post);
				curl_easy_setopt(_curl,CURLOPT_URL, 0);
				curl_easy_setopt(_curl, CURLOPT_HTTPPOST, 0);
				curl_easy_setopt(_curl, CURLOPT_WRITEDATA, 0);
				
				return (RESULT)_error;
			}
		}
		catch(...)
		{
		}
		return eERROR;
	}

	RESULT JmmHTTPConnector::moveElement(const QString& id, const QString& oldParentId, const QString& newParentId, ElementType type, QString& response, const QString& pluginName)
	{
		char* url = 0;
		QString elemType;
		if(type == VFSElement::DIRECTORY)
		{
			elemType = "cat";
		}
		else if(type == VFSElement::FILE)
		{
			elemType = "foto";
		}
		else
		{
			return eERROR;
		}

		try
		{
			if(_curl) 
			{
				QString url = QString( 
					"%1/index2.php?option=com_jmediamanager&no_html=1&controller=server&login=%2"
					"&param1=%3&param2=%4&param3=%5&func=edit&type=move&plugin=%6")
					.arg(_url)
					.arg(_login)
					.arg(elemType)
					.arg(id)
					.arg(newParentId)
					.arg(pluginName);

				curl_easy_setopt(_curl,CURLOPT_URL, qPrintable(url));
				curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, write_str);
				curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &response);
				CURLcode _error = curl_easy_perform(_curl);

				return (RESULT)_error;
			}
		}
		catch(...)
		{
		}
		return eERROR;
	}
};
