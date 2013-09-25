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

#ifndef DATA_H
#define DATA_H

#include <QSettings>
#include <QMutex>
#include "common_stuff.h"

namespace Data
{
	struct GeneralSettings
	{
		QString m_proxyAddress;
		QString m_proxyLogin;
		QString m_proxyPassword;

		QString m_appStoragePath;
		QString m_appSettingStoragePath;

		QString m_appLang;
		QString m_driveLetter;

		QString m_appVersion;
	};

	struct PluginSettings
	{
		QString m_pluginName;
		QString m_serverUrl;
		QString m_userName;
		QString m_prevUserName;

		QString m_userPassword; // password is not kept in setting storage !!!

		bool m_fullSync;
		bool m_autoSync;
		QString m_syncPeriod;
		QString m_lastSync;
		bool m_isOAuthUsing;
		QString m_oAuthToken; 
	};

	class WEBMOUNTER_EXPORT SettingStorage
	{
	public:
		static SettingStorage* getStorage();
		bool restoreStorage();

		void addSettings(/*in*/ const GeneralSettings& settings);
		void addSettings(/*in*/ const PluginSettings& settings);

		void getData(/*out*/ GeneralSettings& settings);
		void getData(/*out*/ PluginSettings& settings, /*in*/ QString pluginName);

		QString getAppStoragePath();
		QString getAppSettingStoragePath();

		~SettingStorage();

	private:
		SettingStorage();

	private:
		static SettingStorage* m_storageInstance;
		static QMutex m_storageMutex;
		QSettings m_settings;
	};
}

#endif