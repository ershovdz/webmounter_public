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

#include "yanarod_plugin.h"
#include "./driver/yandex_narod_driver.h"
#include "./view/yandex_narod_view.h"
#include "data.h"
#include "webmounter.h"

YaNarodPlugin::YaNarodPlugin() : m_driver(NULL), m_view(NULL), m_icon(NULL)
{
}

YaNarodPlugin::~YaNarodPlugin()
{
    delete m_driver;
}

QString YaNarodPlugin::name()
{
	QString name("Yandex.Narod");
	return name;
}

void* YaNarodPlugin::getRVFSDriver()
{
    if(!m_driver)
        m_driver = new RemoteDriver::YandexNarodRVFSDriver(name());
    return (void*)(m_driver);
}

void* YaNarodPlugin::getView()
{
    if(!m_view)
	{
		Data::PluginSettings settings;
        settings.m_pluginName = name();
		Common::WebMounter::getSettingStorage()->getData(settings, name());

        m_view = new Ui::YandexNarodView(&settings, name());
	}

    return (void*)(m_view);
}

void* YaNarodPlugin::getSettings()
{
	return NULL;
}

QIcon* YaNarodPlugin::getIcon()
{
    if(!m_icon)
	{
        m_icon = new QIcon(":/icons/yandexnarod.png");
	}

    return m_icon;
}

QString YaNarodPlugin::getTranslationFile(const QString& locale)
{
	QString file("yanarod_wm_pl_" + locale);
	return file;
}

Q_EXPORT_PLUGIN2(wm-ya-narod-plugin, YaNarodPlugin);
