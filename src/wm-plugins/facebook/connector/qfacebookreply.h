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

#ifndef QFACEBOOKREPLY_H
#define QFACEBOOKREPLY_H

//#include "qfacebook_global.h"
#include "graphapi.h"

#include <QObject>
#include <QNetworkReply>
#include <QSslError>

namespace Connector
{
	class QFacebookReply : public QObject
	{
		Q_OBJECT
	protected:
		explicit QFacebookReply(QNetworkReply *reply, QObject *parent = 0);

	public:
		QVariant data();
		QNetworkReply::NetworkError error() const;

signals:
		void finished();
		void uploadProgress(qint64 bytesSent, qint64 bytesTotal);

		private slots:
			void onParseData();
			void doParse();
			void onError(QNetworkReply::NetworkError code);
			void onSslErrors(const QList<QSslError> &errors);

	private:
		QNetworkReply *m_reply;
		QVariant m_data;

		friend class GraphAPI;
	};
}

#endif // QFACEBOOKREPLY_H
