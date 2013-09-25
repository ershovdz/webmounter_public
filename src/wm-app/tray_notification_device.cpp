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

#include "tray_notification_device.h"

#include <QMessageBox>

using namespace Ui;

TrayNotificationDevice::TrayNotificationDevice(QSystemTrayIcon* ptrTrayIcon) : m_queueMaxLength(5)
{
    m_trayIcon = ptrTrayIcon;
}

TrayNotificationDevice::~TrayNotificationDevice()
{
    delete m_trayIcon;
}

void TrayNotificationDevice::showNotification(const Notification& msg)
{
    QMutexLocker locker(&m_deviceMutex);

    if(m_msgQueue.size() < m_queueMaxLength)
	{
        m_msgQueue.push(msg);
	}

	if(!isRunning())
	{
		start();
	}

    m_queueIsNotEmpty.wakeAll();
}

void TrayNotificationDevice::run()
{
	Notification msg;
	QMutex mutex;
	forever
	{
		{
			QMutexLocker locker(&mutex);

            if(!m_msgQueue.empty())
			{
                m_deviceMutex.lock();
                msg = m_msgQueue.front();

				showTrayMsg(msg.Type, msg.Title, msg.Description);

                m_msgQueue.pop();
                m_deviceMutex.unlock();
			}

            m_queueIsNotEmpty.wait(&mutex, 3000);
		}
	}
}