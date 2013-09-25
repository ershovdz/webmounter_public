#include "jmm_xml.h"

#include "webmounter.h"

#include <QFile>

namespace Xml
{
	QString JmmXmlParser::parseIdResponse(QString& response)
	{
		TiXmlDocument xmlDoc;
		const char* result = xmlDoc.Parse(response.toUtf8());
		if(result)
		{
			return parseIdResponse(&xmlDoc, response);
		}
		else
		{
			response = "";
			return response;
		}
	}
	QString JmmXmlParser::parseIdResponse( TiXmlNode* pParent, QString& response)
	{
		if ( !pParent ) 
		{
			response = "";
			return response;
		}

		TiXmlNode* pChild;
		TiXmlText* pText;
		int t = pParent->Type();
		int num;

#ifdef Q_OS_WIN
		if(t == TiXmlNode::TINYXML_TEXT)
#else
		if(t == TiXmlNode::TEXT)
#endif // Q_OS_WIN
		{
			pText = pParent->ToText();

			if(!strcmp(pParent->Parent()->Value(), "response"))
			{
				response = pText->Value();
				return response;
			}
		}

		for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) 
		{
			parseIdResponse( pChild, response);
		}

		return response;
	}
	QString JmmXmlParser::parseResponse(QString& response)
	{
		TiXmlDocument xmlDoc;
		const char* result = xmlDoc.Parse(response.toUtf8().constData());

		if(result)
		{
			return parseResponse(&xmlDoc, response);
		}
		else
		{
			return QString("1");
		}
	}
	QString JmmXmlParser::parseTextResponse(QString& response)
	{
		TiXmlDocument xmlDoc;
		const char* result = xmlDoc.Parse(response.toUtf8());

		if(result)
		{
			return parseAuthResponse(&xmlDoc, response);
		}
		else
		{
			response = "";
			return response;
		}
	}
	QString JmmXmlParser::parseAuthResponse(QString& response)
	{
		TiXmlDocument xmlDoc;
		const char* result = xmlDoc.Parse(qPrintable(response));

		if(result)
		{
			return parseAuthResponse(&xmlDoc, response);
		}
		else
		{
			response = "";
			return response;
		}
	}
	QString JmmXmlParser::parseResponse( TiXmlNode* pParent, QString& response)
	{
		QString result = "-1";
		if ( !pParent ) 
		{
			return result;
		}

		TiXmlNode* pChild;
		TiXmlText* pText;
		int t = pParent->Type();

#ifdef Q_OS_WIN
		if(t == TiXmlNode::TINYXML_TEXT)
#else
		if(t == TiXmlNode::TEXT)
#endif //Q_OS_WIN
		{
			pText = pParent->ToText();

			if(!strcmp(pParent->Parent()->Value(), "error"))
			{
				response = pText->Value();
                                return response;//.toInt();
			}
		}

		for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) 
		{
			result = parseResponse( pChild, response);
			if(result != "-1")
			{
				return result;
			}
		}

		return result;
	}
	QString JmmXmlParser::parseTextResponse( TiXmlNode* pParent, QString& response)
	{
		if ( !pParent ) 
		{
			response = "";
			return response;
		}

		TiXmlNode* pChild;
		TiXmlText* pText;
		int t = pParent->Type();
		int num;

#ifdef Q_OS_WIN
		if(t == TiXmlNode::TINYXML_TEXT)
#else
		if(t == TiXmlNode::TEXT)
#endif // Q_OS_WIN
		{
			pText = pParent->ToText();

			if(!strcmp(pParent->Parent()->Value(), "error"))
			{
				response = pText->Value();
				return response;
			}
		}

		for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) 
		{
			return parseTextResponse( pChild, response);
		}

		response = "";
		return response;
	}

	QString JmmXmlParser::parseUploadResponse( TiXmlNode* pParent, QString& response)
	{
		if ( !pParent ) 
		{
			return 0;
		}

		TiXmlNode* pChild;
		TiXmlText* pText;
		int t = pParent->Type();
		int num;

#ifdef Q_OS_WIN
		if(t == TiXmlNode::TINYXML_TEXT)
#else
		if(t == TiXmlNode::TEXT)
#endif // Q_OS_WIN
		{
			pText = pParent->ToText();

			if(!strcmp(pParent->Parent()->Value(), "photoid"))
			{
				response = pText->Value();
                                return response;//.toInt();
			}
		}

		for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) 
		{
			return parseUploadResponse( pChild, response);
		}

		return 0;
	}
	QString JmmXmlParser::parseAuthResponse( TiXmlNode* pParent, QString& response)
	{
		if ( !pParent ) 
		{
			response = "";
			return response;
		}

		TiXmlNode* pChild;
		TiXmlText* pText;
		int t = pParent->Type();
		int num;

#ifdef Q_OS_WIN
		if(t == TiXmlNode::TINYXML_TEXT)
#else
		if(t == TiXmlNode::TEXT)
#endif // Q_OS_WIN
		{
			pText = pParent->ToText();

			if(!strcmp(pParent->Parent()->Value(), "response"))
			{
				response = pText->Value();
				return response;
			}
		}

		for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) 
		{
			parseAuthResponse( pChild, response);
		}

		//response = "";
		return response;
	}
	QString JmmXmlParser::parseUploadResponse(QString& response)
	{
		TiXmlDocument xmlDoc;
		const char* result = xmlDoc.Parse(response.toUtf8());
		if(result)
		{
			return parseUploadResponse(&xmlDoc, response);
		}
		else
		{
			return 0;
		}
	}

	void JmmXmlParser::populateCache(TiXmlNode* pParent, VFSElementPtr ptrVFSElem, const QString& rootPath, int& parsingState, const QString& pluginName)
	{
		if ( !pParent ) return;

		VFSCache* vfsCache = Common::WebMounter::getCache();
		
		TiXmlNode* pChild;
		TiXmlText* pText;
		int t = pParent->Type();
		int num;

		switch ( t )
		{
#ifdef Q_OS_WIN
			case TiXmlNode::TINYXML_ELEMENT:
#else
			case TiXmlNode::ELEMENT:
#endif // Q_OS_WIN
			{
				if(!strcmp(pParent->Value(), "node"))
				{
					parsingState |= TYPE_PART;
					ptrVFSElem->setType(getType(pParent->ToElement()));
				}

				break;
			}
#ifdef Q_OS_WIN
			case TiXmlNode::TINYXML_TEXT:
#else
			case TiXmlNode::TEXT:
#endif // Q_OS_WIN
			{
				const char* attr_name = pParent->Parent()->Value();
				pText = pParent->ToText();

				if(!ptrVFSElem)
				{
					return;
				}

				if(!strcmp(attr_name, "caption"))
				{
					parsingState |= NAME_PART;
					ptrVFSElem->setName(QString::fromUtf8(pText->Value()));
				}

				if(!strcmp(attr_name, "cid"))
				{
					QString id = QString::fromUtf8(pText->Value());
					
					//if(TIXML_SSCANF( pText->Value(), "%d", &id ) == 1)
					//{
						if(id == ROOT_ID) //root element
						{
							ptrVFSElem->reset();
							ptrVFSElem->setPluginName(pluginName);
							parsingState = 0;
						}
						else
						{
							parsingState |= ID_PART;
							ptrVFSElem->setId(id);
						}
					//}
				}

				if(!strcmp(attr_name, "parent"))
				{
					QString id = QString::fromUtf8(pText->Value());
					//if(TIXML_SSCANF( pText->Value(), "%d", &id ) == 1)
					//{
						parsingState |= PARENT_ID_PART;
						ptrVFSElem->setParentId(id);
					//}
					
					VFSCache::iterator iter = std::find_if(vfsCache->begin(), vfsCache->end(), CheckId(id, pluginName));
					
					if(iter != vfsCache->end())
					{
						QString path = iter->getPath();
						path += QDir::separator();
						path += ptrVFSElem->getName();
						QDir dir(path);
						ptrVFSElem->setPath(dir.absolutePath());
						parsingState |= PATH_PART;
					}
					else
					{
						QDir dir(rootPath + QDir::separator() + ptrVFSElem->getName());
						ptrVFSElem->setPath(dir.absolutePath());
						parsingState |= PATH_PART;
					}
					

					if(ptrVFSElem->getType() == VFSElement::DIRECTORY)
					{
						if(parsingState & DIR_COMPLETELY_PARSED)
						{
							vfsCache->insert(*ptrVFSElem, true, false); // all elements are "dirty", after inserting
						}
						ptrVFSElem->reset();
						ptrVFSElem->setPluginName(pluginName);
						parsingState = 0;
					}
				}

				if(!strcmp(attr_name, "big_url"))
				{
					ptrVFSElem->setSrcUrl(QString::fromUtf8(pText->Value()));
					parsingState |= ORIGURL_PART;
				}

				if(!strcmp(attr_name, "small_url"))
				{
					ptrVFSElem->setEditMetaUrl(QString::fromUtf8(pText->Value()));
					parsingState |= SMALLURL_PART;
				}

				if(!strcmp(attr_name, "modified"))
				{
					ptrVFSElem->setModified(QString::fromUtf8(pText->Value()));
					parsingState |= MODIFIED_PART;

					if(ptrVFSElem->getType() == VFSElement::FILE 
						&& parsingState & FILE_COMPLETELY_PARSED)
					{
						vfsCache->insert(*ptrVFSElem, true, false); // after inserting all elements are "dirty"
						ptrVFSElem->reset();
						ptrVFSElem->setPluginName(pluginName);
						parsingState = 0;
					}
				}

				break;
			}

		default:
			break;
		}

		for ( pChild = pParent->FirstChild(); pChild != 0; pChild = pChild->NextSibling()) 
		{
			populateCache( pChild, ptrVFSElem, rootPath, parsingState, pluginName );
		}
	}

	bool JmmXmlParser::populateCache(const QString& filePath, const QString& rootPath, const QString& pluginName)
	{
		TiXmlDocument doc;

		QFile fileToParse(filePath);
		fileToParse.open(QIODevice::ReadOnly | QIODevice::Text);
		int fd = fileToParse.handle();
		FILE* pFile = fdopen(fd, "r");
		bool loadOkay = doc.LoadFile(pFile);
		//fclose(pFile);
		int parsingState = 0;
		VFSElementPtr ptrVFSElem (new VFSElement());
		if (loadOkay)
		{
			populateCache( &doc, ptrVFSElem, rootPath, parsingState, pluginName);
			return 0;
		}
		else
		{
			return 1;
		}
	}

	ElementType JmmXmlParser::getType(TiXmlElement* pElement)
	{
		if ( !pElement ) return VFSElement::UNKNOWN;

		TiXmlAttribute* pAttrib=pElement->FirstAttribute();

		if(!strcmp(pAttrib->Name(), "type"))
		{
			if(!strcmp(pAttrib->Value(), "page")
				|| !strcmp(pAttrib->Value(), "root"))
			{
				return VFSElement::DIRECTORY;
			}
			if(!strcmp(pAttrib->Value(), "module"))
			{
				return VFSElement::FILE;
			}

			return VFSElement::UNKNOWN;
		}
		else
		{
			return VFSElement::UNKNOWN;
		}
	}
};
