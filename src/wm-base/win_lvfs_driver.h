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

#ifndef LVFSDRIVER_H
#define LVFSDRIVER_H

#define _WIN32_WINNT 0x5100

//QT headers
#include <QThread>
#include <QMutex>

//Windows specific headers
#include <windows.h>
#include <winbase.h>

//Dokan driver header
#include <dokan.h>

//WebMounter headers
#include "lvfs_driver.h"
#include "file_proxy.h"
//#include "NotificationDevice.h"
#include "data.h"

//#include "control_panel.h"
#include "common_stuff.h"

namespace LocalDriver
{
	using Common::FileProxy;
	using Ui::NotificationDevice;
	using Ui::Notification;

	class LVFSDriver : public ILVFSDriver
	{
		Q_OBJECT

	private:
		//LVFSDriver(void);
		~LVFSDriver(void);
		LVFSDriver(FileProxy * pProxy);
		LVFSDriver(const LVFSDriver &); 
		LVFSDriver& operator=(const LVFSDriver &); 
		void run();

		public slots:
			virtual void mount(Data::GeneralSettings& generalSettings);
			virtual void unmount();

	private:	//filesystem calls
		//static bool __stdcall 
		//	DokanCreateEmptyFile(LPCWSTR FileName);

		static int __stdcall 
			DokanCreateFile(
			LPCWSTR					FileName,
			DWORD					AccessMode,
			DWORD					ShareMode,
			DWORD					CreationDisposition,
			DWORD					FlagsAndAttributes,
			PDOKAN_FILE_INFO		DokanFileInfo);

		static int __stdcall
			DokanCreateDirectory(
			LPCWSTR					FileName,
			PDOKAN_FILE_INFO		DokanFileInfo);

		static int __stdcall
			DokanOpenDirectory(
			LPCWSTR					FileName,
			PDOKAN_FILE_INFO		DokanFileInfo);

		static int __stdcall
			DokanCloseFile(
			LPCWSTR					FileName,
			PDOKAN_FILE_INFO		DokanFileInfo);

		static int __stdcall
			DokanCleanup(
			LPCWSTR					FileName,
			PDOKAN_FILE_INFO		DokanFileInfo);

		static int __stdcall
			DokanReadFile(
			LPCWSTR				FileName,
			LPVOID				Buffer,
			DWORD				BufferLength,
			LPDWORD				ReadLength,
			LONGLONG			Offset,
			PDOKAN_FILE_INFO	DokanFileInfo);

		static int __stdcall
			DokanWriteFile(
			LPCWSTR		FileName,
			LPCVOID		Buffer,
			DWORD		NumberOfBytesToWrite,
			LPDWORD		NumberOfBytesWritten,
			LONGLONG			Offset,
			PDOKAN_FILE_INFO	DokanFileInfo);

		static int __stdcall
			DokanFlushFileBuffers(
			LPCWSTR		FileName,
			PDOKAN_FILE_INFO	DokanFileInfo);

		static int __stdcall
			DokanGetFileInformation(
			LPCWSTR							FileName,
			LPBY_HANDLE_FILE_INFORMATION	HandleFileInformation,
			PDOKAN_FILE_INFO				DokanFileInfo);

		static int __stdcall
			DokanFindFiles(
			LPCWSTR				FileName,
			PFillFindData		FillFindData, // function pointer
			PDOKAN_FILE_INFO	DokanFileInfo);

		static int __stdcall
			DokanDeleteFile(
			LPCWSTR				FileName,
			PDOKAN_FILE_INFO	DokanFileInfo);

		static int __stdcall
			DokanDeleteDirectory(
			LPCWSTR				FileName,
			PDOKAN_FILE_INFO	DokanFileInfo);

		static int __stdcall
			DokanMoveFile(
			LPCWSTR				FileName, // existing file name
			LPCWSTR				NewFileName,
			BOOL				ReplaceIfExisting,
			PDOKAN_FILE_INFO	DokanFileInfo);

		static int __stdcall
			DokanLockFile(
			LPCWSTR				FileName,
			LONGLONG			ByteOffset,
			LONGLONG			Length,
			PDOKAN_FILE_INFO	DokanFileInfo);

		static int __stdcall
			DokanSetEndOfFile(
			LPCWSTR				FileName,
			LONGLONG			ByteOffset,
			PDOKAN_FILE_INFO	DokanFileInfo);

		static int __stdcall
			DokanSetAllocationSize(
			LPCWSTR				FileName,
			LONGLONG			AllocSize,
			PDOKAN_FILE_INFO	DokanFileInfo);

		static int __stdcall
			DokanSetFileAttributes(
			LPCWSTR				FileName,
			DWORD				FileAttributes,
			PDOKAN_FILE_INFO	DokanFileInfo);

		static int __stdcall
			DokanSetFileTime(
			LPCWSTR				FileName,
			CONST FILETIME*		CreationTime,
			CONST FILETIME*		LastAccessTime,
			CONST FILETIME*		LastWriteTime,
			PDOKAN_FILE_INFO	DokanFileInfo);

		static int __stdcall
			DokanUnlockFile(
			LPCWSTR				FileName,
			LONGLONG			ByteOffset,
			LONGLONG			Length,
			PDOKAN_FILE_INFO	DokanFileInfo);

		static int __stdcall
			DokanGetFileSecurity(
			LPCWSTR					FileName,
			PSECURITY_INFORMATION	SecurityInformation,
			PSECURITY_DESCRIPTOR	SecurityDescriptor,
			ULONG				BufferLength,
			PULONG				LengthNeeded,
			PDOKAN_FILE_INFO	DokanFileInfo);

		static int __stdcall
			DokanSetFileSecurity(
			LPCWSTR					FileName,
			PSECURITY_INFORMATION	SecurityInformation,
			PSECURITY_DESCRIPTOR	SecurityDescriptor,
			ULONG				SecurityDescriptorLength,
			PDOKAN_FILE_INFO	DokanFileInfo);

		static int __stdcall
			DokanGetVolumeInformation(
			LPWSTR		VolumeNameBuffer,
			DWORD		VolumeNameSize,
			LPDWORD		VolumeSerialNumber,
			LPDWORD		MaximumComponentLength,
			LPDWORD		FileSystemFlags,
			LPWSTR		FileSystemNameBuffer,
			DWORD		FileSystemNameSize,
			PDOKAN_FILE_INFO	DokanFileInfo);

		static int __stdcall
			DokanWMUnmount(
			PDOKAN_FILE_INFO	DokanFileInfo);

		static void
			GetFilePath(
			PWCHAR	filePath,
			ULONG	numberOfElements,
			LPCWSTR FileName);

	private:
		static FileProxy* m_fileProxy;
		//static NotificationDevice* _pNotificationDevice;

		static LVFSDriver* m_driverInstance;

		static PDOKAN_OPERATIONS m_driverOperations;
		static PDOKAN_OPTIONS m_driverOptions;
		static QMutex m_driverMutex;

		static QString m_rootDirectory;
		static QString m_mountPoint;

	public:
		static LVFSDriver* createDriver(FileProxy*);

	private:
		static void startDriver();
		static void stopDriver();

signals:
		//void showDriverMessage(const Notification&);
		//void mounted();
		//void unmounted();
	};
}
#endif //LVFSDriver_H