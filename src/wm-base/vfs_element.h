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


#ifndef VFS_ELEMENT_H
#define VFS_ELEMENT_H

#include <QString>
#include "common_stuff.h"

namespace Data
{
#define ROOT_ID "0"

	class WEBMOUNTER_EXPORT VFSElement
	{
	public:
		enum VFSElementType {UNKNOWN, DIRECTORY, FILE};
		enum eVFSFlags
		{
			eFl_None		= 0x0,
			eFl_Downloaded	= 0x1,
			eFl_Dirty		= 0x2,
			eFl_SelfMade	= 0x4,
			eFl_NameDup		= 0x8,
			eFl_Downloading	= 0x10,

			eFl_All			= 0xffffff
		};

		VFSElement(void);
		VFSElement(const VFSElement& elem);
		VFSElement& operator=(const VFSElement& elem);
		virtual ~VFSElement(void);
		VFSElement(VFSElementType type
			, const QString& path
			, const QString& name
			, const QString& editMetaURL
			, const QString& editMediaURL
			, const QString& srcURL
			, const QString& id
			, const QString& parent_id
			, const QString& modified
			, const QString& pluginName
			, int flags = eFl_None);

	public:
		bool operator==(const VFSElement& elem);
		bool operator!=(const VFSElement& elem);
		VFSElementType getType() const;
		const QString& getId() const;
		const QString& getParentId() const;
		const QString& getPath() const;
		const QString& getName() const;
		const QString& getEditMetaUrl() const;
		const QString& getEditMediaUrl() const;
		const QString& getSrcUrl() const;
		const QString& getPluginName() const;
		uint getFlags() const;
		bool isDirty() const;
		bool isDownloaded() const;
		const QString& getModified() const;

		void setDirty(bool);
		void setDownloaded(bool);
		void setModified(const QString&);
		void setParentId(const QString&);
		void setId(const QString&);
		void setType(VFSElementType);
		void setPath(const QString&);
		void setName(const QString&);
		void setEditMediaUrl(const QString&);
		void setEditMetaUrl(const QString&);
		void setSrcUrl(const QString&);
		void setPluginName(const QString& pluginName);
		void setFlags(uint set, uint unset = eFl_None);

		void reset();

	private:
		VFSElementType m_type;
		uint m_flags;
		QString m_path;
		QString m_name;

		QString m_id;
		QString m_parentID;

		QString m_editMetaUrl;
		QString m_editMediaUrl;
		QString m_srcUrl;

		QString m_modified;
		QString m_pluginName;
	};
}

#endif //VFS_ELEMENT_H
