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

#include "vk_plugin.h"
#include "./driver/vk_driver.h"
#include "./view/vk_view.h"
#include "../../wm-base/data.h"
#include "webmounter.h"

VkPlugin::VkPlugin() : m_driver(NULL), m_view(NULL), m_icon(NULL)
{
}

VkPlugin::~VkPlugin()
{
}

QString VkPlugin::name()
{
	QString name = QString(tr("VKontakte"));
	return name;
}

void* VkPlugin::getRVFSDriver()
{
    if(!m_driver)
        m_driver = new RemoteDriver::VkRVFSDriver(name());
    return (void*)(m_driver);
}

void* VkPlugin::getView()
{
    if(!m_view)
	{
		Data::PluginSettings settings;
        settings.m_pluginName = name();
		Common::WebMounter::getSettingStorage()->getData(settings, name());

        m_view = new Ui::VkView(&settings, name());
	}

    return (void*)(m_view);
}

void* VkPlugin::getSettings()
{
	return NULL;
}

QIcon* VkPlugin::getIcon()
{
    if(!m_icon)
	{
        m_icon = new QIcon(":/icons/vk.png");
	}

    return m_icon;
}

QString VkPlugin::getTranslationFile(const QString& locale)
{
	return QString("vk_wm_pl_" + locale);
}

Q_EXPORT_PLUGIN2(wm-vk-plugin, VkPlugin);
