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

#ifndef FACEBOOK_CONNECTOR_H
#define FACEBOOK_CONNECTOR_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include "common_stuff.h"
#include "vfs_cache.h"
//#include <curl/curl.h>
#include "graphapi.h"
#include "boost/shared_ptr.hpp"

namespace Connector
{
	using namespace Data;

	class FacebookHTTPConnector : public QObject
	{
		Q_OBJECT
	public:	
		FacebookHTTPConnector();
		~FacebookHTTPConnector();

		void setSettings(
			const QString& login 
			, const QString& password 
			, const QString& proxy = ""//"proxy.te.mera.ru:8080"
			, const QString& proxyLoginPwd = ""//"login:password"
			, bool isOAuth = true
			, const QString& token = "");
		static size_t fwrite_b(void *ptr, size_t size, size_t count, void *path);
		static size_t writeStr(void *ptr, size_t size, size_t count, void *response);

		RESULT downloadFiles(QList <QString>& urlList, QList <QString>& pathList);
		RESULT getAlbums(QVariant& response);
		RESULT getPhotos(const QString& albumId, QVariant& response);
		RESULT uploadFile(const QString& path, const QString& title, const QString& parentId, QVariant& response);
		//RESULT downloadFile(const QString& url, const QString& path);
		RESULT deleteObject(const QString& id);
		RESULT createDirectory(const QString& title, QVariant& response);
		RESULT moveFile(const QString& id, const QString& oldParentId, const QString& newParentId);
		RESULT renameAlbum(const QString& id, const QString& newTitle);
		void setToken(const QString& token);
	private:
		QString m_login;
		QString m_password;
		QString m_proxy;
		QString m_proxyLoginPwd;
		bool m_isOAuth;
		QString m_token;
		QMutex m_connectorMutex;
	};
}

#endif //FACEBOOK_CONNECTOR_H