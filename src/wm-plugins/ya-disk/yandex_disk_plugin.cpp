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

#include "yandex_disk_plugin.h"
#include "./driver/yandex_disk_driver.h"
#include "./view/yandex_disk_view.h"
#include "data.h"
#include "webmounter.h"

YaDiskPlugin::YaDiskPlugin() : m_driver(NULL), m_view(NULL), m_icon(NULL)
{
}

YaDiskPlugin::~YaDiskPlugin()
{
    delete m_driver;
}

QString YaDiskPlugin::name()
{
	QString name("Yandex.Disk");
	return name;
}

void* YaDiskPlugin::getRVFSDriver()
{
    if(!m_driver)
        m_driver = new RemoteDriver::YaDiskRVFSDriver(name());
    return (void*)(m_driver);
}

void* YaDiskPlugin::getView()
{
    if(!m_view)
	{
		Data::PluginSettings settings;
        settings.m_pluginName = name();
		Common::WebMounter::getSettingStorage()->getData(settings, name());

        m_view = new Ui::YaDiskView(&settings, name());
	}

    return (void*)(m_view);
}

void* YaDiskPlugin::getSettings()
{
	return NULL;
}

QIcon* YaDiskPlugin::getIcon()
{
    if(!m_icon)
	{
        m_icon = new QIcon(":/icons/yandex_disk.png");
	}

    return m_icon;
}

QString YaDiskPlugin::getTranslationFile(const QString& locale)
{
	QString file("yandex_disk_wm_pl_" + locale);
	return file;
}

Q_EXPORT_PLUGIN2(wm-ya-disk-plugin, YaDiskPlugin);
