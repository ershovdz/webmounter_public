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

#include <QString>

#include "win_lvfs_driver.h"

#include "WebMounter.h"


namespace LocalDriver
{
	using namespace Common;
	using namespace Data;

	QString LVFSDriver::m_rootDirectory;
	QString LVFSDriver::m_mountPoint;

	static BOOL g_UseStdErr = false;
	static BOOL g_DebugMode = false;

	LVFSDriver* LVFSDriver::m_driverInstance = NULL;
	//bool LVFSDriver::_bDriverIsRunning = false;
	PDOKAN_OPERATIONS LVFSDriver::m_driverOperations = NULL;
	PDOKAN_OPTIONS LVFSDriver::m_driverOptions = NULL;
	QMutex LVFSDriver::m_driverMutex;
	//NotificationDevice* LVFSDriver::_pNotificationDevice = NULL;
	FileProxy* LVFSDriver::m_fileProxy = NULL;

	LVFSDriver::~LVFSDriver(void)
	{
	}
	LVFSDriver::LVFSDriver(FileProxy * pProxy)
	{
		m_fileProxy = pProxy;
	}

	void LVFSDriver::mount(Data::GeneralSettings& generalSettings)
	{
		if( !isRunning() )
		{
			m_rootDirectory = generalSettings.m_appStoragePath;
			m_mountPoint = generalSettings.m_driveLetter;

			startDriver();
		}
	}

	void LVFSDriver::unmount()
	{
		stopDriver();
	}

	static void DbgPrint(LPCWSTR format, ...)
	{
		if (g_DebugMode) 
		{
			WCHAR buffer[512];
			va_list argp;
			va_start(argp, format);
			vswprintf(buffer, format, argp);
			va_end(argp);
			//FILE* log = fopen("G:\\SRC\\WebDisk\\Debug\\driver.log", "a+");
			//if (0)
			//{
			//fwprintf(log, buffer);
			//} else {
			OutputDebugStringW(buffer);
			//}
			//fclose(log);
		}
	}

	void
		LVFSDriver::GetFilePath(
		PWCHAR	filePath,
		ULONG	numberOfElements,
		LPCWSTR FileName)
	{
		RtlZeroMemory(filePath, numberOfElements * sizeof(WCHAR));
		wcsncpy(filePath, m_rootDirectory.toStdWString().c_str(), m_rootDirectory.size());
		wcsncat(filePath, FileName, wcslen(FileName));
	}

#define CheckFlag(val, flag) if (val&flag) { DbgPrint(L"\t" L#flag L"\n"); }

	//bool __stdcall 
	//LVFSDriver::DokanCreateEmptyFile(LPCWSTR FileName)
	//{
	//	HANDLE hFile = CreateFile(FileName,               
	//                       GENERIC_READ,          
	//                       FILE_SHARE_READ,      
	//                       NULL,                  
	//                       CREATE_ALWAYS,         
	//                       FILE_ATTRIBUTE_NORMAL, 
	//                       NULL);
	//
	//	if (hFile == INVALID_HANDLE_VALUE) 
	//    { 
	//        return true; 
	//    }
	//
	//	CloseHandle (hFile);
	//	return false;
	//}

	int __stdcall
		LVFSDriver::DokanCreateFile(
		LPCWSTR					FileName,
		DWORD					AccessMode,
		DWORD					ShareMode,
		DWORD					CreationDisposition,
		DWORD					FlagsAndAttributes,
		PDOKAN_FILE_INFO		DokanFileInfo)
	{
		WCHAR filePath[MAX_PATH];
		HANDLE handle;
		DWORD fileAttr;

		GetFilePath(filePath, MAX_PATH, FileName);

		DbgPrint(L"CreateFile : %s, CreationDisposition = %d\n", filePath, CreationDisposition);

		//PrintUserName(DokanFileInfo);

		QMutexLocker locker(&m_driverMutex);
		if (CreationDisposition == CREATE_NEW)
		{
			DbgPrint(L"\tCREATE_NEW\n");
		}
		if (CreationDisposition == OPEN_ALWAYS)
			DbgPrint(L"\tOPEN_ALWAYS\n");
		if (CreationDisposition == CREATE_ALWAYS)
			DbgPrint(L"\tCREATE_ALWAYS\n");
		if (CreationDisposition == OPEN_EXISTING)
			DbgPrint(L"\tOPEN_EXISTING\n");
		if (CreationDisposition == TRUNCATE_EXISTING)
			DbgPrint(L"\tTRUNCATE_EXISTING\n");

		/*
		if (ShareMode == 0 && AccessMode & FILE_WRITE_DATA)
		ShareMode = FILE_SHARE_WRITE;
		else if (ShareMode == 0)
		ShareMode = FILE_SHARE_READ;
		*/

		DbgPrint(L"\tShareMode = 0x%x\n", ShareMode);

		//CheckFlag(ShareMode, FILE_SHARE_READ);
		//CheckFlag(ShareMode, FILE_SHARE_WRITE);
		//CheckFlag(ShareMode, FILE_SHARE_DELETE);

		DbgPrint(L"\tAccessMode = 0x%x\n", AccessMode);

		//CheckFlag(AccessMode, GENERIC_READ);
		//CheckFlag(AccessMode, GENERIC_WRITE);
		//CheckFlag(AccessMode, GENERIC_EXECUTE);

		//CheckFlag(AccessMode, DELETE);
		//CheckFlag(AccessMode, FILE_READ_DATA);
		//CheckFlag(AccessMode, FILE_READ_ATTRIBUTES);
		//CheckFlag(AccessMode, FILE_READ_EA);
		//CheckFlag(AccessMode, READ_CONTROL);
		//CheckFlag(AccessMode, FILE_WRITE_DATA);
		//CheckFlag(AccessMode, FILE_WRITE_ATTRIBUTES);
		//CheckFlag(AccessMode, FILE_WRITE_EA);
		//CheckFlag(AccessMode, FILE_APPEND_DATA);
		//CheckFlag(AccessMode, WRITE_DAC);
		//CheckFlag(AccessMode, WRITE_OWNER);
		//CheckFlag(AccessMode, SYNCHRONIZE);
		//CheckFlag(AccessMode, FILE_EXECUTE);
		//CheckFlag(AccessMode, STANDARD_RIGHTS_READ);
		//CheckFlag(AccessMode, STANDARD_RIGHTS_WRITE);
		//CheckFlag(AccessMode, STANDARD_RIGHTS_EXECUTE);

		// When filePath is a directory, needs to change the flag so that the file can be opened.
		fileAttr = GetFileAttributes(filePath);
		if (fileAttr && fileAttr & FILE_ATTRIBUTE_DIRECTORY) {
			FlagsAndAttributes |= FILE_FLAG_BACKUP_SEMANTICS;
			DbgPrint(L"\tSetDirAttr !!!\n");
			//AccessMode = 0;
		}
		DbgPrint(L"\tFlagsAndAttributes = 0x%x\n", FlagsAndAttributes);

		//CheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_ARCHIVE);
		//CheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_ENCRYPTED);
		//CheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_HIDDEN);
		//CheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_NORMAL);
		//CheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_NOT_CONTENT_INDEXED);
		//CheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_OFFLINE);
		//CheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_READONLY);
		//CheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_SYSTEM);
		//CheckFlag(FlagsAndAttributes, FILE_ATTRIBUTE_TEMPORARY);
		//CheckFlag(FlagsAndAttributes, FILE_FLAG_WRITE_THROUGH);
		//CheckFlag(FlagsAndAttributes, FILE_FLAG_OVERLAPPED);
		//CheckFlag(FlagsAndAttributes, FILE_FLAG_NO_BUFFERING);
		//CheckFlag(FlagsAndAttributes, FILE_FLAG_RANDOM_ACCESS);
		//CheckFlag(FlagsAndAttributes, FILE_FLAG_SEQUENTIAL_SCAN);
		//CheckFlag(FlagsAndAttributes, FILE_FLAG_DELETE_ON_CLOSE);
		//CheckFlag(FlagsAndAttributes, FILE_FLAG_BACKUP_SEMANTICS);
		//CheckFlag(FlagsAndAttributes, FILE_FLAG_POSIX_SEMANTICS);
		//CheckFlag(FlagsAndAttributes, FILE_FLAG_OPEN_REPARSE_POINT);
		//CheckFlag(FlagsAndAttributes, FILE_FLAG_OPEN_NO_RECALL);
		//CheckFlag(FlagsAndAttributes, SECURITY_ANONYMOUS);
		//CheckFlag(FlagsAndAttributes, SECURITY_IDENTIFICATION);
		//CheckFlag(FlagsAndAttributes, SECURITY_IMPERSONATION);
		//CheckFlag(FlagsAndAttributes, SECURITY_DELEGATION);
		//CheckFlag(FlagsAndAttributes, SECURITY_CONTEXT_TRACKING);
		//CheckFlag(FlagsAndAttributes, SECURITY_EFFECTIVE_ONLY);
		//CheckFlag(FlagsAndAttributes, SECURITY_SQOS_PRESENT);

		handle = CreateFile(
			filePath,
			AccessMode,//GENERIC_READ|GENERIC_WRITE|GENERIC_EXECUTE,
			ShareMode,
			NULL, // security attribute
			CreationDisposition,
			FlagsAndAttributes,// |FILE_FLAG_NO_BUFFERING,
			NULL); // template file handle

		if (handle == INVALID_HANDLE_VALUE) {
			DWORD error = GetLastError();
			DbgPrint(L"\terror code = %d\n\n", error);
			return error * -1; // error codes are negated value of Windows System Error codes
		}

		DbgPrint(L"\n");

		if (CreationDisposition == CREATE_NEW)
		{
			DbgPrint(L"Create File on server\n");

			CloseHandle(handle);
			handle = INVALID_HANDLE_VALUE;
			int res = m_fileProxy->CreateFileW(QString::fromWCharArray(filePath));
			if(res != eNO_ERROR)
			{
				DbgPrint(L"  DeleteFile ");
				if (DeleteFile(filePath) == 0) 
				{
					DbgPrint(L" error code = %d\n\n", GetLastError());
				}
				else 
				{
					DbgPrint(L"success\n\n");
				}

				return -1;
			}

			handle = CreateFile(
				filePath,
				AccessMode,//GENERIC_READ|GENERIC_WRITE|GENERIC_EXECUTE,
				ShareMode,
				NULL, // security attribute
				OPEN_EXISTING,
				FlagsAndAttributes,// |FILE_FLAG_NO_BUFFERING,
				NULL); // template file handle

			if (handle == INVALID_HANDLE_VALUE) 
			{
				DWORD error = GetLastError();
				DbgPrint(L"\terror code = %d\n\n", error);
				return error * -1; // error codes are negated value of Windows System Error codes
			}
		}

		// save the file handle in Context
		DokanFileInfo->Context = (ULONG64)handle;
		return 0;
	}


	int __stdcall
		LVFSDriver::DokanCreateDirectory(
		LPCWSTR					FileName,
		PDOKAN_FILE_INFO		/*DokanFileInfo*/)
	{
		WCHAR filePath[MAX_PATH];
		GetFilePath(filePath, MAX_PATH, FileName);

		DbgPrint(L"CreateDirectory : %s\n", filePath);

		int res = m_fileProxy->CreateDirectoryW(QString::fromWCharArray(filePath));
		if(res == eERROR_WRONG_STATE)
		{
			return ERROR_NOT_SUPPORTED;
		}
		else if(res != eNO_ERROR)
		{
			return -2;
		}

		if (!CreateDirectory(filePath, NULL)) {
			DWORD error = GetLastError();
			DbgPrint(L"\terror code = %d\n\n", error);
			return error * -1; // error codes are negated value of Windows System Error codes
		}
		return 0;
	}


	int __stdcall
		LVFSDriver::DokanOpenDirectory(
		LPCWSTR					FileName,
		PDOKAN_FILE_INFO		DokanFileInfo)
	{
		WCHAR filePath[MAX_PATH];
		HANDLE handle;
		DWORD attr;

		GetFilePath(filePath, MAX_PATH, FileName);

		DbgPrint(L"OpenDirectory : %s\n", filePath);

		attr = GetFileAttributes(filePath);
		if (attr == INVALID_FILE_ATTRIBUTES) {
			DWORD error = GetLastError();
			DbgPrint(L"\terror code = %d\n\n", error);
			return error * -1;
		}
		if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) {
			return -1;
		}

		handle = CreateFile(
			filePath,
			0,
			FILE_SHARE_READ|FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS,
			NULL);

		if (handle == INVALID_HANDLE_VALUE) {
			DWORD error = GetLastError();
			DbgPrint(L"\terror code = %d\n\n", error);
			return error * -1;
		}

		DbgPrint(L"\n");

		DokanFileInfo->Context = (ULONG64)handle;

		return 0;
	}


	int __stdcall
		LVFSDriver::DokanCloseFile(
		LPCWSTR					FileName,
		PDOKAN_FILE_INFO		DokanFileInfo)
	{
		//QMutexLocker locker(&m_driverMutex);

		WCHAR filePath[MAX_PATH];
		GetFilePath(filePath, MAX_PATH, FileName);

		if (DokanFileInfo->Context) {
			DbgPrint(L"CloseFile: %s\n", filePath);
			DbgPrint(L"\terror : not cleanuped file\n\n");
			CloseHandle((HANDLE)DokanFileInfo->Context);
			DokanFileInfo->Context = 0;
		} else {
			//DbgPrint(L"Close: %s\n\tinvalid handle\n\n", filePath);
			DbgPrint(L"Close: %s\n\n", filePath);
			return 0;
		}

		//DbgPrint(L"\n");
		return 0;
	}


	int __stdcall
		LVFSDriver::DokanCleanup(
		LPCWSTR					FileName,
		PDOKAN_FILE_INFO		DokanFileInfo)
	{
		//QMutexLocker locker(&m_driverMutex);

		WCHAR filePath[MAX_PATH];
		GetFilePath(filePath, MAX_PATH, FileName);

		if (DokanFileInfo->Context && (HANDLE)DokanFileInfo->Context != INVALID_HANDLE_VALUE) {
			DbgPrint(L"Cleanup: %s, handle = %d\n\n", filePath, (HANDLE)DokanFileInfo->Context);
			CloseHandle((HANDLE)DokanFileInfo->Context);
			DokanFileInfo->Context = 0;

			if (DokanFileInfo->DeleteOnClose) {
				DbgPrint(L"\tDeleteOnClose\n");
				if (DokanFileInfo->IsDirectory) {
					DbgPrint(L"  DeleteDirectory ");
					if (!RemoveDirectory(filePath)) {
						DbgPrint(L"error code = %d\n\n", GetLastError());
					} else {
						DbgPrint(L"success\n\n");
					}
				} else {
					DbgPrint(L"  DeleteFile ");
					if (DeleteFile(filePath) == 0) {
						DbgPrint(L" error code = %d\n\n", GetLastError());
					} else {
						DbgPrint(L"success\n\n");
					}
				}
			}

		} else {
			DbgPrint(L"Cleanup: %s\n\tinvalid handle\n\n", filePath);
			return -1;
		}

		return 0;
	}


	int __stdcall
		LVFSDriver::DokanReadFile(
		LPCWSTR				FileName,
		LPVOID				Buffer,
		DWORD				BufferLength,
		LPDWORD				ReadLength,
		LONGLONG			Offset,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		WCHAR	filePath[MAX_PATH];
		HANDLE	handle = (HANDLE)DokanFileInfo->Context;
		ULONG	offset = (ULONG)Offset;
		BOOL	opened = FALSE;

		GetFilePath(filePath, MAX_PATH, FileName);
		DbgPrint(L"ReadFile : %s\n", filePath);

		QMutexLocker locker(&m_driverMutex);

		uint fileState = m_fileProxy->CheckFile(QString::fromWCharArray(filePath));

		if(!(fileState & VFSElement::eFl_Downloading)
			&& !(fileState & VFSElement::eFl_Downloaded))
		{
			CloseHandle(handle);
			handle = INVALID_HANDLE_VALUE;
			DokanFileInfo->Context = (ULONG64)handle;

			Sleep(100); //hack. We need a time to close file 

			if(m_fileProxy->ReadFile(QString::fromWCharArray(filePath)))
			{
				return -1;
			}

			Sleep(100); //hack. We need a time to close file 

			handle = CreateFile(filePath
				, GENERIC_READ
				, FILE_SHARE_READ
				, NULL
				, OPEN_EXISTING
				, 0
				, NULL);

			if (handle == INVALID_HANDLE_VALUE) 
			{
				DbgPrint(L"\tCreateFile error : %d\n\n", GetLastError());
				return -1;
			}

			DbgPrint(L"\tNew handle = %d : %d\n\n", handle);
			opened = TRUE;
			DokanFileInfo->Context = (ULONG64)handle;
		}
		else if((fileState &  VFSElement::eFl_Downloading)
			&& (fileState != VFSElement::eFl_All))
		{
			sleep(3);
			return -1;
		}

		if (!handle || handle == INVALID_HANDLE_VALUE) 
		{
			DbgPrint(L"\tinvalid handle, cleanuped?\n");
			handle = CreateFile(filePath
				, GENERIC_READ
				, FILE_SHARE_READ
				, NULL
				, OPEN_EXISTING
				, 0
				, NULL);

			if (handle == INVALID_HANDLE_VALUE) 
			{
				DbgPrint(L"\tCreateFile error : %d\n\n", GetLastError());
				return -1;
			}
			opened = TRUE;
			DokanFileInfo->Context = (ULONG64)handle;
		}

		if (SetFilePointer(handle, offset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) 
		{ 
			DbgPrint(L"\tseek error, offset = %d, error = %d, handle = %d\n\n", offset, GetLastError(), handle);
			if (opened)
				CloseHandle(handle);
			return -1;
		}


		if (!ReadFile(handle, Buffer, BufferLength, ReadLength, NULL)) 
		{
			DbgPrint(L"\tread error = %u, buffer length = %d, read length = %d\n\n",
				GetLastError(), BufferLength, *ReadLength);
			if (opened)
			{
				CloseHandle(handle);
				handle = INVALID_HANDLE_VALUE;
				DokanFileInfo->Context = (ULONG64)handle;
			}
			return -1;

		} 
		else 
		{
			DbgPrint(L"\tread %d, offset %d\n\n", *ReadLength, offset);
		}

		/*if((offset) == GetFileSize((HANDLE)DokanFileInfo->Context,  NULL))
		{
		m_fileProxy->UnCheckFile(QString::fromWCharArray(filePath));
		}*/

		if (opened)
		{
			DbgPrint(L"440 CloseHandle, handle = %d\n\n", (HANDLE)DokanFileInfo->Context);
			CloseHandle(handle);
			handle = INVALID_HANDLE_VALUE;
			DokanFileInfo->Context = (ULONG64)handle;
		}

		return 0;
	}


	int __stdcall
		LVFSDriver::DokanWriteFile(
		LPCWSTR		FileName,
		LPCVOID		Buffer,
		DWORD		NumberOfBytesToWrite,
		LPDWORD		NumberOfBytesWritten,
		LONGLONG			Offset,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		WCHAR	filePath[MAX_PATH];
		HANDLE	handle = (HANDLE)DokanFileInfo->Context;
		ULONG	offset = (ULONG)Offset;
		BOOL	opened = FALSE;

		GetFilePath(filePath, MAX_PATH, FileName);

		DbgPrint(L"WriteFile : %s, offset %I64d, length %d\n", filePath, Offset, NumberOfBytesToWrite);
		if(GetLastError() == ERROR_FILE_NOT_FOUND) // Attempt of file addition at disconnected state. Ignore writing
		{
			return 0;
		}
		// reopen the file
		if (!handle || handle == INVALID_HANDLE_VALUE) 
		{
			DbgPrint(L"\tinvalid handle, cleanuped?\n");
			handle = CreateFile(
				filePath,
				GENERIC_WRITE,
				FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);
			if (handle == INVALID_HANDLE_VALUE) {
				DbgPrint(L"\tCreateFile error : %d\n\n", GetLastError());
				return -1;
			}
			opened = TRUE;
		}

		if (DokanFileInfo->WriteToEndOfFile) {
			if (SetFilePointer(handle, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER) {
				DbgPrint(L"\tseek error, offset = EOF, error = %d\n", GetLastError());
				return -1;
			}
		} else if (SetFilePointer(handle, offset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
			DbgPrint(L"\tseek error, offset = %d, error = %d\n", offset, GetLastError());
			return -1;
		}


		if (!WriteFile(handle, Buffer, NumberOfBytesToWrite, NumberOfBytesWritten, NULL)) {
			DbgPrint(L"\twrite error = %u, buffer length = %d, write length = %d\n",
				GetLastError(), NumberOfBytesToWrite, *NumberOfBytesWritten);
			return -1;

		} else {
			DbgPrint(L"\twrite %d, offset %d\n\n", *NumberOfBytesWritten, offset);
		}

		// close the file when it is reopened
		if (opened)
		{	
			DbgPrint(L"508 CloseHandle, handle = %d\n\n", (HANDLE)DokanFileInfo->Context);
			CloseHandle(handle);
		}

		DbgPrint(L"\t offset = %d, NumberOfBytesWritten = %d, GetFileSize = %d \n", offset, *NumberOfBytesWritten, GetFileSize((HANDLE)DokanFileInfo->Context,  NULL));

		DWORD size = GetFileSize((HANDLE)DokanFileInfo->Context,  NULL);
		if(((offset + *NumberOfBytesWritten) == size && *NumberOfBytesWritten > 4096)
			|| (*NumberOfBytesWritten < 4096))
		{
			try
			{
				CloseHandle(handle);
				handle = INVALID_HANDLE_VALUE;
				DokanFileInfo->Context = (ULONG64)handle;
			}
			catch(...)
			{}

			Sleep(100); //hack. We need a time to close file 

			QMutexLocker locker(&m_driverMutex);
			if(m_fileProxy->CreateFileW(QString::fromWCharArray(filePath)) != eNO_ERROR)
			{
				/*DbgPrint(L"  DeleteFile ");
				if (DeleteFile(filePath) == 0) 
				{
				DbgPrint(L" error code = %d\n\n", GetLastError());
				} 
				else 
				{
				DbgPrint(L"success\n\n");
				}

				return -1;*/
			}
		}
		return 0;
	}


	int __stdcall
		LVFSDriver::DokanFlushFileBuffers(
		LPCWSTR		FileName,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		WCHAR	filePath[MAX_PATH];
		HANDLE	handle = (HANDLE)DokanFileInfo->Context;

		GetFilePath(filePath, MAX_PATH, FileName);

		DbgPrint(L"FlushFileBuffers : %s\n", filePath);

		if (!handle || handle == INVALID_HANDLE_VALUE) {
			DbgPrint(L"\tinvalid handle\n\n");
			return 0;
		}

		if (FlushFileBuffers(handle)) {
			return 0;
		} else {
			DbgPrint(L"\tflush error code = %d\n", GetLastError());
			return -1;
		}

	}


	int __stdcall
		LVFSDriver::DokanGetFileInformation(
		LPCWSTR							FileName,
		LPBY_HANDLE_FILE_INFORMATION	HandleFileInformation,
		PDOKAN_FILE_INFO				DokanFileInfo)
	{
		WCHAR	filePath[MAX_PATH];
		HANDLE	handle = (HANDLE)DokanFileInfo->Context;
		BOOL	opened = FALSE;

		GetFilePath(filePath, MAX_PATH, FileName);

		DbgPrint(L"GetFileInfo : %s\n", filePath);

		if (!handle || handle == INVALID_HANDLE_VALUE) {
			DbgPrint(L"\tinvalid handle\n\n");

			// If CreateDirectory returned FILE_ALREADY_EXISTS and 
			// it is called with FILE_OPEN_IF, that handle must be opened.
			handle = CreateFile(filePath, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING,
				FILE_FLAG_BACKUP_SEMANTICS, NULL);
			if (handle == INVALID_HANDLE_VALUE)
				return -1;
			opened = TRUE;
		}

		if (!GetFileInformationByHandle(handle,HandleFileInformation)) {
			DbgPrint(L"\terror code = %d\n", GetLastError());

			// FileName is a root directory
			// in this case, FindFirstFile can't get directory information
			if (wcslen(FileName) == 1) {
				DbgPrint(L"  root dir\n");
				HandleFileInformation->dwFileAttributes = GetFileAttributes(filePath);

			} else {
				WIN32_FIND_DATAW find;
				ZeroMemory(&find, sizeof(WIN32_FIND_DATAW));
				handle = FindFirstFile(filePath, &find);
				if (handle == INVALID_HANDLE_VALUE) {
					DbgPrint(L"\tFindFirstFile error code = %d\n\n", GetLastError());
					return -1;
				}
				HandleFileInformation->dwFileAttributes = find.dwFileAttributes;
				HandleFileInformation->ftCreationTime = find.ftCreationTime;
				HandleFileInformation->ftLastAccessTime = find.ftLastAccessTime;
				HandleFileInformation->ftLastWriteTime = find.ftLastWriteTime;
				HandleFileInformation->nFileSizeHigh = find.nFileSizeHigh;
				HandleFileInformation->nFileSizeLow = find.nFileSizeLow;
				DbgPrint(L"\tFindFiles OK, file size = %d\n", find.nFileSizeLow);
				FindClose(handle);
			}
		} else {
			DbgPrint(L"\tGetFileInformationByHandle success, file size = %d\n",
				HandleFileInformation->nFileSizeLow);
		}

		DbgPrint(L"\n");

		if (opened) {
			DbgPrint(L"603 CloseHandle, handle = %d\n\n", handle);
			CloseHandle(handle);
		}

		return 0;
	}


	int __stdcall
		LVFSDriver::DokanFindFiles(
		LPCWSTR				FileName,
		PFillFindData		FillFindData, // function pointer
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		WCHAR				filePath[MAX_PATH];
		HANDLE				hFind;
		WIN32_FIND_DATAW	findData;
		DWORD				error;
		PWCHAR				yenStar = L"\\*";
		int count = 0;

		GetFilePath(filePath, MAX_PATH, FileName);

		wcscat(filePath, yenStar);
		DbgPrint(L"FindFiles :%s\n", filePath);

		hFind = FindFirstFile(filePath, &findData);

		if (hFind == INVALID_HANDLE_VALUE) {
			DbgPrint(L"\tinvalid file handle. Error is %u\n\n", GetLastError());
			return -1;
		}

		FillFindData(&findData, DokanFileInfo);
		count++;

		while (FindNextFile(hFind, &findData) != 0) {
			FillFindData(&findData, DokanFileInfo);
			count++;
		}

		error = GetLastError();
		FindClose(hFind);

		if (error != ERROR_NO_MORE_FILES) {
			DbgPrint(L"\tFindNextFile error. Error is %u\n\n", error);
			return -1;
		}

		DbgPrint(L"\tFindFiles return %d entries in %s\n\n", count, filePath);

		return 0;
	}


	int __stdcall
		LVFSDriver::DokanDeleteFile(
		LPCWSTR				FileName,
		PDOKAN_FILE_INFO	/*DokanFileInfo*/)
	{
		WCHAR	filePath[MAX_PATH];

		GetFilePath(filePath, MAX_PATH, FileName);

		int res = m_fileProxy->RemoveFile(QString::fromWCharArray(filePath));
		if(res == eERROR_WRONG_STATE)
		{
			return ERROR_NOT_SUPPORTED;
		}
		else if(res != eNO_ERROR)
		{
			return -(int)ERROR_BAD_ARGUMENTS;
		}

		DbgPrint(L"DeleteFile %s\n", filePath);

		return 0;
	}


	int __stdcall
		LVFSDriver::DokanDeleteDirectory(
		LPCWSTR				FileName,
		PDOKAN_FILE_INFO	/*DokanFileInfo*/)
	{
		WCHAR	filePath[MAX_PATH];
		HANDLE	hFind;
		WIN32_FIND_DATAW	findData;
		ULONG	fileLen;

		ZeroMemory(filePath, sizeof(filePath));
		GetFilePath(filePath, MAX_PATH, FileName);

		DbgPrint(L"DeleteDirectory %s\n", filePath);

		fileLen = wcslen(filePath);
		if (filePath[fileLen-1] != L'\\') {
			filePath[fileLen++] = L'\\';
		}
		filePath[fileLen++] = L'*';
		filePath[fileLen] = L'\0';

		/*hFind = FindFirstFile(filePath, &findData);
		while (hFind != INVALID_HANDLE_VALUE) {
		if (wcscmp(findData.cFileName, L"..") != 0 &&
		wcscmp(findData.cFileName, L".") != 0) {
		FindClose(hFind);
		DbgPrint(L"  Directory is not empty: %s\n", findData.cFileName);
		return -(int)ERROR_DIR_NOT_EMPTY;
		}
		if (!FindNextFile(hFind, &findData)) {
		break;
		}
		}
		FindClose(hFind);
		*/
		//if (GetLastError() == ERROR_NO_MORE_FILES) 
		//{
		filePath[fileLen-1] = '\0';
		QString dir = QString::fromWCharArray(filePath);
		int res = m_fileProxy->RemoveDir(dir);
		if(res == eERROR_WRONG_STATE)
		{
			return ERROR_NOT_SUPPORTED;
		}
		else if(res != eNO_ERROR)
		{
			return -(int)ERROR_BAD_ARGUMENTS;
		}
		return 0;
		//} 
		//else 
		//{
		//  return -1;
		//}
	}


	int __stdcall
		LVFSDriver::DokanMoveFile(
		LPCWSTR				FileName, // existing file name
		LPCWSTR				NewFileName,
		BOOL				ReplaceIfExisting,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		WCHAR			filePath[MAX_PATH];
		WCHAR			newFilePath[MAX_PATH];
		BOOL			status;

		GetFilePath(filePath, MAX_PATH, FileName);
		GetFilePath(newFilePath, MAX_PATH, NewFileName);


		QMutexLocker locker(&m_driverMutex);

		int res = m_fileProxy->MoveElement(QString::fromWCharArray(filePath)
			, QString::fromWCharArray(newFilePath));
		if(res == eERROR_WRONG_STATE)
		{
			return ERROR_NOT_SUPPORTED;
		}
		else if(res != eNO_ERROR)
		{
			return -1;	
		}

		DbgPrint(L"MoveFile %s -> %s\n\n", filePath, newFilePath);

		if (DokanFileInfo->Context) 
		{
			// should close? or rename at closing?
			CloseHandle((HANDLE)DokanFileInfo->Context);
			DokanFileInfo->Context = 0;
		}

		if (ReplaceIfExisting)
			status = MoveFileEx(filePath, newFilePath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
		else
			status = MoveFile(filePath, newFilePath);


		if (status == FALSE) 
		{
			DWORD error = GetLastError();
			DbgPrint(L"\tMoveFile failed status = %d, code = %d\n", status, error);
			return -(int)error;
		} 
		else 
		{
			res =m_fileProxy->MoveElement(QString::fromWCharArray(filePath)
				, QString::fromWCharArray(newFilePath), false); // the file has been moved already

			if(res == eERROR_WRONG_STATE)
			{
				return ERROR_NOT_SUPPORTED;
			}
			else if(res != eNO_ERROR)
			{
				return -1;	
			}

			return 0;
		}
	}


	int __stdcall
		LVFSDriver::DokanLockFile(
		LPCWSTR				FileName,
		LONGLONG			ByteOffset,
		LONGLONG			Length,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		WCHAR	filePath[MAX_PATH];
		HANDLE	handle;
		LARGE_INTEGER offset;
		LARGE_INTEGER length;

		GetFilePath(filePath, MAX_PATH, FileName);

		DbgPrint(L"LockFile %s\n", filePath);

		handle = (HANDLE)DokanFileInfo->Context;
		if (!handle || handle == INVALID_HANDLE_VALUE) {
			DbgPrint(L"\tinvalid handle\n\n");
			return -1;
		}

		length.QuadPart = Length;
		offset.QuadPart = ByteOffset;

		if (LockFile(handle, offset.HighPart, offset.LowPart, length.HighPart, length.LowPart)) {
			DbgPrint(L"\tsuccess\n\n");
			return 0;
		} else {
			DbgPrint(L"\tfail\n\n");
			return -1;
		}
	}


	int __stdcall
		LVFSDriver::DokanSetEndOfFile(
		LPCWSTR				FileName,
		LONGLONG			ByteOffset,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		WCHAR			filePath[MAX_PATH];
		HANDLE			handle;
		LARGE_INTEGER	offset;

		GetFilePath(filePath, MAX_PATH, FileName);

		DbgPrint(L"SetEndOfFile %s, %I64d\n", filePath, ByteOffset);

		handle = (HANDLE)DokanFileInfo->Context;
		if (!handle || handle == INVALID_HANDLE_VALUE) {
			DbgPrint(L"\tinvalid handle\n\n");
			return -1;
		}

		offset.QuadPart = ByteOffset;
		if (!SetFilePointerEx(handle, offset, NULL, FILE_BEGIN)) {
			DbgPrint(L"\tSetFilePointer error: %d, offset = %I64d\n\n",
				GetLastError(), ByteOffset);
			return GetLastError() * -1;
		}

		if (!SetEndOfFile(handle)) {
			DWORD error = GetLastError();
			DbgPrint(L"\terror code = %d\n\n", error);
			return error * -1;
		}

		return 0;
	}


	int __stdcall
		LVFSDriver::DokanSetAllocationSize(
		LPCWSTR				FileName,
		LONGLONG			AllocSize,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		WCHAR			filePath[MAX_PATH];
		HANDLE			handle;
		LARGE_INTEGER	fileSize;

		GetFilePath(filePath, MAX_PATH, FileName);

		DbgPrint(L"SetAllocationSize %s, %I64d\n", filePath, AllocSize);

		handle = (HANDLE)DokanFileInfo->Context;
		if (!handle || handle == INVALID_HANDLE_VALUE) {
			DbgPrint(L"\tinvalid handle\n\n");
			return -1;
		}

		if (GetFileSizeEx(handle, &fileSize)) {
			if (AllocSize < fileSize.QuadPart) {
				fileSize.QuadPart = AllocSize;
				if (!SetFilePointerEx(handle, fileSize, NULL, FILE_BEGIN)) {
					DbgPrint(L"\tSetAllocationSize: SetFilePointer eror: %d, "
						L"offset = %I64d\n\n", GetLastError(), AllocSize);
					return GetLastError() * -1;
				}
				if (!SetEndOfFile(handle)) {
					DWORD error = GetLastError();
					DbgPrint(L"\terror code = %d\n\n", error);
					return error * -1;
				}
			}
		} else {
			DWORD error = GetLastError();
			DbgPrint(L"\terror code = %d\n\n", error);
			return error * -1;
		}
		return 0;
	}


	int __stdcall
		LVFSDriver::DokanSetFileAttributes(
		LPCWSTR				FileName,
		DWORD				FileAttributes,
		PDOKAN_FILE_INFO	/*DokanFileInfo*/)
	{
		WCHAR	filePath[MAX_PATH];

		GetFilePath(filePath, MAX_PATH, FileName);

		/*if(!m_fileProxy->CheckFileAttributes(QString::fromWCharArray(filePath), FileAttributes))
		{
		return -1;
		}*/

		DbgPrint(L"SetFileAttributes %s\n", filePath);

		if (!SetFileAttributes(filePath, FileAttributes)) {
			DWORD error = GetLastError();
			DbgPrint(L"\terror code = %d\n\n", error);
			return error * -1;
		}

		DbgPrint(L"\n");
		return 0;
	}


	int __stdcall
		LVFSDriver::DokanSetFileTime(
		LPCWSTR				FileName,
		CONST FILETIME*		CreationTime,
		CONST FILETIME*		LastAccessTime,
		CONST FILETIME*		LastWriteTime,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		WCHAR	filePath[MAX_PATH];
		HANDLE	handle;

		GetFilePath(filePath, MAX_PATH, FileName);

		DbgPrint(L"SetFileTime %s\n", filePath);

		handle = (HANDLE)DokanFileInfo->Context;

		if (!handle || handle == INVALID_HANDLE_VALUE) {
			DbgPrint(L"\tinvalid handle\n\n");
			return -1;
		}

		if (!SetFileTime(handle, CreationTime, LastAccessTime, LastWriteTime)) {
			DWORD error = GetLastError();
			DbgPrint(L"\terror code = %d\n\n", error);
			return error * -1;
		}

		DbgPrint(L"\n");
		return 0;
	}


	int __stdcall
		LVFSDriver::DokanUnlockFile(
		LPCWSTR				FileName,
		LONGLONG			ByteOffset,
		LONGLONG			Length,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		WCHAR	filePath[MAX_PATH];
		HANDLE	handle;
		LARGE_INTEGER	length;
		LARGE_INTEGER	offset;

		GetFilePath(filePath, MAX_PATH, FileName);

		DbgPrint(L"UnlockFile %s\n", filePath);

		handle = (HANDLE)DokanFileInfo->Context;
		if (!handle || handle == INVALID_HANDLE_VALUE) {
			DbgPrint(L"\tinvalid handle\n\n");
			return -1;
		}

		length.QuadPart = Length;
		offset.QuadPart = ByteOffset;

		if (UnlockFile(handle, offset.HighPart, offset.LowPart, length.HighPart, length.LowPart)) {
			DbgPrint(L"\tsuccess\n\n");
			return 0;
		} else {
			DbgPrint(L"\tfail\n\n");
			return -1;
		}
	}


	int __stdcall
		LVFSDriver::DokanGetFileSecurity(
		LPCWSTR					FileName,
		PSECURITY_INFORMATION	SecurityInformation,
		PSECURITY_DESCRIPTOR	SecurityDescriptor,
		ULONG				BufferLength,
		PULONG				LengthNeeded,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		HANDLE	handle;
		WCHAR	filePath[MAX_PATH];

		GetFilePath(filePath, MAX_PATH, FileName);

		DbgPrint(L"GetFileSecurity %s\n", filePath);

		handle = (HANDLE)DokanFileInfo->Context;
		if (!handle || handle == INVALID_HANDLE_VALUE) {
			DbgPrint(L"\tinvalid handle\n\n");
			return -1;
		}

		if (!GetUserObjectSecurity(handle, SecurityInformation, SecurityDescriptor,
			BufferLength, LengthNeeded)) {
				int error = GetLastError();
				if (error == ERROR_INSUFFICIENT_BUFFER) {
					DbgPrint(L"  GetUserObjectSecurity failed: ERROR_INSUFFICIENT_BUFFER\n");
					return error * -1;
				} else {
					DbgPrint(L"  GetUserObjectSecurity failed: %d\n", error);
					return -1;
				}
		}
		return 0;
	}


	int __stdcall
		LVFSDriver::DokanSetFileSecurity(
		LPCWSTR					FileName,
		PSECURITY_INFORMATION	SecurityInformation,
		PSECURITY_DESCRIPTOR	SecurityDescriptor,
		ULONG				/*SecurityDescriptorLength*/,
		PDOKAN_FILE_INFO	DokanFileInfo)
	{
		HANDLE	handle;
		WCHAR	filePath[MAX_PATH];

		GetFilePath(filePath, MAX_PATH, FileName);

		DbgPrint(L"SetFileSecurity %s\n", filePath);

		handle = (HANDLE)DokanFileInfo->Context;
		if (!handle || handle == INVALID_HANDLE_VALUE) {
			DbgPrint(L"\tinvalid handle\n\n");
			return -1;
		}

		if (!SetUserObjectSecurity(handle, SecurityInformation, SecurityDescriptor)) {
			int error = GetLastError();
			DbgPrint(L"  SetUserObjectSecurity failed: %d\n", error);
			return -1;
		}
		return 0;
	}

	int __stdcall
		LVFSDriver::DokanGetVolumeInformation(
		LPWSTR		VolumeNameBuffer,
		DWORD		VolumeNameSize,
		LPDWORD		VolumeSerialNumber,
		LPDWORD		MaximumComponentLength,
		LPDWORD		FileSystemFlags,
		LPWSTR		FileSystemNameBuffer,
		DWORD		FileSystemNameSize,
		PDOKAN_FILE_INFO	/*DokanFileInfo*/)
	{
		wcscpy(VolumeNameBuffer, L"WebMounter");
		*VolumeSerialNumber = 0x19831116;
		*MaximumComponentLength = 256;
		*FileSystemFlags = FILE_CASE_SENSITIVE_SEARCH | 
			FILE_CASE_PRESERVED_NAMES | 
			FILE_SUPPORTS_REMOTE_STORAGE |
			FILE_UNICODE_ON_DISK |
			FILE_PERSISTENT_ACLS;

		wcscpy(FileSystemNameBuffer, L"WebMounter");

		return 0;
	}


	int __stdcall
		LVFSDriver::DokanWMUnmount(
		PDOKAN_FILE_INFO	/*DokanFileInfo*/)
	{
		DbgPrint(L"Unmount\n");
		return 0;
	}

	void LVFSDriver::run()
	{
		try
		{
			Ui::NotificationDevice* notificationDevice = Common::WebMounter::getNotificationDevice();

			/*notificationDevice->showNotification(Notification(
			Notification::eINFO
			, "Connecting..."
			, "Connecting to website... \n")
			);*/

			//m_fileProxy->Sync(QString(""), false);

			int status;

			LVFSDriver::m_driverOperations =
				(PDOKAN_OPERATIONS)malloc(sizeof(DOKAN_OPERATIONS));
			LVFSDriver::m_driverOptions =
				(PDOKAN_OPTIONS)malloc(sizeof(DOKAN_OPTIONS));

			ZeroMemory(m_driverOptions, sizeof(DOKAN_OPTIONS));
			//m_driverOptions->Version = DOKAN_VERSION;
			m_driverOptions->ThreadCount = 1; // use default

			//m_driverOptions->MountPoint =
			//	(WCHAR*)malloc(sizeof(WCHAR) * m_mountPoint.size() + 1);

			//wcscpy_s((wchar_t*)m_driverOptions->MountPoint, m_mountPoint.size() + 1, m_mountPoint.toStdWString().c_str());
			m_driverOptions->DriveLetter = m_mountPoint.toStdWString().c_str()[0];


			if (g_DebugMode) 
			{
				m_driverOptions->Options |= DOKAN_OPTION_DEBUG;
			}
			if (g_UseStdErr)
			{
				m_driverOptions->Options |= DOKAN_OPTION_STDERR;
			}

			m_driverOptions->Options |= DOKAN_OPTION_KEEP_ALIVE | DOKAN_OPTION_REMOVABLE;

			ZeroMemory(m_driverOperations, sizeof(DOKAN_OPERATIONS));

			m_driverOperations->CreateFile = LVFSDriver::DokanCreateFile;
			m_driverOperations->OpenDirectory = LVFSDriver::DokanOpenDirectory;
			m_driverOperations->CreateDirectory = LVFSDriver::DokanCreateDirectory;
			m_driverOperations->Cleanup = LVFSDriver::DokanCleanup;
			m_driverOperations->CloseFile = LVFSDriver::DokanCloseFile;
			m_driverOperations->ReadFile = LVFSDriver::DokanReadFile;
			m_driverOperations->WriteFile = LVFSDriver::DokanWriteFile;
			m_driverOperations->FlushFileBuffers = LVFSDriver::DokanFlushFileBuffers;
			m_driverOperations->GetFileInformation = LVFSDriver::DokanGetFileInformation;
			m_driverOperations->FindFiles = LVFSDriver::DokanFindFiles;
			m_driverOperations->FindFilesWithPattern = NULL;
			m_driverOperations->SetFileAttributes = LVFSDriver::DokanSetFileAttributes;
			m_driverOperations->SetFileTime = LVFSDriver::DokanSetFileTime;
			m_driverOperations->DeleteFile = LVFSDriver::DokanDeleteFile;
			m_driverOperations->DeleteDirectory = LVFSDriver::DokanDeleteDirectory;
			m_driverOperations->MoveFile = LVFSDriver::DokanMoveFile;
			m_driverOperations->SetEndOfFile = LVFSDriver::DokanSetEndOfFile;
			m_driverOperations->SetAllocationSize = LVFSDriver::DokanSetAllocationSize;	
			m_driverOperations->LockFile = LVFSDriver::DokanLockFile;
			m_driverOperations->UnlockFile = LVFSDriver::DokanUnlockFile;
			//m_driverOperations->GetFileSecurity = LVFSDriver::DokanGetFileSecurity;
			//m_driverOperations->SetFileSecurity = LVFSDriver::DokanSetFileSecurity;
			m_driverOperations->GetDiskFreeSpace = NULL;
			m_driverOperations->GetVolumeInformation = LVFSDriver::DokanGetVolumeInformation;
			m_driverOperations->Unmount = LVFSDriver::DokanWMUnmount;

			emit mounted();

			if(notificationDevice)
			{
				Notification msg(Notification::eINFO, tr("Info"), tr("Disk is mounted\n"));
				notificationDevice->showNotification(msg);
			}

			status = DokanMain(m_driverOptions, m_driverOperations);

			emit unmounted();

			switch (status) 
			{
			case DOKAN_ERROR:
				{
					Notification msg(Notification::eCRITICAL, tr("Error"), tr("Error\n"));
					notificationDevice->showNotification(msg);
					DbgPrint(L"Error\n");
					break;
				}
			case DOKAN_DRIVE_LETTER_ERROR:
				{
					Notification msg(Notification::eCRITICAL, tr("Error"), tr("Bad Drive letter\n"));
					//emit showDriverMessage(msg);
					notificationDevice->showNotification(msg);
					DbgPrint(L"Bad Drive letter\n");
					break;
				}
			case DOKAN_DRIVER_INSTALL_ERROR:
				{
					Notification msg(Notification::eCRITICAL, tr("Error"), tr("Can't install driver\n"));
					notificationDevice->showNotification(msg);
					DbgPrint(L"Can't install driver\n");
					break;
				}
			case DOKAN_START_ERROR:
				{
					Notification msg(Notification::eCRITICAL, tr("Error"), tr("Driver something wrong\n"));
					notificationDevice->showNotification(msg);
					DbgPrint(L"Driver something wrong\n");
					break;
				}
			case DOKAN_MOUNT_ERROR:
				{
					Notification msg(Notification::eCRITICAL, tr("Error"), tr("Can't assign a drive letter\n"));
					notificationDevice->showNotification(msg);
					DbgPrint(L"Can't assign a drive letter\n");
					break;
				}
				/*case DOKAN_MOUNT_POINT_ERROR:
				{
				Notification msg(Notification::eCRITICAL, tr("Error"), tr("Mount point error\n"));
				notificationDevice->showNotification(msg);
				DbgPrint(L"Mount point error\n");
				break;
				}*/
			case DOKAN_SUCCESS:
				{
					Notification msg(Notification::eINFO, tr("Info"), tr("Disk is unmounted\n"));
					notificationDevice->showNotification(msg);
					DbgPrint(L"Disk is unmounted\n");
					break;
				}
			default:
				{
					Notification msg(Notification::eCRITICAL, tr("Error"), tr("Unknown error\n"));
					notificationDevice->showNotification(msg);
					DbgPrint(L"Unknown error: %d\n", status);
					break;
				}
			}

			free(m_driverOptions);
			free(m_driverOperations);
		}
		catch(...)
		{
		}
	}

	LVFSDriver* LVFSDriver::createDriver(FileProxy * pProxy)
	{
		QMutexLocker locker(&m_driverMutex);
        if(!m_driverInstance)
		{
            m_driverInstance = new LVFSDriver(pProxy);
		}
        return m_driverInstance;
	}

	void LVFSDriver::startDriver()
	{
        if(m_driverInstance)
		{
            m_driverInstance->start();
		}
	}
	void LVFSDriver::stopDriver()
	{
        if(m_driverInstance->isRunning())
		{
			DokanUnmount(m_mountPoint.toStdWString().c_str()[0]);
		}
	}

};