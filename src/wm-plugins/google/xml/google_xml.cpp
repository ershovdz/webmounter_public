#include "google_xml.h"
#include "webmounter.h"
#include <QFile>
#include <glibmm/convert.h>

namespace Xml
{
	using xmlpp::TextNode;
	using xmlpp::Attribute;

	GoogleAtomParser::GoogleAtomParser(const QString& pluginName) : _pluginName(pluginName) 
	{
		// register namespaces used by the DocList APIs
		map<QString, QString> namespaces;
		namespaces["gAcl"] = "http://schemas.google.com/acl/2007";
		namespaces["gd"] = "http://schemas.google.com/g/2005";
		namespaces_["atom"] = "http://www.w3.org/2005/Atom";
		namespaces_["openSearch"] = "http://a9.com/-/spec/opensearch/1.1/";
		RegisterNamespaces(namespaces);
	}

	void GoogleAtomParser::RegisterNamespaces(const map<QString, QString>& namespaces) 
	{
		map<QString, QString>::const_iterator iter;
		for (iter = namespaces.begin(); iter != namespaces.end(); ++iter) 
		{
			namespaces_[iter->first.toStdString()] = iter->second.toStdString();
		}
	}

	const Node* GoogleAtomParser::Entry() const 
	{
		const Node *node = NULL;

		if (parser_) 
		{
			node = parser_.get_document()->get_root_node();
		}
		return node;
	}

	NodeSet GoogleAtomParser::Entries() const 
	{
		NodeSet entries;
		if (parser_) 
		{
			entries = Find(parser_.get_document()->get_root_node(), "./atom:entry");
		}
		return entries;
	}

	QString GoogleAtomParser::AlternateLinkHref(const Node *entry) const 
	{
		if (!parser_) return "";
		return Attribute(Link(entry, "alternate"), "href");
	}

	QString GoogleAtomParser::CategoryLabel(const Node *entry) const 
	{
		if (!parser_) return "";

		QString label;

		NodeSet categories = Find(entry, "./atom:category");
		for (unsigned int i = 0; i < categories.size(); ++i) 
		{
			const Element *nodeElement = dynamic_cast<const Element*>(categories[i]);
			const Element::AttributeList& attributes =
				nodeElement->get_attributes();

			Element::AttributeList::const_iterator iter;
			for (iter = attributes.begin(); iter != attributes.end(); ++iter) 
			{
				const xmlpp::Attribute *attribute = *iter;
				if (attribute->get_name() == "scheme" &&
					attribute->get_value() == "http://schemas.google.com/g/2005#kind") 
				{
					label = Attribute(nodeElement, "label");
					break;
				}
			}
		}

		if(label == "document"
			|| label == "presentation"
			|| label == "spreadsheet"
			|| label == "pdf"
			|| label == "folder")
		{
			return label;
		}
		
		return "";
	}

	QString GoogleAtomParser::AclRole(const Node *entry) const 
	{
		if (!parser_) return "";
		NodeSet roles = Find(entry, "./gAcl:role");
		if (roles.size()) 
		{
			return Attribute(dynamic_cast<const Element*>(roles[0]), "value");
		} 
		else 
		{
			return "";
		}
	}

	QString GoogleAtomParser::AclScope(const Node *entry) const 
	{
		if (!parser_) return "";
		NodeSet scopes = Find(entry, "./gAcl:scope");
		if (scopes.size()) 
		{
			return Attribute(dynamic_cast<const Element*>(scopes[0]), "value");
		} 
		else 
		{
			return "";
		}
	}

	QString GoogleAtomParser::ContentSrc(const Node *entry) const 
	{
		if (!parser_) return "";
		NodeSet contents = Find(entry, "./atom:content");
		if (contents.size()) 
		{
			return Attribute(dynamic_cast<const Element*>(contents[0]), "src");
		} 
		else 
		{
			return "";
		}
	}

	QString GoogleAtomParser::EditLinkHref(const Node *entry) const 
	{
		if (!parser_) return "";
		return Attribute(Link(entry, "edit"), "href");
	}

	QString GoogleAtomParser::ParentId(const Node *entry) const 
	{
		if (!parser_) return ROOT_ID;
		QString id = Attribute(Link(entry, "http://schemas.google.com/docs/2007#parent"), "href");
		if(id == "")
		{
			return ROOT_ID;
		}
		
		return id.mid(id.lastIndexOf("/") + 1);
	}
	
	QString GoogleAtomParser::NextLinkHref() const
	{
		if (!parser_) return ROOT_ID;
		return Attribute(Link(Entry(), "next"), "href");
	}

	QString GoogleAtomParser::EditMediaLinkHref(const Node *entry) const 
	{
		if (!parser_) return "";
		return Attribute(Link(entry, "edit-media"), "href");
	}

	QString GoogleAtomParser::ETag(const Node *entry) const 
	{
		if (!parser_) return "";
		return Attribute(dynamic_cast<const Element*>(entry), "etag");
	}

	QString GoogleAtomParser::FeedLinkHref(const Node *entry) const 
	{
		if (!parser_) return "";
		NodeSet feedLinks = Find(entry, "./gd:feedLink");
		if (feedLinks.size()) 
		{
			return Attribute(dynamic_cast<const Element*>(feedLinks[0]), "href");
		} 
		else 
		{
			return "";
		}
	}

	QString GoogleAtomParser::Id(const Node *entry) const 
	{
		if (!parser_) return "";
		NodeSet ids = Find(entry, "./atom:id");
		if (ids.size()) 
		{
			const TextNode *id =
				dynamic_cast<const TextNode*>(*ids[0]->get_children().begin());
			
			QString id_str = QString::fromStdString(id->get_content());
			return id_str.mid(id_str.lastIndexOf("/") + 1); 
		} 
		else 
		{
			return "";
		}
	}

	QString GoogleAtomParser::Title(const Node *entry) const 
	{
		if (!parser_) return "";
		NodeSet titles = Find(entry, "./atom:title");
		if (titles.size()) 
		{
			const TextNode *title = dynamic_cast<const TextNode*>(*titles[0]->get_children().begin());
			QString title_str = QString::fromLocal8Bit(locale_from_utf8(title->get_content()).c_str());
			return title_str;
		} 
		else 
		{
			return "";
		}
	}
	
	QString GoogleAtomParser::getElementName(const Node* entry)
	{
		if(checkExtension(entry) == Common::eNO_ERROR || CategoryLabel(entry) == "folder") // first of all try to use file extension
		{
			return Title(entry);
		}
		else
		{
			return Title(entry) + "." + exportFormat(entry);
		}
	}

	QString GoogleAtomParser::getElementPath(int index)
	{
		xmlpp::NodeSet entries = Entries();
		QString parentId = ParentId(entries[index]);
		QString rootPath = QFileInfo(Common::WebMounter::getSettingStorage()->getAppStoragePath() 
							+ QDir::separator() 
							+ _pluginName).absoluteFilePath(); 
		
		if(ROOT_ID == parentId)
		{
			return QFileInfo(rootPath + QDir::separator() + getElementName(entries[index]))
				.absoluteFilePath();	
		}
		else
		{
			for (unsigned int i = 0; i < entries.size(); ++i) 
			{
				if(Id(entries[i]) == parentId)
				{
					return QFileInfo(getElementPath(i) + QDir::separator() + getElementName(entries[index]))
						.absoluteFilePath();		
				}
			}
			
		}
		return "";
	}
	
	RESULT GoogleAtomParser::checkExtension(const QString& file_extension) const
	{
		//List of supported file extensions
		if(file_extension == "doc"
			|| file_extension == "docx"
			|| file_extension == "txt"
			|| file_extension == "rtf"
			|| file_extension == "html"
			|| file_extension == "htm"
			|| file_extension == "ods"
			|| file_extension == "odt")
		{
			return Common::eNO_ERROR;
		}
		else if(file_extension == "pdf")
		{
			return Common::eNO_ERROR;
		}
		else if(file_extension == "ppt")
		{
			return Common::eNO_ERROR;
		}
		else if(file_extension == "xls" /*|| file_extension == "xlsx"*/)
		{
			return Common::eNO_ERROR;
		}

		return Common::eERROR;
	}
	
	RESULT GoogleAtomParser::checkExtension(const Node* entry) const
	{
		QString filename = Title(entry);
		int offset = filename.lastIndexOf(".");
		if(offset != -1)
		{
			QString file_extension = filename.mid(offset + 1).toLower();
			//List of supported file extensions
			if((file_extension == "doc"
				|| file_extension == "docx"
				|| file_extension == "txt"
				|| file_extension == "rtf"
				|| file_extension == "html"
				|| file_extension == "htm"
				|| file_extension == "ods"
				|| file_extension == "odt") && CategoryLabel(entry) == "document")
			{
				return Common::eNO_ERROR;
			}
			else if(file_extension == "pdf" && CategoryLabel(entry) == "pdf")
			{
				return Common::eNO_ERROR;
			}
			else if(file_extension == "ppt" && CategoryLabel(entry) == "presentation" )
			{
				return Common::eNO_ERROR;
			}
			else if((file_extension == "xls"
					/*|| file_extension == "xlsx"*/) && CategoryLabel(entry) == "spreadsheet")
			{
				return Common::eNO_ERROR;
			}
		}
		return Common::eERROR;
	}
	
	QString GoogleAtomParser::exportFormat(const Node* entry) const
	{
		if(checkExtension(entry) == Common::eNO_ERROR) // first of all try to use file extension
		{
			QString format = Title(entry);
			int offset = format.lastIndexOf(".");
			format = format.mid(offset + 1).toLower();
			
			if(format == "xlsx") // google does not allow to export xlsx documents, only xls
			{
				return "xls";
			}
			else
			{
				return format;
			}
		}
		
		//if extension is missed or is not supported - lets use element's category 
		QString category = CategoryLabel(entry);
		if(category == "document")
		{
			return "doc";
		}
		else if(category == "presentation")
		{
			return "ppt";
		}
		else if(category == "spreadsheet")
		{
			return "xls";
		}
		else if(category == "pdf")
		{
			return "pdf";
		}
		else if(category == "folder")
		{
			return "";
		}

		return "html"; //default format
	}
	
	RESULT GoogleAtomParser::parseDocList(const QString& xml_doc_list, QList<VFSElement>& elements)
	{
		try
		{
			if(xml_doc_list == "")
			{
				return Common::eERROR;
			}
			
			Parse(xml_doc_list);
			xmlpp::NodeSet entries = Entries();
			
			for (unsigned int i = 0; i < entries.size(); ++i) 
			{
				if(CategoryLabel(entries[i]) == "") continue;
				
				VFSElement elem (
					CategoryLabel(entries[i]) == "folder" ? VFSElement::DIRECTORY : VFSElement::FILE
					, getElementPath(i)
					, getElementName(entries[i])
					, EditLinkHref(entries[i])
					, EditMediaLinkHref(entries[i])
					, CategoryLabel(entries[i]) != "folder" ? ContentSrc(entries[i]) + "&exportFormat=" + exportFormat(entries[i]) : ContentSrc(entries[i])
					, Id(entries[i])
					, ParentId(entries[i])
					, ETag(entries[i])
					, _pluginName
					, (VFSElement::eFl_Dirty)
					);

				elements.append(elem);
			}
			return Common::eNO_ERROR;
		}
		catch(...)
		{
		}
		return Common::eERROR;
	}
	
	RESULT GoogleAtomParser::parseEntry(const QString& xml_entry, VFSElement& element)
	{
		try
		{
			if(xml_entry == "")
			{
				return Common::eERROR;
			}

			Parse(xml_entry);
			const xmlpp::Node* singleEntry = Entry();

			if(singleEntry) // We have recieved a single entry, not a list
			{
				if(CategoryLabel(singleEntry) == "folder")
				{
					return parseAlbumEntry(singleEntry, element);
				}
				else
				{
					return parseDocEntry(singleEntry, element);
				}
			}
		}
		catch(...)
		{
		}
		return Common::eERROR;
	}
	
	RESULT GoogleAtomParser::parseAlbumEntry(const xmlpp::Node* singleEntry, VFSElement& element)
	{
		try
		{
			element.setType(VFSElement::DIRECTORY);
			element.setPath(""); // we cannot generate path here, because we don't know path of parent folder. Lets driver do it.
			element.setName(Title(singleEntry));
			element.setSrcUrl(ContentSrc(singleEntry));
			element.setEditMetaUrl(EditLinkHref(singleEntry));
			element.setEditMediaUrl(EditMediaLinkHref(singleEntry));
			element.setId(Id(singleEntry));
			element.setParentId(ParentId(singleEntry));
			element.setModified(ETag(singleEntry));
			element.setPluginName(_pluginName);
			element.setFlags(VFSElement::eFl_Dirty | VFSElement::eFl_Downloaded);

			return Common::eNO_ERROR;

		}
		catch(...)
		{
		}
		return Common::eERROR;
	}
	
	RESULT GoogleAtomParser::parseDocEntry(const xmlpp::Node* singleEntry, VFSElement& element)
	{
		try
		{
			element.setType(VFSElement::FILE);
			element.setPath(""); // we cannot generate path here, because we don't know path of parent folder. Lets driver do it.
			element.setName(getElementName(singleEntry));
			element.setSrcUrl(ContentSrc(singleEntry) + "&exportFormat=" + exportFormat(singleEntry));
			element.setEditMetaUrl(EditLinkHref(singleEntry));
			element.setEditMediaUrl(EditMediaLinkHref(singleEntry));
			element.setId(Id(singleEntry));
			element.setParentId(ParentId(singleEntry));
			element.setModified(ETag(singleEntry));
			element.setPluginName(_pluginName);
			element.setFlags(VFSElement::eFl_Dirty | VFSElement::eFl_Downloaded);

			return Common::eNO_ERROR;
		}
		catch(...)
		{
		}
		return Common::eERROR;
	}
	
	QString GoogleAtomParser::Attribute(const Element *from_element,
		QString attr_name) const 
	{
		if (!parser_ || !from_element) return "";

		const Element::AttributeList& attributes = from_element->get_attributes();

		Element::AttributeList::const_iterator iter;
		for (iter = attributes.begin(); iter != attributes.end(); ++iter) 
		{
			const xmlpp::Attribute *attribute = *iter;
			if (QString::fromStdString(attribute->get_name()) == attr_name) 
			{
				return QString::fromStdString(attribute->get_value());
			}
		}
		return "";
	}

	const Element* GoogleAtomParser::Link(const Node *entry, QString rel) const 
	{
		if (!parser_) return NULL;

		NodeSet links = Find(entry, "./atom:link");
		for (unsigned int i = 0; i < links.size(); ++i) 
		{
			const Element *nodeElement = dynamic_cast<const Element*>(links[i]);
			const Element::AttributeList& attributes =
				nodeElement->get_attributes();

			Element::AttributeList::const_iterator iter;
			for (iter = attributes.begin(); iter != attributes.end(); ++iter) 
			{
				const xmlpp::Attribute *attribute = *iter;
				if (attribute->get_name() == "rel" && QString::fromStdString(attribute->get_value()) == rel) 
				{
					return nodeElement;
				}
			}
		}
		return NULL;
	}

	QString GoogleAtomParser::find_and_replace(QString source
												, const QString& find
												, const QString& replace) 
	{
			source = source.replace(QRegExp(find), replace);
			return source;
	}

};