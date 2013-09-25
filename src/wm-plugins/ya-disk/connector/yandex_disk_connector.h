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

#ifndef YANDEX_DISK_CONNECTION_H
#define YANDEX_DISK_CONNECTION_H

#include <QThread>
#include <QMutex>
//#include "common_stuff.h"
#include "vfs_cache.h"
#include <curl/curl.h>

class QWebdavUrlInfo;

namespace Connector
{
	using namespace Data;

	class YaDiskHTTPConnector : public QObject
	{
		Q_OBJECT
	public:
		YaDiskHTTPConnector();
		~YaDiskHTTPConnector();

		void setSettings(const QString& login
			, const QString& password
			, const QString& proxy
			, const QString& proxyLoginPwd
			, bool isOAuth
			, const QString& token);
		RESULT getTreeElements(const QString& path, QString& response);
		RESULT downloadFile(const QString& url, const QString& path);
		RESULT downloadFiles(const QList <QString>& urlList, const QList <QString>& pathList);
		RESULT uploadFile(const QString& path, const QString& title, const QString& parentId, QString& response);
		RESULT deleteFile(const QString& path, QString& response);
		RESULT createDirectory(const QString& title, const QString& parentId, QString& response);
		RESULT moveElement(const QString& id, const QString& oldParentId, const QString& newParentId, ElementType type, QString& response);
		RESULT renameElement(const QString& id, ElementType type, const QString& newTitle, QString& response);
		void setToken(const QString& token);
	private:
		static size_t writeStr(void *ptr, size_t size, size_t count, void *response);
		static size_t fwrite_b(void *ptr, size_t size, size_t count, void *path); 
		static size_t readStr(void *ptr, size_t size, size_t nmemb, void *stream);
		static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *stream);
		int execQuery(const QString &url, const QString &header, const QString &postFields, QString* response);
	private:
		struct sPutData
		{
			const char* m_data;
			size_t m_len;
		};
	private:
		QString m_login;
		QString m_password;
		QString m_proxy;
		QString m_proxyLoginPwd;
		bool m_isOAuth;

		QString m_token;
		QString m_requestId;
		QString m_key;
		QMutex m_connectorMutex;
	};
}

#endif // YAF_CONNECTION_H
