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

#include "vfs_element.h"
#include <QFileInfo>

using namespace Data;

VFSElement::VFSElement(void) : m_flags(eFl_None)
{
}

VFSElement::~VFSElement(void)
{
}

VFSElement& VFSElement::operator=(const VFSElement& elem)
{
	this->setFlags(elem.getFlags());
	this->setDirty(elem.isDirty());
	this->setDownloaded(elem.isDownloaded());
	this->setId(elem.getId());
	this->setModified(elem.getModified());
	this->setSrcUrl(elem.getSrcUrl());
	this->setEditMetaUrl(elem.getEditMetaUrl());
	this->setEditMediaUrl(elem.getEditMediaUrl());
	this->setName(elem.getName());
	this->setParentId(elem.getParentId());
	this->setType(elem.getType());
	this->setPath(elem.getPath());
	this->setPluginName(elem.getPluginName());

	return *this;
}

bool VFSElement::operator==(const VFSElement& elem)
{
	bool equal = (this->getPluginName() == elem.getPluginName()
		&& this->getParentId() == elem.getParentId()
		//&& this->getOrigUrl() == elem.getOrigUrl()
		//&& this->getSmallUrl() == elem.getSmallUrl()
		&& this->getType() == elem.getType()
		&& this->getId() == elem.getId()
		&& this->getModified() == elem.getModified()
		/*&& this->getFlags() == elem.getFlags()*/
		);

	if(equal)
	{
		QFileInfo fCurrent(this->getPath());
		QFileInfo fNew(elem.getPath());

		QString path = fCurrent.path();
		QString path2 = fNew.path();
		QString name =  fCurrent.baseName();
		QString name2 = fNew.baseName();

		if(fCurrent.path() == fNew.path()
			&& fCurrent.baseName() == fNew.baseName())
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	return equal;
}

bool VFSElement::operator!=(const VFSElement& elem)
{
	if(*this == elem)
	{
		return false;
	}
	else
	{
		return true;
	}
}

VFSElement::VFSElement(VFSElementType type
	, const QString &path
	, const QString &name
	, const QString &editMetaUrl
	, const QString &editMediaUrl
	, const QString &srcUrl
	, const QString& id
	, const QString& parent_id
	, const QString& modified
	, const QString& pluginName
	, int flags
	) 

    : m_type(type)
    , m_flags(flags)
    , m_id(id)
	, m_parentID(parent_id)
    , m_path(path)
    , m_name(name)
	, m_srcUrl(srcUrl)
	, m_editMetaUrl(editMetaUrl)
	, m_editMediaUrl(editMediaUrl)
	, m_pluginName(pluginName)
    , m_modified(modified)
{
}

VFSElement::VFSElement(const VFSElement& elem) 
    : m_type(elem.getType())
    , m_flags(eFl_None)
    , m_id(elem.getId())
	, m_parentID(elem.getParentId())
    , m_path(elem.getPath())
    , m_name(elem.getName())
	, m_srcUrl(elem.getSrcUrl())
	, m_editMetaUrl(elem.getEditMetaUrl())
	, m_editMediaUrl(elem.getEditMediaUrl())
	, m_pluginName(elem.getPluginName())
    , m_modified(elem.getModified())
{
	setFlags(elem.getFlags());
};

uint VFSElement::getFlags() const
{
    return m_flags;
}

bool VFSElement::isDirty() const
{
    return m_flags&eFl_Dirty;
}

bool VFSElement::isDownloaded() const
{
    return m_flags&eFl_Downloaded;
}

const QString& VFSElement::getModified() const
{
    return this->m_modified;
}

const QString& VFSElement::getPath() const
{
    return this->m_path;
}

const QString& VFSElement::getName() const
{
    return this->m_name;
}

const QString& VFSElement::getEditMetaUrl() const
{
	return this->m_editMetaUrl;
}

const QString& VFSElement::getEditMediaUrl() const
{
	return this->m_editMediaUrl;
}

const QString& VFSElement::getSrcUrl() const
{
	return this->m_srcUrl;
}

const QString& VFSElement::getParentId(void) const
{
	return this->m_parentID;
}

const QString& VFSElement::getId(void) const
{
    return this->m_id;
}

VFSElement::VFSElementType VFSElement::getType(void) const
{
    return this->m_type;
}

const QString& VFSElement::getPluginName() const
{
	return this->m_pluginName;
}

void VFSElement::setPath(const QString& path)
{
    m_path = path;
}

void VFSElement::setName(const QString& name)
{
    m_name = name;
}

void VFSElement::setEditMetaUrl(const QString& editMetaUrl)
{
	m_editMetaUrl = editMetaUrl;
}

void VFSElement::setSrcUrl(const QString& srcUrl)
{
	m_srcUrl = srcUrl;
}

void VFSElement::setEditMediaUrl(const QString& editMediaUrl)
{
	m_editMediaUrl = editMediaUrl;
}

void VFSElement::setParentId(const QString& parentId)
{
	this->m_parentID = parentId;
}

void VFSElement::setId(const QString& id)
{
    this->m_id = id;
}

void VFSElement::setType(VFSElement::VFSElementType type)
{
    this->m_type = type;
}

void VFSElement::setDirty(bool dirty)
{
	uint set = dirty ? eFl_Dirty : eFl_None;
	uint unset = !dirty ? eFl_Dirty : eFl_None;
	setFlags(set, unset);
}

void VFSElement::setDownloaded(bool downloaded)
{
	uint set = downloaded ? eFl_Downloaded : eFl_None;
	uint unset = !downloaded ? eFl_Downloaded : eFl_None;
	setFlags(set, unset);
}

void VFSElement::setModified(const QString& date)
{
    this->m_modified = date;
}

void VFSElement::setPluginName(const QString& pluginName)
{
	this->m_pluginName = pluginName;
}

void VFSElement::setFlags(uint set, uint unset)
{
    m_flags |= set;
    m_flags &= ~unset;
}

void VFSElement::reset()
{
    m_flags = eFl_None;
	this->setDirty(true);
	this->setDownloaded(false);
	this->setId(0);
	this->setModified("");
	this->setSrcUrl("");
	this->setEditMetaUrl("");
	this->setEditMediaUrl("");
	this->setName("");
	this->setParentId(0);
	this->setType(UNKNOWN);
	this->setPath("");
	this->setPluginName("");
}