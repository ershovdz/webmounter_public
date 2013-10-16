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

#include <QtGui>
#include <QtWebKit>
#include <QNetworkProxy>
#include "webmounter.h"
#include "reg_exp.h"
#include "facebook_oauth.h"

namespace Ui
{
	using namespace Common;

	void WebView::closeEvent(QCloseEvent* /*event*/)
	{
		emit finished(eERROR_CANCEL);
	}


        FacebookOAuth::FacebookOAuth() : m_view(NULL)
	{
                m_oAuthTimer = new QTimer();
                connect(m_oAuthTimer, SIGNAL(timeout()), this, SLOT(slotOAuthTimeout()));
	}

	FacebookOAuth::~FacebookOAuth()
	{
                delete m_oAuthTimer;
	}

	void FacebookOAuth::initializeWebView()
	{
                m_view = new WebView();
                m_view->setWindowTitle(tr("Facebook | Authentication"));
                m_view->page()->setForwardUnsupportedContent(true);
		QNetworkAccessManager * manager = new QNetworkAccessManager(this);

		connect(manager, SIGNAL(finished(QNetworkReply*)),this,SLOT(finished(QNetworkReply*)));
		connect(manager, SIGNAL(sslErrors(QNetworkReply *, const QList<QSslError> &)), this, SLOT(ignoreSSL( QNetworkReply *, const QList<QSslError> & )));
                connect(m_view->page(), SIGNAL(unsupportedContent(QNetworkReply *)), this, SLOT(handleUnsupportedContent(QNetworkReply *)));
                connect(m_view, SIGNAL(finished(RESULT)), this, SLOT(finished(RESULT)));

                m_view->page()->setNetworkAccessManager(manager);
                m_view->page()->triggerAction(QWebPage::Forward);

                m_view->resize(600, 380);
	}

	void FacebookOAuth::finished(QNetworkReply *reply) 
	{
		int attr = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		QString url = reply->url().toString();

		if(attr == 302) // redirect
		{
                        m_oAuthTimer->stop();

			QString url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
			if(url.contains("access_token="))
			{
                                m_token = Data::RegExp::getByPattern("access_token=(.*)&expires_in=", url);

                                delete m_view;
                                m_view = NULL;

                                emit authFinished(eNO_ERROR, "", m_token);
			}
		}
		else if(attr == 200 && !reply->url().toString().contains("login.php") 
			&& reply->url().toString().contains("error_description=The+user+denied+your+request."))
		{
                        delete m_view;
                        m_view = NULL;

			emit authFinished(eERROR_CANCEL, "", "");
		}
	}

	void FacebookOAuth::handleUnsupportedContent(QNetworkReply *reply)
	{
		if (reply->error() == QNetworkReply::NoError) {
			return;
		}

		QString url = reply->url().toString();
		if(url.contains("webmounter://token#access_token="))
		{
                        m_token = Data::RegExp::getByPattern("webmounter://token#access_token=(.*)&state", url);
                        QNetworkCookieJar *cookie = m_view->page()->networkAccessManager()->cookieJar();
			QUrl yafUrl ("https://oauth.yandex.ru");
			QString login;
			for (int i=0; i < cookie->cookiesForUrl(yafUrl).count(); i++)
			{
				if (cookie->cookiesForUrl(yafUrl).at(i).name() == "yandex_login")
				{
					login = cookie->cookiesForUrl(yafUrl).at(i).value();
					break;
				}
			}

                        delete m_view;
                        m_view = NULL;

                        emit authFinished(eNO_ERROR, login, m_token);
		}
	}

	void FacebookOAuth::ignoreSSL(QNetworkReply* reply, const QList<QSslError>& /*list*/)
	{
		reply->ignoreSslErrors();
	}

	void FacebookOAuth::authenticate()
	{
                if(!m_view)
		{
			initializeWebView();
		}

		GeneralSettings generalSettings; 
		WebMounter::getSettingStorage()->getData(generalSettings);
                if(generalSettings.m_proxyAddress.length())
		{
			QNetworkProxy proxy;
			proxy.setType(QNetworkProxy::HttpProxy);
                        proxy.setHostName(generalSettings.m_proxyAddress.left(generalSettings.m_proxyAddress.lastIndexOf(":")));
                        QString portStr = generalSettings.m_proxyAddress.right(generalSettings.m_proxyAddress.length() - generalSettings.m_proxyAddress.lastIndexOf(":")-1);
			proxy.setPort(portStr.toInt());
                        proxy.setUser(generalSettings.m_proxyLogin);
                        proxy.setPassword(generalSettings.m_proxyPassword);

                        m_view->page()->networkAccessManager()->setProxy(proxy);
		}

                m_view->load(QUrl("https://graph.facebook.com/oauth/authorize?client_id=279257372195269&redirect_uri=http://www.facebook.com/connect/login_success.html&type=user_agent&display=popup&scope=publish_stream,user_photos,read_stream,email"));
                m_view->show();
                if (!m_oAuthTimer->isActive())
                        m_oAuthTimer->start(60*1000);
	}

	void FacebookOAuth::finished(RESULT error)
	{
                delete m_view;
                m_view = NULL;

		emit authFinished(error, "", "");
	}

	void FacebookOAuth::slotOAuthTimeout()
	{
                m_oAuthTimer->stop();
                delete m_view;
                m_view = NULL;

		emit authFinished(eERROR_GENERAL, "", "");
	}
}
