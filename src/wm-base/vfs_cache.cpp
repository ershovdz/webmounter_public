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

#include "vfs_cache.h"
#include <QtCore/qfile.h>

#include <vector>
#include <QString>
#include <QVariant>

#include "webmounter.h"

using std::vector;

namespace Data
{
	using namespace Common;

	QSqlDatabase VFSCache::m_db;
	VFSCache* VFSCache::m_vfsCacheInstance = 0;
	QMutex VFSCache::m_vfsCacheMutex;
	QString VFSCache::m_localDBPath;

	VFSCache::VFSCache(void)
	{
	}

	VFSCache::VFSCache(const QString& localDBPath)
	{
		m_localDBPath = localDBPath;

		m_db = QSqlDatabase::addDatabase("QSQLITE", "webmounter");
		m_db.setDatabaseName(localDBPath + QDir::separator() + "webmounter.db");

		if(!QFile::exists(localDBPath + QDir::separator() + "webmounter.db")) // Database does not exist yet
		{
			m_db.open(); // Open db and create db file.
			initDB(); // Populate database with plugin tables
		}
		else // Database exists. Just open it
		{
			m_db.open(); 
		}
	}

	VFSCache::~VFSCache(void)
	{
		m_db.close();
		m_db.removeDatabase("webmounter");
	}

	RESULT VFSCache::initDB()
	{
		QSqlQuery query(m_db);

		if(m_db.isOpen())
		{
			query.exec("PRAGMA synchronous=OFF");

			query.exec("DROP table Elements");

			bool isOk = query.exec("create table Elements (path varchar(256),"
				"elemid varchar(256),"
				"parentid varchar(256),"
				"name varchar(256),"
				"type int,"
				"flags int,"
				"modified varchar(256),"
				"editMetaUrl varchar(1024),"
				"editMediaUrl varchar(1024),"
				"srcUrl varchar(1024),"
				"pluginName varchar(256))");

			if(!isOk)
			{
				return eERROR_GENERAL;
			}

			return eNO_ERROR;
		}
		return eERROR_GENERAL;
	}

	VFSCache* VFSCache::getCache(const QString& localDBPath)
	{
		if(m_vfsCacheInstance)
		{
			if(m_db.isOpen())
			{
				return m_vfsCacheInstance;
			}
			else
			{
				bool isOk = m_db.open();
				if(isOk)
				{
					return m_vfsCacheInstance;
				}
				else
				{
					delete m_vfsCacheInstance;
					m_vfsCacheInstance = 0;
					return 0;
				}
			}
		}
		else
		{
			m_vfsCacheInstance = new VFSCache(localDBPath);
			if(m_db.isOpen())
			{
				if(m_vfsCacheInstance->restoreCache() == eNO_ERROR)
				{
					return m_vfsCacheInstance;
				}
				else
				{
					m_vfsCacheInstance->initDB();
					return m_vfsCacheInstance;
				}
			}
			else
			{
				delete m_vfsCacheInstance;
				m_vfsCacheInstance = 0;
				return 0;
			}
		}
	}

	void VFSCache::insert(VFSElement& elem, bool dirty, bool updatedb)
	{
		VFSCache::iterator iter = m_fileList.find(elem.getPath());

		if(iter != m_fileList.end()) // element exists
		{
			if(iter != elem ) // element not equal to existing one
			{
				iter.getNatureIter()->second->setFlags(VFSElement::eFl_Dirty);
				iter.getNatureIter()->second->setDownloaded(elem.isDownloaded());
				iter.getNatureIter()->second->setId(elem.getId());
				iter.getNatureIter()->second->setModified(elem.getModified());
				iter.getNatureIter()->second->setSrcUrl(elem.getSrcUrl());
				iter.getNatureIter()->second->setEditMetaUrl(elem.getEditMetaUrl());
				iter.getNatureIter()->second->setEditMediaUrl(elem.getEditMediaUrl());
				iter.getNatureIter()->second->setName(elem.getName());
				iter.getNatureIter()->second->setParentId(elem.getParentId());
				iter.getNatureIter()->second->setType(elem.getType());
				iter.getNatureIter()->second->setPath(elem.getPath());
				iter.getNatureIter()->second->setPluginName(elem.getPluginName());

				if(updatedb)
				{
					flush();
				}
			}
			else if(dirty) // element equal to existing one. Let's mark as dirty
			{
				iter.getNatureIter()->second->setDirty(true);
			}
		}
		else // element does not exist or moved. Let's insert it to DB and to list
		{
			for(iterator iter = this->begin(); iter != this->end(); ++iter) 
			{
				if(iter == elem) // HACK!! Identical elements, except file extension.
				{
					elem.setDownloaded(iter->isDownloaded());
					elem.setName(iter->getName());
					elem.setPath(iter->getPath());

					erase(iter);

					elem.setDirty(dirty);

					VFSElementPtr elemPtr (new VFSElement(elem)); 

                    m_fileList.insert(VFSFileList_Pair(elem.getPath(), elemPtr));

					if(updatedb)
					{
						flush();
					}
					return;
				}

				if(iter->getId() == elem.getId() 
					&& iter->getType() == elem.getType() 
					&& iter->getPluginName() == elem.getPluginName()) // element moved or renamed
				{
					//iter.getNatureIter()->second->setPath(elem.getPath());
					//iter.getNatureIter()->second->setDirty(dirty);
					//iter.getNatureIter()->first. = elem.getPath();
					elem.setDownloaded(iter->isDownloaded());
					erase(iter);

					elem.setDirty(dirty);

					VFSElementPtr elemPtr (new VFSElement(elem)); 

					m_fileList.insert(VFSFileList_Pair(elem.getPath(), elemPtr));

					if(updatedb)
					{
						flush();
					}
					return;
				}

			}

			elem.setDirty(dirty);

			VFSElementPtr elemPtr (new VFSElement(elem)); 

			m_fileList.insert(VFSFileList_Pair(elem.getPath(), elemPtr));

			//if(updatedb)
			//{
			QSqlQuery query(m_db);

			query.prepare("INSERT INTO Elements(path, elemid, parentid, name, type, flags, modified, editMetaUrl, editMetaUrl, srcUrl, pluginName) "
				"VALUES(:path_elem, :elemid,"
				":parentid,"
				":name,"
				":type,"
				":flags,"
				":modified,"
				":editMetaUrl,"
				":editMediaUrl,"
				":srcUrl,"
				":pluginName)");

			query.bindValue(":path_elem", QVariant(elem.getPath()));
			query.bindValue(":elemid", QVariant(elem.getId()));
			query.bindValue(":parentid", QVariant(elem.getParentId()));
			query.bindValue(":name", QVariant(elem.getName()));
			query.bindValue(":type", QVariant(elem.getType()));
			query.bindValue(":flags", QVariant(elem.getFlags()));
			query.bindValue(":modified", QVariant(elem.getModified()));
			query.bindValue(":editMetaUrl", QVariant(elem.getEditMetaUrl()));
			query.bindValue(":editMediaUrl", QVariant(elem.getEditMediaUrl()));
			query.bindValue(":srcUrl", QVariant(elem.getSrcUrl()));
			query.bindValue(":pluginName", QVariant(elem.getPluginName()));

			query.exec();
			//}
		}
	}

	void VFSCache::setFlag(iterator& iter, uint set, uint unset, bool updateDB)
	{
		iter.getNatureIter()->second->setDirty(true);
		iter.getNatureIter()->second->setFlags(set, unset);
		if(updateDB) 
			flush();
	}

	RESULT VFSCache::erasePlugin(const QString& pluginName)
	{
		for(VFSCache::iterator iter = m_fileList.begin(); 
			iter != m_fileList.end() && iter->getPluginName() == pluginName;
			)
		{
			VFSCache::iterator iter1 = iter++;
			erase(iter1);
		}
		return eNO_ERROR;
	}

	RESULT VFSCache::erase(iterator& iter)
	{
		QSqlQuery query(m_db);

		if(iter != m_fileList.end())
		{
			query.prepare("DELETE FROM Elements WHERE elemid=:id");
			query.bindValue(":id", iter->getId());

			if(!query.exec())
			{
				return eERROR_GENERAL;
			}

			m_fileList.erase(iter);
			return eNO_ERROR;
		}
		return eNO_ERROR;
	}

	RESULT VFSCache::clean()
	{
		initDB();
		m_fileList.clear();
		return eNO_ERROR;
	}

	RESULT VFSCache::restoreCache()
	{
		//QMutexLocker locker(&_VFSCacheMutex);

		try
		{
			m_fileList.clear();

			QSqlQuery query(m_db);

			bool isOk = query.exec("SELECT path, elemid, parentid, name, type, editMetaUrl, editMediaUrl, srcUrl, flags, modified, pluginName FROM Elements WHERE 1");

			if(!isOk)
			{
				return eERROR_GENERAL;
			}

			while (query.next()) 
			{
				QString path = query.value(0).toString();
				QString elemid = query.value(1).toString()/*.toInt()*/;
				QString parentid = query.value(2).toString()/*.toInt()*/;
				QString name = query.value(3).toString();
				VFSElement::VFSElementType type = (VFSElement::VFSElementType)query.value(4).toInt();
				QString editMetaUrl = query.value(5).toString();
				QString editMediaUrl = query.value(6).toString();
				QString srcUrl = query.value(7).toString();
				int flags = query.value(8).toInt();
				QString modified = query.value(9).toString();
				QString pluginName = query.value(10).toString();

				flags &= ~VFSElement::eFl_Dirty; // we get it from database, thus it is not dirty
				VFSElement elem(VFSElement(type
					, path
					, name
					, editMetaUrl
					, editMediaUrl
					, srcUrl
					, elemid
					, parentid
					, modified
					, pluginName
					, flags));

				VFSElementPtr elemPtr (new VFSElement(elem)); 

				m_fileList.insert(VFSFileList_Pair(elem.getPath(), elemPtr));
			}
			return eNO_ERROR;
		}
		catch(...)
		{
		}
		return eERROR_GENERAL;
	}

	RESULT VFSCache::flush()
	{
		//QMutexLocker locker(&_VFSCacheMutex);
		QSqlQuery query(m_db);

		for(iterator iter = m_fileList.begin(); iter != m_fileList.end(); iter++)
		{
			if(iter->isDirty())
			{
				query.prepare("UPDATE Elements SET "
					"elemid=:elemid,"
					"parentid=:parentid,"
					"name=:name,"
					"type=:type,"
					"flags=:flags,"
					"modified=:modified,"
					"editMetaUrl=:editMetaUrl,"
					"editMediaUrl=:editMediaUrl,"
					"srcUrl=:srcUrl,"
					"pluginName=:pluginName WHERE path=:path_elem");

				query.bindValue(":path_elem", QVariant(iter->getPath()));
				query.bindValue(":elemid", QVariant(iter->getId()));
				query.bindValue(":parentid", QVariant(iter->getParentId()));
				query.bindValue(":name", QVariant(iter->getName()));
				query.bindValue(":type", QVariant(iter->getType()));
				query.bindValue(":flags", QVariant(iter->getFlags()));
				query.bindValue(":modified", QVariant(iter->getModified()));
				query.bindValue(":editMetaUrl", QVariant(iter->getEditMetaUrl()));
				query.bindValue(":editMediaUrl", QVariant(iter->getEditMediaUrl()));
				query.bindValue(":srcUrl", QVariant(iter->getSrcUrl()));
				query.bindValue(":pluginName", QVariant(iter->getPluginName()));

				if(!query.exec())
				{
					return eERROR_GENERAL;
				}
				iter.getNatureIter()->second->setDirty(false);
			}
		}
		return eNO_ERROR;
	}
}
