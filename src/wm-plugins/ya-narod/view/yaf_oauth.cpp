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


	YafOAuth::YafOAuth() : _view(NULL)
	{
		//initializeWebView();
		_oAuthTimer = new QTimer();
		connect(_oAuthTimer, SIGNAL(timeout()), this, SLOT(slotOAuthTimeout()));
	}

	YafOAuth::~YafOAuth()
	{
		delete _oAuthTimer;
	}

	void YafOAuth::initializeWebView()
	{
		_view = new WebView();
		_view->setWindowTitle(tr("Yandex | Authentication"));
		_view->page()->setForwardUnsupportedContent(true);
		QNetworkAccessManager * manager = new QNetworkAccessManager(this);

		connect(manager, SIGNAL(finished(QNetworkReply*)),this,SLOT(finished(QNetworkReply*)));
		connect(manager, SIGNAL(sslErrors(QNetworkReply *, const QList<QSslError> &)), this, SLOT(ignoreSSL( QNetworkReply *, const QList<QSslError> & )));
		connect(_view->page(), SIGNAL(unsupportedContent(QNetworkReply *)), this, SLOT(handleUnsupportedContent(QNetworkReply *)));
		connect(_view, SIGNAL(finished(RESULT)), this, SLOT(finished(RESULT)));

		_view->page()->setNetworkAccessManager(manager);
		_view->page()->triggerAction(QWebPage::Forward);

		_view->resize(900, 580);
	}

	void YafOAuth::finished(QNetworkReply *reply) 
	{
		int attr = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

		if(attr == 302) // redirect
		{
			_oAuthTimer->stop();

			QString url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
			if(url.contains("http://passport-ckicheck.yandex.ru/passport?mode=ckicheck")
				|| url.contains("https://oauth.yandex.ru/authorize?allow=True&request_id"))
			{
				_view->load(reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl());
				if (!_oAuthTimer->isActive())
					_oAuthTimer->start(10*1000);
			}
			else if(url.contains("webmounter://token#access_token="))
			{
				_token = Data::RegExp::getByPattern("webmounter://token#access_token=(.*)&state", url);
				QNetworkCookieJar *cookie = _view->page()->networkAccessManager()->cookieJar();
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

				delete _view;
				_view = NULL;

				emit authFinished(eNO_ERROR, login, _token);
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
			_token = Data::RegExp::getByPattern("webmounter://token#access_token=(.*)&state", url);
			QNetworkCookieJar *cookie = _view->page()->networkAccessManager()->cookieJar();
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

			delete _view;
			_view = NULL;

			emit authFinished(eNO_ERROR, login, _token);
		}
	}

	void YafOAuth::ignoreSSL(QNetworkReply * reply, const QList<QSslError>& /*list*/)
	{
		reply->ignoreSslErrors();
	}

	void YafOAuth::authenticate()
	{
		if(!_view)
		{
			initializeWebView();
		}

		GeneralSettings generalSettings; 
		WebMounter::getSettingStorage()->getData(generalSettings);
		if(generalSettings.proxyAddress.length())
		{
			QNetworkProxy proxy;
			proxy.setType(QNetworkProxy::HttpProxy);
			proxy.setHostName(generalSettings.proxyAddress.left(generalSettings.proxyAddress.lastIndexOf(":")));
			QString portStr = generalSettings.proxyAddress.right(generalSettings.proxyAddress.length() - generalSettings.proxyAddress.lastIndexOf(":")-1);
			proxy.setPort(portStr.toInt());
			proxy.setUser(generalSettings.proxyLogin);
			proxy.setPassword(generalSettings.proxyPassword);

			_view->page()->networkAccessManager()->setProxy(proxy);
		}

		_view->load(QUrl("https://oauth.yandex.ru/authorize?response_type=token&client_id=e56c86e3da064686bbac48edbfb00fae"));
		_view->show();
		if (!_oAuthTimer->isActive())
			_oAuthTimer->start(20*1000);
	}

	void YafOAuth::finished(RESULT error)
	{
		delete _view;
		_view = NULL;

		emit authFinished(error, "", "");
	}

	void YafOAuth::slotOAuthTimeout()
	{
		_oAuthTimer->stop();
		delete _view;
		_view = NULL;

		emit authFinished(eERROR_GENERAL, "", "");
	}
}
