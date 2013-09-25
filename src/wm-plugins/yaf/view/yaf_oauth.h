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

#ifndef YAF_OAUTH_H
#define YAF_OAUTH_H

#include <QtGui>
#include <QNetworkReply>
#include <QWebView>
#include "common_stuff.h"

//QT_BEGIN_NAMESPACE
//class QWebView;
//QT_END_NAMESPACE

using namespace Common;
namespace Ui
{
	class WebView : public QWebView
	{
		Q_OBJECT
	protected:
		void closeEvent(QCloseEvent *event);
Q_SIGNALS:
		void finished(RESULT error);
	};

	class YafOAuth : public QObject
	{
		Q_OBJECT
	public:
		YafOAuth();
		~YafOAuth();
		void authenticate();
Q_SIGNALS:
		void authFinished(RESULT error, const QString& login, const QString& token);
		public slots:
			void ignoreSSL( QNetworkReply *, const QList<QSslError> & );
			void finished(QNetworkReply *reply);
			void finished(RESULT error);
			void handleUnsupportedContent(QNetworkReply *reply);
			private slots:
				void slotOAuthTimeout();
	private:
		void initializeWebView();
	private:
		QString m_token;
		WebView* m_view;
		QTimer* m_oAuthTimer;
	};
}


#endif // YAF_OAUTH_H