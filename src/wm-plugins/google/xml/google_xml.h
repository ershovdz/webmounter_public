#ifndef GOOGLE_XML_H
#define GOOGLE_XML_H

//#include "common_stuff.h"
#include "vfs_cache.h"

#include <iostream>
#include <map>
#include <QString>
#include <libxml++/libxml++.h>

namespace Xml
{
	using std::map;
	using xmlpp::Node;
	using xmlpp::NodeSet;
	using xmlpp::Element;
	using Data::VFSElement;

	class GoogleAtomParser 
	{
	public:
		GoogleAtomParser(const QString& pluginName);
		~GoogleAtomParser() {}

		QString NextLinkHref() const;
		RESULT checkExtension(const QString& file_extension) const;
		RESULT parseDocList(const QString& xml_doc_list, QList<VFSElement>& elements);
		RESULT parseEntry(const QString& xml_entry, VFSElement& element);
	
	private:
		RESULT parseAlbumEntry(const xmlpp::Node* singleEntry, VFSElement& element);
		RESULT parseDocEntry(const xmlpp::Node* singleEntry, VFSElement& element);
		QString exportFormat(const Node* entry) const;
		RESULT checkExtension(const Node* entry) const;
		QString getElementName(const Node* entry);
		QString getElementPath(int index);
		static QString find_and_replace(QString source,
                                const QString& find,
                                const QString& replace);

		void Parse(const QString& xml_str) 
		{ 
			parser_.parse_memory(xml_str.toUtf8().constData()); 
		}

		void RegisterNamespaces(const map<QString, QString>& namespaces);

		NodeSet Find(const Node *node, QString xpath) const 
		{
			return node->find(xpath.toStdString(), namespaces_);
		}
		const Element* Link(const xmlpp::Node *entry, QString rel) const;
		QString Attribute(const Element *from_element, QString attr_name) const;
		const Node* Entry() const;
		NodeSet Entries() const;
		QString AlternateLinkHref(const Node *entry) const;
		QString CategoryLabel(const Node *entry) const;
		QString AclRole(const Node *entry) const;
		QString AclScope(const Node *entry) const;
		QString ParentId(const Node *entry) const; 
		QString ContentSrc(const Node *entry) const;
		QString EditLinkHref(const Node *entry) const;
		QString EditMediaLinkHref(const Node *entry) const;
		QString ETag(const Node *entry) const;
		QString FeedLinkHref(const Node *entry) const;
		QString Id(const Node *entry) const;
		QString Title(const Node *entry) const;
		QString getFolderPath(int index);
	
	private:
		xmlpp::DomParser parser_;
		Node::PrefixNsMap namespaces_;
		const QString _pluginName;
	};
} 
#endif  // GOOGLE_XML_H
