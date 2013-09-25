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
#include <qregexp.h>
#include <QStringList>
#include <QCryptographicHash>
#include <QFile>
#include "vk_connector.h"
#include "reg_exp.h"

namespace Connector
{
	using namespace Common;

	// VkHTTPConnector
	QString VkHTTPConnector::m_appId = "2950346";

	RESULT VkHTTPConnector::uploadFile(const QString& path, const QString& /*title*/, const QString& parentId, QString& response)
	{
		RESULT res = eNO_ERROR;
		QString uploadUrl = getUploadServer(parentId);
		res = uploadPhoto(uploadUrl, path);
		if(res == eNO_ERROR)
		{
			res = savePhoto(parentId, response);
		}

		return res;
	}

	QString VkHTTPConnector::getUploadServer(const QString& albumId)
	{
		QString res;
		QStringList params;
		params.append("aid="+ albumId);
		QString query;
		query = genQuery("photos.getUploadServer",params);
		QString response;
		int err = execQuery("https://api.vk.com/method/photos.getUploadServer.xml", "", query, &response);

		if(!err)
		{
			res = RegExp::getByPattern("<upload_url>(.*)</upload_url>", response);
			res.replace("&amp;", "&");
		}

		return res;
	}

	RESULT VkHTTPConnector::uploadPhoto(const QString& uploadServer, const QString& path)
	{
		CURL* p_curl = curl_easy_init();
		if(p_curl)
		{
			struct curl_httppost *post = NULL;
			struct curl_httppost *last = NULL;

			QString response;

			if((m_proxy != "") && (m_proxy != ":"))
			{
				curl_easy_setopt(p_curl, CURLOPT_PROXY, qPrintable(m_proxy));
				curl_easy_setopt(p_curl, CURLOPT_PROXYUSERPWD, qPrintable(m_proxyLoginPwd));
			}

			curl_easy_setopt(p_curl, CURLOPT_URL, qPrintable(uploadServer));
			curl_easy_setopt(p_curl, CURLOPT_VERBOSE, 1L);

			curl_formadd(&post, &last, CURLFORM_COPYNAME, "file1", CURLFORM_FILE, qPrintable(path), CURLFORM_END);

			curl_easy_setopt(p_curl, CURLOPT_HTTPPOST, post);

			struct curl_slist *chunk = NULL;
            QString header = QString("Cookie: remixchk=5; remixsid=%1").arg(m_s);

			chunk = curl_slist_append(chunk, qPrintable(header));

			curl_easy_setopt(p_curl, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt(p_curl, CURLOPT_HTTPHEADER, chunk); 

			curl_easy_setopt(p_curl, CURLOPT_WRITEHEADER, writeStr);
			curl_easy_setopt(p_curl, CURLOPT_HEADERDATA, &response);
			curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, writeStr);
			curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &response);

			CURLcode _error = curl_easy_perform(p_curl);
			curl_formfree(post);
			curl_easy_cleanup(p_curl);

			if(!_error)
			{
                m_upload.m_photos_list = RegExp::getByPattern("\"photos_list\"(.*)\"aid\"", response);

                if( ":\"[]\"," == m_upload.m_photos_list )
				{
					return eERROR_GENERAL;
				} 
				else
				{
                    m_upload.m_photos_list = m_upload.m_photos_list.mid(2, m_upload.m_photos_list.length() - 4);
                    m_upload.m_photos_list.remove("\\");
                    m_upload.m_server = RegExp::getByPattern("\"server\"(.*)\"photos_list\"", response);
                    m_upload.m_server = m_upload.m_server.mid(1, m_upload.m_server.length()-2);
                    m_upload.m_hash = response.mid(response.indexOf("hash") + 7, response.lastIndexOf("}")-response.indexOf("hash") - 8);
					return eNO_ERROR;
				}
			}
			else
			{
				return eERROR_GENERAL;
			}
		}
		return eERROR_GENERAL;
	}

	RESULT VkHTTPConnector::savePhoto(const QString& parentId, QString& response)
	{
		RESULT res = eERROR_GENERAL;
		QStringList params;
		params.append("aid=" + parentId);

        QString server = "server="+m_upload.m_server;
		params.append(server);

        QString hash = "hash="+m_upload.m_hash;
		params.append(hash);

        QString photos_list = "photos_list="+m_upload.m_photos_list;
		params.append(photos_list);

		QString query;
		query = genQuery("photos.save",params);

		res = execQuery("https://api.vk.com/method/photos.save.xml", "", query, &response);

		return res;
	}

	size_t VkHTTPConnector::writeStr(void *ptr, size_t size, size_t count, void *response)
	{
		if(response)
		{
			((QString*)response)->append(QString::fromUtf8((char*)ptr, count));
			return size*count;
		}
		return -1;
	}

	size_t VkHTTPConnector::fwrite_b(void *ptr, size_t size, size_t count, void *path) 
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

	VkHTTPConnector::VkHTTPConnector()
	{
	}

	VkHTTPConnector::~VkHTTPConnector()
	{
	}

	void VkHTTPConnector::setSettings(const QString& login
		, const QString& password
		, const QString& proxy
		, const QString& proxyLoginPwd
		, bool isOAuth
		, const QString& token)
	{
        m_login = login;
        m_password = password;
        m_proxy = proxy;
        m_proxyLoginPwd = proxyLoginPwd;
        m_isOAuth = isOAuth;
        m_token = token;
	}

	QString VkHTTPConnector::genQuery(const QString& /*method*/, const QStringList &params)
	{
		QStringList paramsList;

        paramsList.append("access_token=" + m_token);
		if (!params.isEmpty()) 
		{
			paramsList << params;
		}

		paramsList.replaceInStrings("&","%26");
		paramsList.replaceInStrings("+","%2B");

		return paramsList.join("&");
	}

	RESULT VkHTTPConnector::execQuery(const QString &url, const QString &header, const QString &postFields, QString* response)
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

			curl_easy_setopt(p_curl, CURLOPT_URL, qPrintable(url));
			curl_easy_setopt(p_curl, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(p_curl, CURLOPT_SSL_VERIFYHOST, 0L);

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
			}

			curl_easy_setopt(p_curl, CURLOPT_WRITEHEADER, writeStr);
			curl_easy_setopt(p_curl, CURLOPT_HEADERDATA, response);
			curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, writeStr);
			curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, response);
			curl_easy_setopt(p_curl, CURLOPT_FOLLOWLOCATION, 1L);

			CURLcode _error = curl_easy_perform(p_curl);
            curl_easy_getinfo(p_curl, CURLINFO_RESPONSE_CODE, &m_responseCode);
			res = (_error == CURLE_OK) ? eNO_ERROR: eERROR_GENERAL;
			curl_easy_cleanup(p_curl);
		}

		return res;
	}

	RESULT VkHTTPConnector::getAlbums(QString& response, int& errorCode)
	{
		RESULT res = eERROR_GENERAL;
		QStringList params;
		QString query = genQuery("photos.getAlbums",params);

		res = execQuery("https://api.vk.com/method/photos.getAlbums.xml", "", query, &response);
		if(response.indexOf("error") != -1)
		{
			res = eERROR_GENERAL;
			QString errorStr = RegExp::getByPattern("<error_code>(.*)</error_code>", response);
			errorCode = errorStr.length() ? errorStr.toInt() : 0;
		}

		return res;
	}

	RESULT VkHTTPConnector::getPhotos(int offset, QString& response)
	{
		RESULT res = eERROR_GENERAL;
		QStringList params;
		params.append("offset=" + QString::number(offset));
		params.append("count=100");
		QString query = genQuery("photos.getAll",params);

		res = execQuery("https://api.vk.com/method/photos.getAll.xml", "", query, &response);

		return res;
	}

	RESULT VkHTTPConnector::downloadFile(const QString& url, const QString& path)
	{
		RESULT res = eERROR_GENERAL;
        QMutexLocker locker(&m_connectorMutex);

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

			CURLcode err = curl_easy_perform(p_curl);
			res = (err == CURLE_OK) ? eNO_ERROR: eERROR_GENERAL;
			curl_easy_cleanup(p_curl);
		}

		return res;
	}

	RESULT VkHTTPConnector::downloadFiles(QList <QString>& urlList, QList <QString>& pathList)
	{
		RESULT res = eNO_ERROR;
        QMutexLocker locker(&m_connectorMutex);

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

				curl_easy_setopt(p_curl, CURLOPT_URL, qPrintable(urlList.at(i)));

				curl_easy_setopt(p_curl, CURLOPT_VERBOSE, 1L);
				curl_easy_setopt(p_curl, CURLOPT_WRITEFUNCTION, fwrite_b);
				curl_easy_setopt(p_curl, CURLOPT_WRITEDATA, &pathList.at(i));
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

	RESULT VkHTTPConnector::deleteFile(const QString& id)
	{
		RESULT res = eERROR_GENERAL; 
		QStringList params;
		params.append("pid=" + id);
		QString query = genQuery("photos.delete",params);
		QString response;
		res = execQuery("https://api.vk.com/method/photos.delete.xml", "", query, &response);
		QString errorStr = RegExp::getByPattern("<response>(.*)</response>", response);
		if(errorStr.length() != 0)
		{
			res = errorStr.toInt() == 1 ? eNO_ERROR : eERROR_GENERAL;
		}
		return res;
	}

	RESULT VkHTTPConnector::createDirectory(const QString& title, QString& response)
	{
		RESULT res = eERROR_GENERAL;
		QStringList params;
		params.append("title="+title);
		QString query = genQuery("photos.createAlbum",params);
		res = execQuery("https://api.vk.com/method/photos.createAlbum.xml", "", query, &response);

		return res;
	}

	RESULT VkHTTPConnector::deleteAlbum(const QString& id)
	{
		RESULT res = eERROR_GENERAL;
		QStringList params;
		params.append("aid=" + id);
		QString query = genQuery("photos.deleteAlbum",params);
		QString response;
		res = execQuery("https://api.vk.com/method/photos.deleteAlbum.xml", "", query, &response);
		QString errorStr = RegExp::getByPattern("<response>(.*)</response>", response);
		if(errorStr.length() != 0)
		{
			res = errorStr.toInt() == 1 ? eNO_ERROR : eERROR_GENERAL;
		}

		return res;
	}

	RESULT VkHTTPConnector::moveFile(const QString& id, const QString& /*oldParentId*/, const QString& newParentId)
	{
		RESULT res = eERROR_GENERAL;
		QStringList params;
		params.append("pid=" + id);
		params.append("target_aid=" + newParentId);
		QString query = genQuery("photos.move",params);

		QString response;
		res = execQuery("https://api.vk.com/method/photos.move.xml", "", query, &response);
		QString errorStr = RegExp::getByPattern("<response list=\"true\">(.*)</response>", response);
		if(errorStr.length() != 0)
		{
			res = errorStr.toInt() == 1 ? eNO_ERROR : eERROR_GENERAL;
		}

		return res;
	}

	RESULT VkHTTPConnector::renameAlbum(const QString& id, const QString& newTitle)
	{
		RESULT res = eERROR_GENERAL;
		QStringList params;
		params.append("aid=" + id);
		params.append("title=" + newTitle);
		QString query = genQuery("photos.editAlbum",params);

		QString response;
		res = execQuery("https://api.vk.com/method/photos.editAlbum.xml", "", query, &response);
		QString errorStr = RegExp::getByPattern("<response list=\"true\">(.*)</response>", response);
		if(errorStr.length() != 0)
		{
			res = errorStr.toInt() == 1 ? eNO_ERROR : eERROR_GENERAL;
		}

		return res;
	}
}
