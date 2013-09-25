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

#ifndef RVFS_DRIVER_H
#define RVFS_DRIVER_H

#include <map>
#include <QWaitCondition>
#include <QThread>
#include <QDir>

#include "common_stuff.h"
#include "data.h"
#include "vfs_cache.h"
#include "notification_device.h"

using namespace std;
using Common::RESULT;

#define MAX_DOWNLOAD_RETRY 5
#define MAX_UPLOAD_RETRY 5
#define DOWNLOAD_CHUNK_SIZE 30

namespace RemoteDriver
{
	using namespace Data;

	enum DriverState
	{
		eNotConnected
		, eAuthInProgress
		, eAuthorized
		, eSync
		, eConnected
		, eSyncStopping
	};

	class WEBMOUNTER_EXPORT RVFSDriver : public QThread
	{
		Q_OBJECT

	public:
		virtual RESULT downloadFiles();

		virtual RESULT downloadFiles(QList <QString>& urlList, QList <QString>& pathList) = 0;

		virtual RESULT uploadFile(const QString& path, const QString& title,  const QString& id, const QString& parentId) = 0;

		virtual RESULT modifyFile(const QString&) = 0;

		virtual RESULT renameElement( const QString& path, const QString& newTitle) = 0;

		virtual RESULT deleteDirectory( const QString& path) = 0;

		virtual RESULT deleteFile( const QString& path) = 0;

		virtual RESULT moveElement( const QString& path, const QString& newParentId) = 0;

		virtual RESULT createDirectory(const QString& path,  const QString& parentid, const QString& title) = 0;

		virtual RESULT createFile(const QString& path, const QString& title,  const QString& id, const QString& parentId);

		virtual RESULT getElements(QList<VFSElement>& elements) = 0;

		virtual RESULT sync();

		virtual bool areFileAttributesValid(const QString& path, unsigned long attributes);

	protected:
		virtual void notifyUser(Ui::Notification::_Types type, QString title, QString description) const;
		virtual void updateState(int progress, DriverState newState);
		virtual void syncCacheWithFileSystem(const QString& path);
		virtual void updateDownloadStatus(RESULT downloadResult, const unsigned int uDownloaded, const unsigned int uNotDownloaded);
		unsigned int countNotDownloaded();
		unsigned int countObjects(VFSElement::VFSElementType type);
		void updateSyncStatus(double currentStatus);
		void updateChildrenPath(const VFSElement& elem);
		void mergeToCache(QList<VFSElement>& elements);

		void run();
Q_SIGNALS:

		void updateView(int, int);
		void fileUploaded(/*QString, RESULT*/);

		public Q_SLOTS:

			virtual void startPlugin(Data::PluginSettings&);

			virtual void stopPlugin();

			virtual void startSync();

			virtual void stopSync();

	public:

		virtual void connectHandler(PluginSettings& pluginSettings);

		virtual void disconnectHandler();

		virtual void syncHandler();

		virtual void stopSyncHandler();

		virtual DriverState getState() {return m_state;};

		virtual void setState(DriverState state) {m_state = state;};

	protected:
		DriverState m_state;
		QString m_pluginName;
		QMutex m_driverMutex;
		QWaitCondition m_forceSync;
		QMutex m_syncMutex;
		struct SyncProgressData
		{
			double m_currProgress;
			int m_maxValue;
		} m_progressData;
	};
}

#endif //RVFS_DRIVER_H
