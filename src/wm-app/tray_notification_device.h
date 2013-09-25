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

#ifndef TRAYNOTIFICATIONDEVICE_H
#define TRAYNOTIFICATIONDEVICE_H

#include "notification_device.h"

#include <queue>
#include <QMutex>
#include <QSystemTrayIcon>
#include <QWaitCondition>
#include <QMessageBox>

namespace Ui
{
	using namespace std;

	class TrayNotificationDevice : public NotificationDevice				   
	{
		Q_OBJECT

	public:
		TrayNotificationDevice(QSystemTrayIcon*);
		~TrayNotificationDevice();
		//void showNotification(const Notification&);

		//public slots:	
		virtual void showNotification(const Notification&);
		//virtual void initiateDevice();
signals:
		void showMsgBox(const QString&, const QString&);
		void showTrayMsg(int, const QString&, const QString&);

	protected:
		void run();

	private:
        const unsigned int m_queueMaxLength;
        queue<Notification> m_msgQueue;
        QSystemTrayIcon* m_trayIcon;
        QMutex m_deviceMutex;
        QWaitCondition m_queueIsNotEmpty;

	};
}
#endif // TRAYNOTIFICATIONDEVICE_H
