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

#include "filesystemtools.h"

using namespace Common;

RESULT FileSystemTools::removeFolder( QDir& dir )
{
	if ( eNO_ERROR != removeFolderContent( dir ) )
	{
		return eERROR_FILE_SYSTEM;
	}
	else if ( !QDir().rmdir(dir.absolutePath()) )
	{
		return eERROR_FILE_SYSTEM;
	}
	return eNO_ERROR;
}

RESULT FileSystemTools::removeFolderContent( QDir& dir )
{
	RESULT res = eNO_ERROR;

	QStringList lstDirs  = dir.entryList(QDir::Dirs | QDir::AllDirs | QDir::NoDotAndDotDot);
	QStringList lstFiles = dir.entryList(QDir::Files);

	foreach (QString entry, lstFiles)
	{
		QString entryAbsPath = dir.absolutePath() + "/" + entry;

		QFile::Permissions permissions = QFile::permissions(entryAbsPath);
		permissions |= (QFile::WriteGroup|QFile::WriteOwner|QFile::WriteUser|QFile::WriteOther);
		QFile::setPermissions(entryAbsPath, permissions);

		QFile::remove(entryAbsPath);
	}

	foreach (QString entry, lstDirs)
	{
		QString entryAbsPath = dir.absolutePath() + "/" + entry;
		QDir internalDir(entryAbsPath);

		if ( eNO_ERROR != removeFolderContent( internalDir )  )
		{
			res = eERROR_FILE_SYSTEM;
		}
		else if( !QDir().rmdir( internalDir.absolutePath() ))
		{
			res = eERROR_FILE_SYSTEM;
		}
	}

	return res;
}