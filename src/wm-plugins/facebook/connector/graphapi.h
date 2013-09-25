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

#ifndef GRAPHAPI_H
#define GRAPHAPI_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "qfacebookreply.h"

namespace Connector
{
	class QFacebookReply;

	class GraphAPI : public QObject
	{
	public:
		enum HttpMethod 
		{
			eGET = 0,
			ePOST,
			ePUT,
			eDELETE
		};

		GraphAPI(QObject *parent = 0);
		void setSettings(const QString& proxy = ""//"proxy.te.mera.ru:8080"
			, const QString& proxyLoginPwd = ""//"login:password"
			, const QString& token = "");

		QFacebookReply *getObject(QString id);
		QFacebookReply *getConnections(QString id, QString connectionName);
		QFacebookReply *putObject(QString id, QString connection,
			QByteArray data = QByteArray());
		QFacebookReply *deleteObject(QString id);
		QFacebookReply *request(HttpMethod method, QString path,
			QByteArray postArgs = QByteArray());

		QNetworkReply *get(QUrl url);
		QNetworkReply *put(QUrl url, QByteArray data);
		QNetworkReply *del(QUrl url);
		QNetworkReply *request(HttpMethod method, QUrl url,
			QByteArray postArgs = QByteArray());

	private:
		QString m_accessToken;
		QNetworkAccessManager *m_networkManager;
	};
}

#endif //GRAPHAPI_H
