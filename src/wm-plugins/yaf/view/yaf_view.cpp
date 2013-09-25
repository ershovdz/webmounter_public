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
#include "yaf_view.h"
#include "../driver/yaf_driver.h"
#include "webmounter.h"

namespace Ui
{
	using namespace RemoteDriver;

	YafView::YafView(const Data::PluginSettings* settings, const QString& title)
		: PluginView(settings, title)
	{
        m_urlEdit->setText("fotki.yandex.ru");
        m_urlEdit->setEnabled(false);

        m_oauthCheckBox->setChecked(true);
        m_oauthCheckBox->setVisible(false);

        m_nameLabel->setVisible(false);
        m_nameEdit->setVisible(false);
        m_passwordLabel->setVisible(false);
        m_passwordEdit->setVisible(false);

        m_oauthObj = new YafOAuth();
        connect(m_oauthObj, SIGNAL(authFinished(RESULT, const QString&, const QString&)), this, SLOT(oAuthFinished(RESULT, const QString&, const QString&)));
	}

	bool YafView::isKeyValueValid(const Data::PluginSettings&)
	{
		return true;
	}

	void YafView::updateView(int progress, int state)
	{
		PluginView::updateView(progress, state);
        m_urlEdit->setEnabled(false);

        m_nameLabel->setVisible(false);
        m_nameEdit->setVisible(false);
        m_passwordLabel->setVisible(false);
        m_passwordEdit->setVisible(false);

		PluginSettings pluginSettings; 
		Common::WebMounter::getSettingStorage()->getData(pluginSettings, "Yandex.Fotki");
        if(pluginSettings.m_isOAuthUsing && (state == RemoteDriver::eAuthInProgress) && progress == 0) // Started to authenticate
		{
            m_oauthObj->authenticate();
		}
	}

	void YafView::oAuthFinished(RESULT error, const QString& login, const QString& token)
	{
		if(error == eERROR_CANCEL)
		{
			emit disconnectPlugin();
		}
		else
		{
			PluginSettings pluginSettings; 
			Common::WebMounter::getSettingStorage()->getData(pluginSettings, "Yandex.Fotki");
            pluginSettings.m_prevUserName = pluginSettings.m_userName;
            pluginSettings.m_userName = login;
            pluginSettings.m_oAuthToken = token;

            static_cast<YafRVFSDriver*>(m_driver)->connectHandlerStage2(error, pluginSettings);
		}
	}
}
