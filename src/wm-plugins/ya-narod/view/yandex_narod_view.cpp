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

#include "yaf_oauth.h"
#include "yandex_narod_view.h"
#include "../driver/yandex_narod_driver.h"
#include "webmounter.h"

namespace Ui
{
	using namespace RemoteDriver;

	YandexNarodView::YandexNarodView(const Data::PluginSettings* settings, const QString& title)
		: PluginView(settings, title)
	{
        m_urlEdit->setText("narod.yandex.ru");
        m_urlEdit->setEnabled(false);

        m_authGroup = new QGroupBox(tr("Authorization"));
        m_authLayout = new QGridLayout;
		//_authLayout->addWidget(_urlLabel, 0, 0);
		//_authLayout->addWidget(_urlEdit, 0, 1);
        m_authLayout->addWidget(m_nameLabel, 1, 0);
        m_authLayout->addWidget(m_nameEdit, 1, 1);
        m_authLayout->addWidget(m_passwordLabel, 2, 0);
        m_authLayout->addWidget(m_passwordEdit, 2, 1);
        m_authGroup->setLayout(m_authLayout);

        m_mainLayout->insertWidget(0, m_authGroup);
	}

	void YandexNarodView::updateView(int progress, int state)
	{
		PluginView::updateView(progress, state);
        m_urlEdit->setEnabled(false);
	}
}
