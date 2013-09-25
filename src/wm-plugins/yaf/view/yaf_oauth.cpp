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
#include "webmounter.h"
#include "reg_exp.h"
#include "yaf_oauth.h"

namespace Ui
{
	using namespace Common;

	void WebView::closeEvent(QCloseEvent* /*event*/)
	{
		emit finished(eERROR_CANCEL);
	}


    YafOAuth::YafOAuth() : m_view(NULL)
	{
		//initializeWebView();
        m_oAuthTimer = new QTimer();
        connect(m_oAuthTimer, SIGNAL(timeout()), this, SLOT(slotOAuthTimeout()));
	}

	YafOAuth::~YafOAuth()
	{
        delete m_oAuthTimer;
	}

	void YafOAuth::initializeWebView()
	{
        m_view = new WebView();
        m_view->setWindowTitle(tr("Yandex | Authentication"));
        m_view->page()->setForwardUnsupportedContent(true);
		QNetworkAccessManager * manager = new QNetworkAccessManager(this);

		connect(manager, SIGNAL(finished(QNetworkReply*)),this,SLOT(finished(QNetworkReply*)));
		connect(manager, SIGNAL(sslErrors(QNetworkReply *, const QList<QSslError> &)), this, SLOT(ignoreSSL( QNetworkReply *, const QList<QSslError> & )));
        connect(m_view->page(), SIGNAL(unsupportedContent(QNetworkReply *)), this, SLOT(handleUnsupportedContent(QNetworkReply *)));
        connect(m_view, SIGNAL(finished(RESULT)), this, SLOT(finished(RESULT)));

        m_view->page()->setNetworkAccessManager(manager);
        m_view->page()->triggerAction(QWebPage::Forward);

        m_view->resize(900, 580);
	}

	void YafOAuth::finished(QNetworkReply *reply) 
	{
		int attr = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

		if(attr == 302) // redirect
		{
            m_oAuthTimer->stop();

			QString url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
			if(url.contains("http://passport-ckicheck.yandex.ru/passport?mode=ckicheck")
				|| url.contains("https://oauth.yandex.ru/authorize?allow=True&request_id"))
			{
                m_view->load(reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl());
                if (!m_oAuthTimer->isActive())
                    m_oAuthTimer->start(10*1000);
			}
			else if(url.contains("webmounter://token#access_token="))
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
						login = login.replace(".", "-");
						break;
					}
				}

                delete m_view;
                m_view = NULL;

                emit authFinished(eNO_ERROR, login, m_token);
			}
			else if(url.contains("error=access_denied"))
			{
                delete m_view;
                m_view = NULL;

				emit authFinished(eERROR_CANCEL, "", "");
			}
		}
	}

	void YafOAuth::handleUnsupportedContent(QNetworkReply *reply)
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

	void YafOAuth::ignoreSSL(QNetworkReply* reply, const QList<QSslError>& /*list*/)
	{
		reply->ignoreSslErrors();
	}

	void YafOAuth::authenticate()
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

        m_view->load(QUrl("https://oauth.yandex.ru/authorize?response_type=token&client_id=e56c86e3da064686bbac48edbfb00fae"));
        m_view->show();
		//if (!_oAuthTimer->isActive())
		//	_oAuthTimer->start(20*1000);
	}

	void YafOAuth::finished(RESULT error)
	{
        delete m_view;
        m_view = NULL;

		emit authFinished(error, "", "");
	}

	void YafOAuth::slotOAuthTimeout()
	{
        m_oAuthTimer->stop();
        delete m_view;
        m_view = NULL;

		emit authFinished(eERROR_CANCEL, "", "");
	}
}
