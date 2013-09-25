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

#ifndef FILE_PROXY_H
#define FILE_PROXY_H

#include "common_stuff.h"
#include "rvfs_driver.h"
#include "notification_device.h"

#include <map>
#include <set>
#include <QString>
#include <QMutex>
#include <QFuture>
#include <QtCore>

namespace Common
{
	using RemoteDriver::RVFSDriver;

	class WEBMOUNTER_EXPORT FileProxy
	{
		FileProxy(void);
		//FileProxy(RVFSDriver* pDriver);
	public:
		static FileProxy* CreateFileProxy();
		~FileProxy(void);

		RESULT CreateDirectoryW(QString path);
		RESULT CreateFileW(QString path);

		RESULT MoveElement(QString from, QString to, bool bFirstRequest = true);

		void OpenDirectory(QString path);
		RESULT ReadFile(QString path);
		RESULT RemoveFile(QString path);
		RESULT RemoveDir(QString& path);

		//void Sync(QString path, bool bFullSync = false);
		RESULT UnCheckFile(QString path);
		unsigned int CheckFile(QString path);
		void WriteFile(QString path);
		//void RegisterRVFSDriver(RVFSDriver*);
		bool CheckFileAttributes(const QString& path, unsigned long attributes);

		static unsigned int GetNotUploadedCounter()
		{
			QMutexLocker locker(&m_fileProxyMutex);
			return m_notUploaded;
		}
		static unsigned int GetUploadedCounter()
		{
			QMutexLocker locker(&m_fileProxyMutex);
			return m_uploaded;
		}
		static void IncreaseUploadedCounter()
		{
			QMutexLocker locker(&m_fileProxyMutex);
			m_uploaded++;
		}
		static void IncreaseNotUploadedCounter()
		{
			//QMutexLocker locker(&m_fileProxyMutex);
			if(m_notUploaded == 0)
			{
				notifyUser(Ui::Notification::eINFO
					, QObject::tr("Info")
					, QObject::tr("Upload has been started !"));
			}
			m_notUploaded++;
		}

		static unsigned int getNotDeletedCounter()
		{
			QMutexLocker locker(&m_fileProxyMutex);
			return m_notDeleted;
		}
		static unsigned int getDeletedCounter()
		{
			QMutexLocker locker(&m_fileProxyMutex);
			return m_deleted;
		}
		static void increaseDeletedCounter()
		{
			QMutexLocker locker(&m_fileProxyMutex);
			m_deleted++;
		}
		static void increaseNotDeletedCounter()
		{
			if(m_notDeleted == 0)
			{
				notifyUser(Ui::Notification::eINFO
					, QObject::tr("Info")
					, QObject::tr("Deletion has been started !"));
			}
			m_notDeleted++;
		}

	public:
		void fileUploaded(QString filePath, RESULT result);
		void fileDeleted(const QString& filePath, RESULT result);

	private:
		static void notifyUser(Ui::Notification::_Types type, QString title, QString description);
		RVFSDriver* extractPlugin(const QString& path) const;

	private:
		static QMutex m_fileProxyMutex;
		static set<QString> m_currentFiles;

		static unsigned int m_uploaded;
		static unsigned int m_notUploaded;

		static QList<QString> m_uploadQueue;

		static unsigned int m_deleted;
		static unsigned int m_notDeleted;
		static QList<QString> m_deleteQueue;
	public:
		static FileProxy* m_fileProxyInstance;
	};
}

#endif
