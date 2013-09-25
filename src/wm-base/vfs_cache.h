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

#ifndef VFS_CACHE_H
#define VFS_CACHE_H

#include "common_stuff.h"

#include <map>
#include <QString>
#include "vfs_element.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

#include <QMutex>
#include <QMutexLocker>
#include "boost/shared_ptr.hpp"

using Common::RESULT;

namespace Data
{
	typedef boost::shared_ptr<VFSElement> VFSElementPtr;
	typedef std::map<QString, VFSElementPtr> VFSFileList;
	typedef VFSElement::VFSElementType ElementType;
	typedef std::pair <QString, VFSElementPtr> VFSFileList_Pair;

	class WEBMOUNTER_EXPORT VFSCache_Iter
	{
		friend class VFSCache;

	public:
		typedef VFSFileList::iterator::iterator_category iterator_category;
		typedef VFSFileList::iterator::value_type value_type;
		typedef VFSFileList::iterator::difference_type difference_type;
		typedef difference_type distance_type;
		typedef VFSFileList::iterator::pointer pointer;
		typedef VFSFileList::iterator::reference reference;

	public:
		VFSCache_Iter operator++(int) // postfix form
		{
			VFSCache_Iter iter = m_iter;
			m_iter++;
			return iter;
		}

		VFSCache_Iter operator--(int) // postfix form
		{
			VFSCache_Iter iter = m_iter;
			m_iter--;
			return iter;
		}

		VFSCache_Iter& operator++() // prefix form
		{
			m_iter++;
			return *this;
		}

		VFSCache_Iter& operator--() // prefix form
		{
			m_iter--;
			return *this;
		}

		operator VFSFileList::iterator()
		{
			return m_iter;
		}

		VFSCache_Iter& operator*()
		{
			return *this;
		}

		bool operator!=(VFSFileList::iterator iter)
		{
			return (m_iter != iter);
		}

		bool operator==(VFSFileList::iterator iter)
		{
			return (m_iter == iter);
		}

		bool operator!=(VFSElement& elem)
		{
			return (*(m_iter->second.get()) != elem);
		}

		bool operator==(VFSElement& elem)
		{
			return (*(m_iter->second.get()) == elem);
		}

		const VFSElement* operator->()
		{
			return m_iter->second.get();
		}

	protected:

		VFSCache_Iter& operator=(VFSFileList::iterator iter)
		{
			m_iter = iter;
			return *this;
		}

		VFSCache_Iter& operator=(VFSElement elem)
		{
			*(m_iter->second.get()) = elem;
			return *this;
		}

		VFSCache_Iter(VFSFileList::iterator iter)
		{
			m_iter = iter;
		}
		VFSCache_Iter()
		{	
		}

	protected:
		VFSFileList::iterator getNatureIter()
		{
			return m_iter;
		}

	private:
		VFSFileList::iterator m_iter;
	};

	class WEBMOUNTER_EXPORT VFSCache
	{
	protected:
		VFSCache(void);
		VFSCache(const QString& localDBPath);

	public:
		~VFSCache(void);

	public:
		typedef VFSCache_Iter iterator;

		static VFSCache* getCache(const QString& localDBPath = "");
		RESULT clean();
		RESULT restoreCache();
		void insert(VFSElement& elem, bool dirty = false, bool db = true);
		RESULT erase(iterator& elem);
		RESULT erasePlugin(const QString& pluginName);
		RESULT flush();

		iterator find(const QString& key)
		{
			return m_fileList.find(key);
		}

		iterator begin()
		{
			return m_fileList.begin();
		}
		iterator end()
		{
			return m_fileList.end();
		}

		void setFlag(iterator& iter, uint set, uint unset = VFSElement::eFl_None, bool updateDB = true);

	private:
		RESULT initDB();
	private:
		VFSFileList m_fileList;
		static QSqlDatabase m_db;
		static VFSCache* m_vfsCacheInstance;
		static QMutex m_vfsCacheMutex;
		static QString m_localDBPath;
	};
}

#endif