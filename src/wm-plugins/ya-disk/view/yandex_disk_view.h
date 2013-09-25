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

#ifndef YANDEX_DISK_VIEW_H
#define YANDEX_DISK_VIEW_H

#include "plugin_view.h"

namespace Ui
{
	class YaDiskOAuth;

	class YaDiskView : public PluginView
	{
		Q_OBJECT
	public:
		YaDiskView(const Data::PluginSettings* settings, const QString& title);

		public slots:
			virtual void updateView(int progress, int state);
			void oAuthFinished(RESULT error, const QString& login, const QString& token);
	private:
		virtual bool isKeyValueValid(const Data::PluginSettings& settings);
	private:
		YaDiskOAuth* m_oauthObj;
	};
}

#endif // YANDEX_DISK_VIEW_H
