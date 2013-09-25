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

#include "facebook_oauth.h"
#include "common_stuff.h"
#include "facebook_view.h"
#include "../driver/facebook_driver.h"
#include "webmounter.h"

namespace Ui
{
	using namespace RemoteDriver;
	using namespace Common;

	FacebookView::FacebookView(const Data::PluginSettings* settings, const QString& title)
		: PluginView(settings, title)
	{
		_urlEdit->setText("facebook.com");
		_urlEdit->setEnabled(false);

		_oauthCheckBox->setChecked(true);
		_oauthCheckBox->setVisible(false);

		_nameLabel->setVisible(false);
		_nameEdit->setVisible(false);
		_passwordLabel->setVisible(false);
		_passwordEdit->setVisible(false);

		_oauthObj = new FacebookOAuth();
		connect(_oauthObj, SIGNAL(authFinished(RESULT, const QString&, const QString&)), this, SLOT(oAuthFinished(RESULT, const QString&, const QString&)));
	}

	bool FacebookView::isKeyValueValid(const Data::PluginSettings&)
	{
		return true;
	}

	void FacebookView::updateView(int progress, int state)
	{
		PluginView::updateView(progress, state);
		_urlEdit->setEnabled(false);

		_nameLabel->setVisible(false);
		_nameEdit->setVisible(false);
		_passwordLabel->setVisible(false);
		_passwordEdit->setVisible(false);

		PluginSettings pluginSettings; 
		Common::WebMounter::getSettingStorage()->getData(pluginSettings, "Facebook");
		if(pluginSettings.isOAuthUsing && (state == RemoteDriver::eAuthInProgress) && progress == 0) // Started to authenticate
		{
			_oauthObj->authenticate();
		}
	}

	void FacebookView::oAuthFinished(RESULT error, const QString& login, const QString& token)
	{
		if(error == eERROR_CANCEL)
		{
			emit disconnectPlugin();
		}
		else
		{
			PluginSettings pluginSettings; 
			Common::WebMounter::getSettingStorage()->getData(pluginSettings, "Facebook");
			pluginSettings.userName = login;
			pluginSettings.oAuthToken = token;

			static_cast<FacebookRVFSDriver*>(_driver)->connectHandlerStage2(error, pluginSettings);
		}
	}
}
