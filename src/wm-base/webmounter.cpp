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

#include "common_stuff.h"

#include "webmounter.h"
#include "filesystemtools.h"
#include "plugin_interface.h"

#include <QDir>
#include <QFileInfo>
#include <QtNetwork/QNetworkProxyFactory>

#include LVFS_DRIVER_H

using namespace Common;
using namespace Ui;

VFSCache* WebMounter::m_vfsCache = 0;
SettingStorage* WebMounter::m_globalSettings = 0;
Ui::IGui* WebMounter::m_gui = 0;
Common::PluginList WebMounter::m_pluginList;
FileProxy* WebMounter::m_fileProxy = 0;
ILVFSDriver* WebMounter::m_lvfsDriver = 0;

WebMounter::WebMounter(void)
{
	m_globalSettings = SettingStorage::getStorage();

	m_vfsCache = VFSCache::getCache( m_globalSettings->getAppSettingStoragePath() );
	cleanCacheIfNeeded();

	initGlobalSettings();

	m_fileProxy = FileProxy::CreateFileProxy();

	m_lvfsDriver = LVFSDriver::createDriver( m_fileProxy );

	connect(m_lvfsDriver, SIGNAL(mounted()), this, SLOT(mounted()));
	connect(m_lvfsDriver, SIGNAL(unmounted()), this, SLOT(unmounted())); 

	loadPlugins();
}

void WebMounter::initGlobalSettings()
{
	QNetworkProxyFactory::setUseSystemConfiguration( true );
	QList<QNetworkProxy> proxyList = QNetworkProxyFactory::systemProxyForQuery(QNetworkProxyQuery(QUrl("http://www.yandex.ru")));

	Data::GeneralSettings settings;
	m_globalSettings->getData( settings );

	if( !proxyList.empty() )
	{
		if( !proxyList[0].hostName().isEmpty() )
		{
			settings.m_proxyAddress = proxyList[0].hostName() + ":" + QString::number(proxyList[0].port());
		}
	}

#ifdef Q_OS_WIN
	settings.m_driveLetter = findFreeDriveLetter();
#endif

	m_globalSettings->addSettings(settings);
}

#ifdef Q_OS_WIN
QString WebMounter::findFreeDriveLetter()
{
	QFileInfoList driveList = QFSFileEngine::drives();

	for(char a = 67; a < 91; a++)
	{
		bool insert = true;
		QString letter = (QString::fromAscii(&a, 1) + ":");
		QString letterWithSlash = (QString::fromAscii(&a, 1) + ":/");

		for( QFileInfoList::Iterator iter = driveList.begin(); 
			iter != driveList.end();
			++iter )
		{
			if(iter->canonicalPath() == letter || iter->canonicalPath() == letterWithSlash)
			{
				insert = false;
				break;
			}
		}
		if(insert)
		{
			return letter;
		}
	}

	return "";
}
#endif

WebMounter::~WebMounter(void)
{
	delete m_lvfsDriver;
	delete m_fileProxy;
	if(m_vfsCache) delete m_vfsCache; // to release DB connection
	delete m_globalSettings;
}

RemoteDriver::RVFSDriver* WebMounter::getPlugin(const QString& pluginName)
{
	PluginList::iterator iter;
	iter = m_pluginList.find(pluginName);
	if(iter != m_pluginList.end())
	{
		return (RemoteDriver::RVFSDriver*)iter->second.get()->getRVFSDriver();
	}
	return 0;
}

FileProxy* WebMounter::getProxy()
{
	return m_fileProxy;
}

void WebMounter::startApp(Ui::IGui* pGui)
{
	Q_ASSERT(pGui);
	m_gui = pGui;

	mount();
}

LocalDriver::ILVFSDriver* WebMounter::getLocalDriver()
{
	return m_lvfsDriver;
}

bool WebMounter::loadPlugins()
{
	QDir pluginsDir(qApp->applicationDirPath());
	pluginsDir.cd("..");
	pluginsDir.cd("lib");

	if(pluginsDir.exists(pluginsDir.absolutePath() + QDir::separator() + "webmounter"))
	{
		pluginsDir.cd("webmounter/plugins");
	}
	else
	{
		pluginsDir.cd("plugins");
	}

	foreach (QString fileName, pluginsDir.entryList(QDir::Files))
	{
		QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
		QObject *plugin = pluginLoader.instance();
		if (plugin)
		{
			PluginInterface* pluginInterface = qobject_cast<PluginInterface*>(plugin);
			if(pluginInterface)
			{
				QString name = pluginInterface->name();
				m_pluginList.insert(PluginList_Pair(pluginInterface->name(), PluginPtr(pluginInterface)));

				QDir dir;
				if(!dir.exists(m_globalSettings->getAppStoragePath() + QDir::separator() + name))
				{
					dir.mkdir(m_globalSettings->getAppStoragePath() + QDir::separator() + name);
				}

				QFileInfo fInfo(m_globalSettings->getAppStoragePath() + QDir::separator() + name);
				VFSElement elem = VFSElement(VFSElement::DIRECTORY
					, fInfo.absoluteFilePath()
					, "ROOT"
					, ""
					, ""
					, ""
					, ROOT_ID
					, ROOT_ID
					, ""
					, name);

				VFSCache::iterator parentDir = WebMounter::getCache()->find(m_globalSettings->getAppStoragePath() + QDir::separator() + name);
				if(parentDir == WebMounter::getCache()->end())
				{
					WebMounter::getCache()->insert(elem);
				}
			}
		}
	}

	return true;
}

void WebMounter::mount()
{
	Q_ASSERT(m_lvfsDriver);

	Data::GeneralSettings settings;
	m_globalSettings->getData( settings );

	m_lvfsDriver->mount( settings );
}

void WebMounter::unmount()
{
	Q_ASSERT(m_lvfsDriver);
	m_lvfsDriver->unmount();
}

void WebMounter::mounted()
{
	m_gui->mounted();
}

void WebMounter::unmounted()
{
	m_gui->unmounted();

}

void WebMounter::cleanCacheIfNeeded() 
{
	Data::GeneralSettings settings;
	m_globalSettings->getData( settings );
	QString currentVersion(VERSION);
	if( currentVersion != settings.m_appVersion)
	{
		if( largeDifference(currentVersion, settings.m_appVersion) )
		{
			cleanCaches();
		}
		settings.m_appVersion = currentVersion;
		m_globalSettings->addSettings(settings);
	}
}

bool WebMounter::largeDifference(QString version1, QString version2)
{
	if( "" == version1 || "" == version2 || !version1.startsWith(version2.mid(0,4)) ) // e.g. 1.3.0 and 1.2.0
		return true;
	else
		return false;
}

void WebMounter::cleanCaches()
{
	if( NULL == m_vfsCache )
		throw std::string("vfsCache has not been loaded");

	Data::GeneralSettings settings;
	m_globalSettings->getData( settings );

	QDir dir( settings.m_appStoragePath );

	FileSystemTools::removeFolderContent( dir );
	m_vfsCache->clean();
}