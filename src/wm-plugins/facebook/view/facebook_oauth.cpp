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
#include "facebook_oauth.h"

namespace Ui
{
	using namespace Common;

	void WebView::closeEvent(QCloseEvent* /*event*/)
	{
		emit finished(eERROR_CANCEL);
	}


	FacebookOAuth::FacebookOAuth() : _view(NULL)
	{
		_oAuthTimer = new QTimer();
		connect(_oAuthTimer, SIGNAL(timeout()), this, SLOT(slotOAuthTimeout()));
	}

	FacebookOAuth::~FacebookOAuth()
	{
		delete _oAuthTimer;
	}

	void FacebookOAuth::initializeWebView()
	{
		_view = new WebView();
		_view->setWindowTitle(tr("Facebook | Authentication"));
		_view->page()->setForwardUnsupportedContent(true);
		QNetworkAccessManager * manager = new QNetworkAccessManager(this);

		connect(manager, SIGNAL(finished(QNetworkReply*)),this,SLOT(finished(QNetworkReply*)));
		connect(manager, SIGNAL(sslErrors(QNetworkReply *, const QList<QSslError> &)), this, SLOT(ignoreSSL( QNetworkReply *, const QList<QSslError> & )));
		connect(_view->page(), SIGNAL(unsupportedContent(QNetworkReply *)), this, SLOT(handleUnsupportedContent(QNetworkReply *)));
		connect(_view, SIGNAL(finished(RESULT)), this, SLOT(finished(RESULT)));

		_view->page()->setNetworkAccessManager(manager);
		_view->page()->triggerAction(QWebPage::Forward);

		_view->resize(600, 380);
	}

	void FacebookOAuth::finished(QNetworkReply *reply) 
	{
		int attr = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		QString url = reply->url().toString();

		if(attr == 302) // redirect
		{
			_oAuthTimer->stop();

			QString url = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
			if(url.contains("access_token="))
			{
				_token = Data::RegExp::getByPattern("access_token=(.*)&expires_in=", url);

				delete _view;
				_view = NULL;

				emit authFinished(eNO_ERROR, "", _token);
			}
		}
		else if(attr == 200 && !reply->url().toString().contains("login.php") 
			&& reply->url().toString().contains("error_description=The+user+denied+your+request."))
		{
			delete _view;
			_view = NULL;

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

	void FacebookOAuth::ignoreSSL(QNetworkReply* reply, const QList<QSslError>& /*list*/)
	{
		reply->ignoreSslErrors();
	}

	void FacebookOAuth::authenticate()
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

		_view->load(QUrl("https://graph.facebook.com/oauth/authorize?client_id=279257372195269&redirect_uri=http://www.facebook.com/connect/login_success.html&type=user_agent&display=popup&scope=publish_stream,user_photos,read_stream,email"));
		_view->show();
		if (!_oAuthTimer->isActive())
			_oAuthTimer->start(60*1000);
	}

	void FacebookOAuth::finished(RESULT error)
	{
		delete _view;
		_view = NULL;

		emit authFinished(error, "", "");
	}

	void FacebookOAuth::slotOAuthTimeout()
	{
		_oAuthTimer->stop();
		delete _view;
		_view = NULL;

		emit authFinished(eERROR_GENERAL, "", "");
	}
}
