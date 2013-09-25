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

#include "data.h"
#include <QMutexLocker>
#include <QDesktopServices>
#include <QDir>

namespace Data
{
	SettingStorage* SettingStorage::m_storageInstance = 0;
	QMutex SettingStorage::m_storageMutex;

	SettingStorage::SettingStorage() 
		: m_settings(QSettings::IniFormat, QSettings::UserScope, QString("JSoft"), QString("WebMounter"))
	{	
		QString storagePath = m_settings.value("general/appStoragePath").toString();
		QString settingStoragePath = m_settings.value("general/appSettingStoragePath").toString();
		QDir dir;

		QString fn = m_settings.fileName();
		if(storagePath == "")
		{
			storagePath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
			storagePath += QString(QDir::separator()) + "storage";

			m_settings.setValue("general/appStoragePath", storagePath);
		}

		if(settingStoragePath == "")
		{
			settingStoragePath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
			settingStoragePath += QString(QDir::separator()) + "settings";

			m_settings.setValue("general/appSettingStoragePath", settingStoragePath);
		}

		bool res = false;
		if(!dir.exists(storagePath))
		{
			res = dir.mkpath(storagePath);
		}

		if(!dir.exists(settingStoragePath))
		{
			res = dir.mkpath(settingStoragePath);
		}
	}

	SettingStorage::~SettingStorage()
	{
	}

	SettingStorage* SettingStorage::getStorage()
	{
		QMutexLocker locker(&m_storageMutex);

		if(!m_storageInstance)
		{
			m_storageInstance = new SettingStorage();
		}
		return m_storageInstance;
	}

	void SettingStorage::addSettings(const GeneralSettings& settings)
	{
		QMutexLocker locker(&m_storageMutex);

		m_settings.beginGroup("general");
		m_settings.setValue("proxyAddress", settings.m_proxyAddress);
		m_settings.setValue("proxyLogin", settings.m_proxyLogin);
		m_settings.setValue("proxyPassword", settings.m_proxyPassword);
		m_settings.setValue("appStoragePath", settings.m_appStoragePath);
		m_settings.setValue("appLang", settings.m_appLang);
		m_settings.setValue("driveLetter", settings.m_driveLetter);
		m_settings.setValue("appVersion", settings.m_appVersion);
		m_settings.endGroup();
	}

	void SettingStorage::addSettings(const PluginSettings& settings)
	{
		QMutexLocker locker(&m_storageMutex);

		m_settings.beginGroup(settings.m_pluginName);
		m_settings.setValue("serverUrl", settings.m_serverUrl);
		m_settings.setValue("userName", settings.m_userName);
		m_settings.setValue("prevUserName", settings.m_prevUserName);
		m_settings.setValue("fullSync", settings.m_fullSync);
		m_settings.setValue("autoSync", settings.m_autoSync);
		m_settings.setValue("syncPeriod", settings.m_syncPeriod);
		m_settings.setValue("lastSync", settings.m_lastSync);
		m_settings.setValue("isOAuthUsing", settings.m_isOAuthUsing);
		m_settings.setValue("oAuthToken", settings.m_oAuthToken);
		m_settings.endGroup();
	}

	void SettingStorage::getData(GeneralSettings& settings)
	{
		QMutexLocker locker(&m_storageMutex);

		settings.m_proxyAddress   = m_settings.value("general/proxyAddress", "").toString();
		settings.m_proxyLogin     = m_settings.value("general/proxyLogin", "").toString();
		settings.m_proxyPassword  = m_settings.value("general/proxyPassword", "").toString();
		settings.m_appStoragePath = m_settings.value("general/appStoragePath", "").toString();
		settings.m_appLang		= m_settings.value("general/appLang", "").toString();
		settings.m_driveLetter	= m_settings.value("general/driveLetter", "").toString();
		settings.m_appVersion	= m_settings.value("general/appVersion", "").toString();
	}

	QString SettingStorage::getAppStoragePath()
	{
		QMutexLocker locker(&m_storageMutex);

		return m_settings.value("general/appStoragePath", "").toString();
	}

	QString SettingStorage::getAppSettingStoragePath()
	{
		QMutexLocker locker(&m_storageMutex);

		return  m_settings.value("general/appSettingStoragePath", "").toString();
	}

	void SettingStorage::getData(PluginSettings& settings, QString pluginName)
	{
		QMutexLocker locker(&m_storageMutex);

		settings.m_pluginName   = pluginName;
		m_settings.beginGroup(pluginName);

		settings.m_serverUrl  = m_settings.value("serverUrl", "").toString();
		settings.m_userName   = m_settings.value("userName", "").toString();
		settings.m_prevUserName = m_settings.value("prevUserName", "").toString();
		settings.m_fullSync  = m_settings.value("fullSync", true).toBool();
		settings.m_autoSync  = m_settings.value("autoSync", true).toBool();
		settings.m_syncPeriod = m_settings.value("syncPeriod", "300").toString();
		settings.m_isOAuthUsing = m_settings.value("isOAuthUsing", false).toBool();
		settings.m_oAuthToken = m_settings.value("oAuthToken", "").toString();

		if(!settings.m_autoSync)
		{
			settings.m_syncPeriod = QString("300");
		}
		settings.m_lastSync   = m_settings.value("lastSync", "").toString();

		m_settings.endGroup();
	}
};

