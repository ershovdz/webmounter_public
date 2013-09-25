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

#ifndef COMMON_STUFF_H
#define COMMON_STUFF_H

#include <qglobal.h>

#define VERSION "0.4.0"

#ifdef Q_OS_WIN
#define LVFS_DRIVER_H "win_lvfs_driver.h"
#endif
#ifdef Q_OS_LINUX
#define LVFS_DRIVER_H "linux_lvfs_driver.h"
#endif

#define WM_FULL_VERSION

#if defined(WEBMOUNTER_LIBRARY)
#  define WEBMOUNTER_EXPORT Q_DECL_EXPORT
#else
#  define WEBMOUNTER_EXPORT Q_DECL_IMPORT
#endif

namespace Common
{
	enum RESULT
	{
		eNO_ERROR,
		eERROR_GENERAL,
		eERROR_CANCEL,
		eERROR_NOT_SUPPORTED,
		eERROR_FILE_SYSTEM,
		eERROR_WRONG_STATE
	};
};

#define LOCK(mutex) QMutexLocker locker(&mutex);

#include <QString>

#define IPP_VERSION

#define WmInterface struct

#endif
