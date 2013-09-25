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

#ifndef WEBMOUNTER_H
#define WEBMOUNTER_H

#include <vector>
#include <QString>

#include "vfs_cache.h"
#include "plugin_interface.h"
#include "rvfs_driver.h"
#include "lvfs_driver.h"

#include "data.h"

#include "file_proxy.h"

#include "notification_device.h"
#include "IGui.h"

//#include "control_panel.h"

using std::vector;
using namespace Data;
using namespace LocalDriver;

namespace Common
{
	typedef boost::shared_ptr<PluginInterface> PluginPtr;
	typedef std::map<QString, PluginPtr> PluginList;
	typedef std::pair <QString, PluginPtr> PluginList_Pair;

	class WEBMOUNTER_EXPORT WebMounter : public QObject
	{
		Q_OBJECT

	public:
		WebMounter(void);
		~WebMounter(void);

		void startApp(Ui::IGui* pGui);

		static Ui::NotificationDevice* getNotificationDevice() { return m_gui->getNotificationDevice(); };
		static RemoteDriver::RVFSDriver* getPlugin(const QString& pluginName);
		static FileProxy* getProxy();

		static VFSCache* getCache() {return m_vfsCache;};
		static LocalDriver::ILVFSDriver* getLocalDriver();
		static SettingStorage* getSettingStorage() {return m_globalSettings;};

		static PluginList& plugins() { return m_pluginList; };

		public slots:
			void mount();
			void unmount();

			private slots:
				void mounted();
				void unmounted();

	private:
		bool loadPlugins();
		void initGlobalSettings();
		void cleanCacheIfNeeded();
		void cleanCaches();
		bool largeDifference(QString version1, QString version2);

#ifdef Q_OS_WIN
		QString findFreeDriveLetter();
#endif

		//static QString _rootPath;

		static LocalDriver::ILVFSDriver* m_lvfsDriver;
		static PluginList m_pluginList;
		static VFSCache* m_vfsCache;
		static SettingStorage* m_globalSettings;
		static FileProxy* m_fileProxy;
		static Ui::IGui* m_gui;
	};
}

#endif
