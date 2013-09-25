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

#ifndef VK_CONNECTOR_H
#define VK_CONNECTOR_H

#include <QThread>
#include <QMutex>
#include "common_stuff.h"
#include "vfs_cache.h"
#include <curl/curl.h>

namespace Connector
{
	using namespace Data;

	class VkHTTPConnector
	{
	public:	
		VkHTTPConnector();
		~VkHTTPConnector();

		void setSettings(
			const QString& login 
			, const QString& password 
			, const QString& proxy = ""//"proxy.te.mera.ru:8080"
			, const QString& proxyLoginPwd = ""//"login:password"
			, bool isOAuth = true
			, const QString& token = "");
		RESULT downloadFiles(QList <QString>& urlList, QList <QString>& pathList);
		RESULT getAlbums(QString& response, int& errorCode);
		RESULT getPhotos(int offset, QString& response);
		RESULT uploadFile(const QString& path, const QString& title, const QString& parentId, QString& response);
		RESULT downloadFile(const QString& url, const QString& path);
		RESULT deleteFile(const QString& id);
		RESULT deleteAlbum(const QString& id);
		RESULT createDirectory(const QString& title, QString& response);
		RESULT moveFile(const QString& id, const QString& oldParentId, const QString& newParentId);
		RESULT renameAlbum(const QString& id, const QString& newTitle);
		void setToken(const QString& token);
	private:
		QString genQuery(const QString &method, const QStringList &params);
		RESULT execQuery(const QString &url, const QString &header, const QString &postFields, QString* response);

		QString getUploadServer(const QString& albumId);
		RESULT uploadPhoto(const QString& uploadServer, const QString& path);
		RESULT savePhoto(const QString& parentId, QString& response);
	private:
		static size_t writeStr(void *ptr, size_t size, size_t count, void *response);
		static size_t fwrite_b(void *ptr, size_t size, size_t count, void *path); 
	private:
		struct sVkSession
		{
			QString m_expire;     
			QString m_mid;   		
			QString m_secret;   
			QString m_sid;
		};

		struct sUploadData
		{
			QString m_server;     
			QString m_photos_list;   		
			QString m_aid;   
			QString m_hash;
		};
	private:
		QString m_login;
		QString m_password;
		QString m_proxy;
		QString m_proxyLoginPwd;
		bool m_isOAuth;
		QString m_token;
		QMutex m_connectorMutex;

		QString m_id;
		static QString m_appId;
		QString m_hash;
		QString m_settings_hash;
		QString m_s;
		QString m_p;
		sVkSession m_session;
		sUploadData m_upload;
		long m_responseCode;
	};
}

#endif //VK_CONNECTOR_H
