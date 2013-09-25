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

#ifndef NOTIFICATIONDEVICE_H
#define NOTIFICATIONDEVICE_H

#include <QString>
#include <QThread>

namespace Ui
{
	class Notification
	{
	public:
		enum _Types 
		{
			eINFO
			, eWARNING
			, eERROR
			, eCRITICAL
			, eINTERACTIVE
		} Type;

		Notification()
		{
		}

		Notification(_Types type, const QString& title, const QString& description)
		{
			Type = type;
			Title = title;
			Description = description; 
		}

		Notification& operator=(const Notification& msg)
		{
			Type = msg.Type;
			Title = msg.Title;
			Description = msg.Description;

			return *this;
		}

		void init(_Types type, const QString& title, const QString& description)
		{
			Type = type;
			Title = title;
			Description = description; 
		}

		QString Title;
		QString Description;
	};

	class NotificationDevice : public QThread
	{
	public:
		virtual void showNotification(const Notification&) = 0;
	};
}
#endif // NOTIFICATIONDEVICE_H
