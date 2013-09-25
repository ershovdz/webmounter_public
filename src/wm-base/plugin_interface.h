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

#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

#include <QtCore/qplugin.h>
#include <QString>
#include <QIcon>

class PluginInterface
{
public:
	virtual ~PluginInterface() {}

	virtual QString name() = 0;
	virtual void* getRVFSDriver() = 0;
	virtual void* getView() = 0;
	virtual void* getSettings() = 0;
	virtual QIcon* getIcon() = 0;
	virtual QString getTranslationFile(const QString& locale) = 0;
};

Q_DECLARE_INTERFACE(PluginInterface, "com.jsoft.webmounter.plugin/1.0");

#endif // PLUGIN_INTERFACE_H
