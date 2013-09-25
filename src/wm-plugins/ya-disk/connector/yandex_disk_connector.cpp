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

#include <curl/curl.h>
#include <QFile>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include "yandex_disk_connector.h"
#include "reg_exp.h"
#include <QFileInfo>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef Q_OS_WIN
#include <io.h>
#endif



namespace Connector
{
	using namespace Common;
	using namespace Data;
    //	QMutex YaDiskHTTPConnector::m_connectorMutex;

	size_t YaDiskHTTPConnector::writeStr(void *ptr, size_t size, size_t count, void *response)
	{
		if(response)
		{
			((QString*)response)->append(QString::fromUtf8((char*)ptr, count));
			return size*count;
		}
		return -1;
	}

	size_t YaDiskHTTPConnector::fwrite_b(void *ptr, size_t size, size_t count, void *path) 
	{
        //QMutexLocker locker(&m_connectorMutex);
		if(path)
		{
			QFile file(*(QString*)path);
			file.open(QIODevice::WriteOnly | QIODevice::Append);
			size_t result = file.write((char*)ptr, size*count);
			return result;
		}
		return -1;
	}

	size_t YaDiskHTTPConnector::readStr(void *ptr, size_t size, size_t nmemb, void *stream)
	{
		size_t res = 0;
		if(stream)
		{
			sPutData* userdata = (sPutData*)stream;

			size_t curl_size = nmemb * size;
            res = (userdata->m_len < curl_size) ? userdata->m_len : curl_size;
            memcpy(ptr, userdata->m_data, res);
            userdata->m_len -= res;
            userdata->m_data += res;
		}

		return res;
	} 

	YaDiskHTTPConnector::YaDiskHTTPConnector()
	{
	}

	YaDiskHTTPConnector::~YaDiskHTTPConnector()
	{
	}

	void YaDiskHTTPConnector::setSettings(const QString& login
		, const QString& password
		, const QString& proxy
		, const QString& proxyLoginPwd
		, bool isOAuth
		, const QString& token)
	{
        m_login = login.left(login.lastIndexOf("@"));
        m_password = password;
        m_proxy = proxy;
        m_proxyLoginPwd = proxyLoginPwd;
        m_isOAuth = isOAuth;
        m_token = token;
	}

	int YaDiskHTTPConnector::execQuery(const QString &url, const QString &header, const QString& postFields, QString* response)
	{
		int res = 0;
		CURL* p_curl = curl_easy_init();
		if(p_curl)
		{
            if((m_proxy != "") && (m_proxy != ":"))
			{
                curl_easy_setopt(p_curl, CURLOPT_PROXY, qPrintable(m_proxy));
                curl_easy_setopt(p_curl, CURLOPT_PROXYUSERPWD, qPrintable(m_proxyLoginPwd));
			}

			curl_easy_setopt(p_curl, CURLOPT_URL, qPrintable(url));
			curl_easy_setopt(p_curl, CURLOPT_VERBOSE, 1L);

			struct curl_slist *chunk = NULL;
			chunk = curl_slist_append(chunk, "Cache-Control: no-store, no-cache, must-revalidate");

			if(header.length())
			{
				chunk = curl_slist_append(chunk, qPrintable(header));
			}

			curl_easy_setopt(p_curl, CURLOPT_HTTPHEADER, chunk); 

			QByteArray postArray = postFields.toUtf8();
			if(postFields.length())
			{
				curl_easy_setopt(p_curl, CURLOPT_POSTFIELDS, postArray.constData());
				curl_easy_setopt(p_curl, CURLOPT_POST, 1L);
			}

			curl_easy_setopt(p_curl, CURLOPT_WRITEHEADER, writeStr);
			curl_easy_setopt(p_curl, CURLOPT_HEADERDATA, response);
			curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, writeStr);
			curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, response);

			CURLcode error = curl_easy_perform(p_curl);
			long code;
			curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &code);
			res = (error == CURLE_OK) ? code : error;
			curl_easy_cleanup(p_curl);
			curl_slist_free_all(chunk);
		}
		return res;
	}

	RESULT YaDiskHTTPConnector::getTreeElements(const QString& path, QString& response)
	{
		RESULT res = eERROR_GENERAL;
		CURL* p_curl = curl_easy_init();
		if(p_curl)
		{
            if((m_proxy != "") && (m_proxy != ":"))
			{
                curl_easy_setopt(p_curl, CURLOPT_PROXY, qPrintable(m_proxy));
                curl_easy_setopt(p_curl, CURLOPT_PROXYUSERPWD, qPrintable(m_proxyLoginPwd));
			}

			QString url = QString("https://webdav.yandex.ru/%1").arg(path);

			curl_easy_setopt(p_curl, CURLOPT_URL, qPrintable(url));

			struct curl_slist *chunk = NULL;
            QString header = QString("Authorization: OAuth %1").arg(m_token);

			chunk = curl_slist_append(chunk, qPrintable(header));

			QString contentType = "Content-Type: text/xml";
			chunk = curl_slist_append(chunk, qPrintable(contentType));

			QString useragent = "User-Agent: WebMounter";
			chunk = curl_slist_append(chunk, qPrintable(useragent));

			QString depth = "Depth: 1";
			chunk = curl_slist_append(chunk, qPrintable(depth));

			curl_easy_setopt(p_curl, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt(p_curl, CURLOPT_HTTPHEADER, chunk);
			curl_easy_setopt(p_curl, CURLOPT_CUSTOMREQUEST, "PROPFIND");
			curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, writeStr);
			curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &response);
			curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYPEER, FALSE);

			CURLcode _error = curl_easy_perform(p_curl);
			long code;
			curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &code);

			res = (_error == CURLE_OK && code == 207) ? eNO_ERROR : eERROR_GENERAL;
			curl_easy_cleanup(p_curl);
		}

		return res;
	}

	RESULT YaDiskHTTPConnector::createDirectory(const QString& title, const QString& parentId, QString& response)
	{
		RESULT res = eERROR_GENERAL;
		CURL* p_curl = curl_easy_init();
		if(p_curl)
		{
            if((m_proxy != "") && (m_proxy != ":"))
			{
                curl_easy_setopt(p_curl, CURLOPT_PROXY, qPrintable(m_proxy));
                curl_easy_setopt(p_curl, CURLOPT_PROXYUSERPWD, qPrintable(m_proxyLoginPwd));
			}

			QString path = title;
			if(parentId != "0")
			{
				path = QString("%1/%2").arg(parentId).arg(title);				
			}

			QString url = QString("https://webdav.yandex.ru/%1").arg(path);

			curl_easy_setopt(p_curl, CURLOPT_URL, qPrintable(url));

			struct curl_slist *chunk = NULL;
            QString header = QString("Authorization: OAuth %1").arg(m_token);

			chunk = curl_slist_append(chunk, qPrintable(header));

			QString contentType = "Content-Type: text/xml";
			chunk = curl_slist_append(chunk, qPrintable(contentType));

			QString useragent = "User-Agent: WebMounter";
			chunk = curl_slist_append(chunk, qPrintable(useragent));

			QString depth = "Depth: 1";
			chunk = curl_slist_append(chunk, qPrintable(depth));

			curl_easy_setopt(p_curl, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt(p_curl, CURLOPT_HTTPHEADER, chunk);
			curl_easy_setopt(p_curl, CURLOPT_CUSTOMREQUEST, "MKCOL");
			curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, writeStr);
			curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &response);
			curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYPEER, FALSE);

			CURLcode _error = curl_easy_perform(p_curl);
			long code;
			curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &code);

			res = (_error == CURLE_OK && code == 201) ? eNO_ERROR : eERROR_GENERAL;
			curl_easy_cleanup(p_curl);
		}

		return res;
	}

	RESULT YaDiskHTTPConnector::deleteFile(const QString& path, QString& response)
	{
		RESULT res = eERROR_GENERAL;

		CURL* p_curl = curl_easy_init();
		if(p_curl)
		{
            if((m_proxy != "") && (m_proxy != ":"))
			{
                curl_easy_setopt(p_curl, CURLOPT_PROXY, qPrintable(m_proxy));
                curl_easy_setopt(p_curl, CURLOPT_PROXYUSERPWD, qPrintable(m_proxyLoginPwd));
			}

			QString url = QString::fromAscii("https://webdav.yandex.ru/%1").arg(path);

			curl_easy_setopt(p_curl, CURLOPT_URL, qPrintable(url));

			struct curl_slist *chunk = NULL;
            QString header = QString("Authorization: OAuth %1").arg(m_token);

			chunk = curl_slist_append(chunk, qPrintable(header));

			curl_easy_setopt(p_curl, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt(p_curl, CURLOPT_HTTPHEADER, chunk);
			curl_easy_setopt(p_curl, CURLOPT_CUSTOMREQUEST, "DELETE");
			curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, writeStr);
			curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &response);
			curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYPEER, 0L);

			CURLcode _error = curl_easy_perform(p_curl);
			long code;
			curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &code);

			res = (_error == CURLE_OK && code == 200) ? eNO_ERROR : eERROR_GENERAL;
			curl_easy_cleanup(p_curl);
		}

		return res;
	}

	size_t YaDiskHTTPConnector::read_callback(void *ptr, size_t size, size_t nmemb, void *stream)
	{
		size_t retcode;

		/* in real-world cases, this would probably get this data differently
		as this fread() stuff is exactly what the library already would do
		by default internally */
		retcode = fread(ptr, size, nmemb, (FILE*)stream);

		fprintf(stderr, "*** We read %d bytes from file\n", retcode);

		return retcode;
	}

	RESULT YaDiskHTTPConnector::uploadFile(const QString& path, const QString& title, const QString& /*parentId*/, QString& response)
	{
		RESULT res = eERROR_GENERAL;

		CURL* p_curl = curl_easy_init();
		if(p_curl)
		{
			QString fileName = title + "." + QFileInfo(path).suffix().toLower();

			QString url = QString("https://webdav.yandex.ru/%1").arg(fileName);
			curl_easy_setopt(p_curl, CURLOPT_URL, qPrintable(url));

			curl_easy_setopt(p_curl, CURLOPT_UPLOAD, 1L);
			curl_easy_setopt(p_curl, CURLOPT_PUT, 1L);
			curl_easy_setopt(p_curl, CURLOPT_VERBOSE, 1L);

			struct curl_slist *chunk = NULL;
			//QString contentType = "Content-Type: application/atom+xml; charset=utf-8; type=entry";
			//chunk = curl_slist_append(chunk, qPrintable(contentType));
            QString authorization = QString("Authorization: OAuth %1").arg(m_token);
			chunk = curl_slist_append(chunk, qPrintable(authorization));

			curl_easy_setopt(p_curl, CURLOPT_HTTPHEADER, chunk);

			/* get the file size of the local file */
			struct stat file_info;
			int hd = open(qPrintable(path), O_RDONLY) ;
			fstat(hd, &file_info);
			close(hd);

			FILE* hd_src = fopen(qPrintable(path), "rb");

			curl_easy_setopt(p_curl, CURLOPT_READFUNCTION, read_callback);
			curl_easy_setopt(p_curl, CURLOPT_READDATA, hd_src);
			curl_easy_setopt(p_curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);
			curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYPEER, 0L);

			curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, writeStr);

			curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &response);

			CURLcode err = curl_easy_perform(p_curl);
			long code;
			curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &code);
			res = (err == CURLE_OK && code == 201) ? eNO_ERROR : eERROR_GENERAL;

			curl_easy_cleanup(p_curl);
			fclose(hd_src);
		}

		return res;
	}

	RESULT YaDiskHTTPConnector::downloadFile(const QString& url, const QString& path)
	{
        QMutexLocker locker(&m_connectorMutex);

		RESULT res = eERROR_GENERAL;
		CURL* p_curl = curl_easy_init();
		if(p_curl)
		{
            if((m_proxy != "") && (m_proxy != ":"))
			{
                curl_easy_setopt(p_curl, CURLOPT_PROXY, qPrintable(m_proxy));
                curl_easy_setopt(p_curl, CURLOPT_PROXYUSERPWD, qPrintable(m_proxyLoginPwd));
			}

			curl_easy_setopt(p_curl, CURLOPT_URL, qPrintable(url));

			curl_easy_setopt(p_curl, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, fwrite_b);
			curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &path);

			CURLcode _error = curl_easy_perform(p_curl);
			long code;
			curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &code);
			res = (_error == CURLE_OK && code == 200) ? eNO_ERROR : eERROR_GENERAL;
			curl_easy_cleanup(p_curl);
		}

		return res;
	}

	RESULT YaDiskHTTPConnector::downloadFiles(const QList <QString>& urlList, const QList <QString>& pathList)
	{
        QMutexLocker locker(&m_connectorMutex);

		RESULT res = eNO_ERROR;
		QList <CURL*> curls;
		CURLM* p_mcurl = curl_multi_init();

		for(int i = 0; i < urlList.size(); i++)
		{
			CURL* p_curl = curl_easy_init();
			if(p_curl)
			{
                if((m_proxy != "") && (m_proxy != ":"))
				{
                    curl_easy_setopt(p_curl, CURLOPT_PROXY, qPrintable(m_proxy));
                    curl_easy_setopt(p_curl, CURLOPT_PROXYUSERPWD, qPrintable(m_proxyLoginPwd));
				}

				QString url = QString("https://webdav.yandex.ru/%1").arg(urlList.at(i));
				curl_easy_setopt(p_curl, CURLOPT_URL, qPrintable(url));

				struct curl_slist *chunk = NULL;
                QString authorization = QString("Authorization: OAuth %1").arg(m_token);
				chunk = curl_slist_append(chunk, qPrintable(authorization));

				curl_easy_setopt(p_curl, CURLOPT_HTTPHEADER, chunk);

				curl_easy_setopt(p_curl, CURLOPT_VERBOSE, 1L);
				curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, fwrite_b);
				curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &pathList.at(i));
				curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYPEER, 0L);
			}
			curls.append(p_curl);
			curl_multi_add_handle(p_mcurl, p_curl);
		}

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
			if((RESULT)msg->data.result != eNO_ERROR)
			{
				if(curl_easy_perform(msg->easy_handle) != CURLE_OK)
				{
					res = eERROR_GENERAL;
					break;
				}
			}
		}
		while(msgs_in_queue);

		for(int i = 0; i < urlList.size(); i++)
		{
			curl_multi_remove_handle(p_mcurl, curls.at(i)); 
			curl_easy_cleanup(curls.at(i));
		}

		curl_multi_cleanup(p_mcurl);
		return res;
	}


	RESULT YaDiskHTTPConnector::moveElement(const QString& id, const QString& oldParentId, const QString& newParentId, ElementType /*type*/, QString& response)
	{
		RESULT res = eERROR_GENERAL;
		CURL* p_curl = curl_easy_init();
		if(p_curl)
		{
            if((m_proxy != "") && (m_proxy != ":"))
			{
                curl_easy_setopt(p_curl, CURLOPT_PROXY, qPrintable(m_proxy));
                curl_easy_setopt(p_curl, CURLOPT_PROXYUSERPWD, qPrintable(m_proxyLoginPwd));
			}

			QString pathFrom = id;
			if(oldParentId != "0")
			{
				pathFrom = QString("%1/%2").arg(oldParentId).arg(pathFrom);				
			}

			QString url = QString("https://webdav.yandex.ru/%1").arg(pathFrom);

			curl_easy_setopt(p_curl, CURLOPT_URL, qPrintable(url));

			struct curl_slist *chunk = NULL;
            QString header = QString("Authorization: OAuth %1").arg(m_token);

			chunk = curl_slist_append(chunk, qPrintable(header));

			QString pathTo = id;
			if(newParentId != "0")
			{
				pathTo = QString("%1/%2").arg(newParentId).arg(pathTo);				
			}

			QString destination = QString("Destination: /%1").arg(pathTo);
			chunk = curl_slist_append(chunk, qPrintable(destination));

			QString useragent = "User-Agent: WebMounter";
			chunk = curl_slist_append(chunk, qPrintable(useragent));

			curl_easy_setopt(p_curl, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt(p_curl, CURLOPT_HTTPHEADER, chunk);
			curl_easy_setopt(p_curl, CURLOPT_CUSTOMREQUEST, "MOVE");
			curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, writeStr);
			curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &response);
			curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYPEER, FALSE);

			CURLcode _error = curl_easy_perform(p_curl);
			long code;
			curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &code);

			res = (_error == CURLE_OK && code == 201) ? eNO_ERROR : eERROR_GENERAL;
			curl_easy_cleanup(p_curl);
		}

		return res;
	}

	RESULT YaDiskHTTPConnector::renameElement(const QString& id, ElementType type, const QString& newTitle, QString& response)
	{
		RESULT res = eERROR_GENERAL;

		QString typeStr = type == (VFSElement::DIRECTORY) ? QString("album") : QString("photo");
        QString url = QString("http://api-fotki.yandex.ru/api/users/%1/%2/%3/").arg(m_login).arg(typeStr).arg(id);
        QString header = m_isOAuth ? QString("Authorization: OAuth %1").arg(m_token)
            : QString("Authorization: FimpToken realm=\"fotki.yandex.ru\", token=\"%1\"").arg(m_token);

		int err = execQuery(url, header, "", &response);
		if(err == 200)
		{
			QString entry = RegExp::getByPattern("<entry (.*)</entry>", response);
			QString title = QString("<title>%1</title>").arg(RegExp::getByPattern("<title>(.*)</title>", entry));
			QString to = QString("<title>%1</title>").arg(newTitle);
			entry.replace(title, to);

			CURL* p_curl = curl_easy_init();
			if(p_curl) 
			{
				curl_easy_setopt(p_curl, CURLOPT_URL, qPrintable(url));

				curl_easy_setopt(p_curl, CURLOPT_UPLOAD, 1L);
				curl_easy_setopt(p_curl, CURLOPT_PUT, 1L);
				curl_easy_setopt(p_curl, CURLOPT_VERBOSE, 1L);

				struct curl_slist *chunk = NULL;
				QString contentType = "Content-Type: application/atom+xml; charset=utf-8; type=entry";
				chunk = curl_slist_append(chunk, qPrintable(contentType));
                QString authorization = m_isOAuth ? QString("Authorization: OAuth %1").arg(m_token)
                    : QString("Authorization: FimpToken realm=\"fotki.yandex.ru\", token=\"%1\"").arg(m_token);
				chunk = curl_slist_append(chunk, qPrintable(authorization));

				curl_easy_setopt(p_curl, CURLOPT_HTTPHEADER, chunk); 

				QString datastr = QString("<entry %1</entry>").arg(entry);
				sPutData userdata;
				QByteArray arr = datastr.toUtf8();
                userdata.m_data = arr.constData();
                userdata.m_len = strlen(userdata.m_data);

				curl_easy_setopt(p_curl, CURLOPT_READFUNCTION, readStr);
				curl_easy_setopt(p_curl, CURLOPT_READDATA, &userdata);
                curl_easy_setopt(p_curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)userdata.m_len);
				curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, writeStr);

				curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &response);

				CURLcode err = curl_easy_perform(p_curl);
				long code;
				curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &code);
				res = (err == CURLE_OK && code == 200) ? eNO_ERROR : eERROR_GENERAL;

				curl_easy_cleanup(p_curl);
			}
		}

		return res;
	}

	void YaDiskHTTPConnector::setToken(const QString& token)
	{
        m_token = token;
	}
}
