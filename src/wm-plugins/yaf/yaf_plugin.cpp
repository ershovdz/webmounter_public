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

#include "yaf_plugin.h"
#include "./driver/yaf_driver.h"
#include "./view/yaf_view.h"
#include "data.h"
#include "webmounter.h"

YafPlugin::YafPlugin() : _driver(NULL), _view(NULL), _icon(NULL)
{
}

YafPlugin::~YafPlugin()
{
	delete _driver;
}

QString YafPlugin::name()
{
	QString name("Yandex.Fotki");
	return name;
}

void* YafPlugin::getRVFSDriver()
{
	if(!_driver)
		_driver = new RemoteDriver::YafRVFSDriver(name());
	return (void*)(_driver);
}

void* YafPlugin::getView()
{
	if(!_view)
	{
		Data::PluginSettings settings;
        settings.m_pluginName = name();
		Common::WebMounter::getSettingStorage()->getData(settings, name());

		_view = new Ui::YafView(&settings, name());
	}

	return (void*)(_view);
}

void* YafPlugin::getSettings()
{
	return NULL;
}

QIcon* YafPlugin::getIcon()
{
	if(!_icon)
	{
		_icon = new QIcon(":/icons/yaf.png");
	}

	return _icon;
}

QString YafPlugin::getTranslationFile(const QString& locale)
{
	QString file("yaf_wm_pl_" + locale);
	return file;
}

Q_EXPORT_PLUGIN2(wm-yandex-plugin, YafPlugin);
