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

#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>
#include <QNetworkProxy>

#include "graphapi.h"

namespace Connector
{
	GraphAPI::GraphAPI(QObject *parent)
		: QObject(parent),
		m_networkManager( new QNetworkAccessManager(this))
	{
	}

	void GraphAPI::setSettings(const QString& proxy
		, const QString& proxyLoginPwd
		, const QString& token)
	{
		if(proxy.length() > 1)
		{
			QNetworkProxy nwProxy;
			nwProxy.setType(QNetworkProxy::HttpProxy);
			nwProxy.setHostName(proxy.left(proxy.lastIndexOf(":")));
			QString portStr = proxy.right(proxy.length() - proxy.lastIndexOf(":")-1);
			nwProxy.setPort(portStr.toInt());
			if(proxyLoginPwd.length() > 1)
			{
				nwProxy.setUser(proxyLoginPwd.left(proxyLoginPwd.lastIndexOf(":")));
				nwProxy.setPassword(proxyLoginPwd.right(proxyLoginPwd.length() - proxyLoginPwd.lastIndexOf(":")-1));
			}

			m_networkManager->setProxy(nwProxy);
		}

		m_accessToken = token;
	}

	QFacebookReply *GraphAPI::getObject(QString id)
	{
		return request(GraphAPI::eGET, id);
	}

	QFacebookReply *GraphAPI::getConnections(QString id, QString connectionName)
	{
		return request(GraphAPI::eGET, id + "/" + connectionName);
	}

	QFacebookReply *GraphAPI::putObject(QString id, QString connection, QByteArray data)
	{
		// You need extended permissions to do it.
		// http://developers.facebook.com/docs/authentication/
		return request(GraphAPI::ePOST, id + "/" + connection, data);
	}

	QFacebookReply *GraphAPI::deleteObject(QString id)
	{
		return request(GraphAPI::eDELETE, id);
	}

	QFacebookReply *GraphAPI::request(HttpMethod method, QString path, QByteArray postArgs)
	{
		QUrl url("https://graph.facebook.com");
		url.setPath(path);

		switch (method) {
		case GraphAPI::eGET:
		case GraphAPI::eDELETE:
			url.addQueryItem("access_token", m_accessToken);
			break;
		case GraphAPI::ePOST:
			postArgs.append("&access_token=" + m_accessToken);
			break;
		default:
			qWarning() << "Request method is not supported.";
			return 0;
		}

		return new QFacebookReply(request(method, url, postArgs), this);
	}

	QNetworkReply *GraphAPI::get(QUrl url)
	{
		return request(GraphAPI::eGET, url);
	}

	QNetworkReply *GraphAPI::put(QUrl url, QByteArray data)
	{
		return request(GraphAPI::ePOST, url, data);
	}

	QNetworkReply *GraphAPI::del(QUrl url)
	{
		return request(GraphAPI::eDELETE, url);
	}

	QNetworkReply *GraphAPI::request(HttpMethod method, QUrl url, QByteArray postArgs)
	{
		QNetworkRequest request;
		QNetworkReply *reply(0);
		request.setUrl(url);

		switch (method) {
		case GraphAPI::eGET:
			reply = m_networkManager->get(request);
			break;
		case GraphAPI::ePOST:
			reply = m_networkManager->post(request, postArgs);
			break;
		case GraphAPI::eDELETE:
			reply = m_networkManager->deleteResource(request);
			break;
		default:
			qWarning() << "Request method is not supported.";
			return 0;
		}

		return reply;
	}
}
