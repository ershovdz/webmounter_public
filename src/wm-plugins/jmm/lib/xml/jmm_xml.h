#include "common_stuff.h"

#include "vfs_cache.h"

#include <algorithm>
#include <tinyxml.h>

#define NAME_PART                0x00000001
#define SMALLURL_PART            0x00000010
#define ORIGURL_PART             0x00000100
#define PATH_PART                0x00001000
#define TYPE_PART                0x00010000
#define ID_PART			         0x00100000
#define PARENT_ID_PART           0x01000000
#define MODIFIED_PART            0x10000000

#define FILE_COMPLETELY_PARSED   0x11111111
#define DIR_COMPLETELY_PARSED    0x01111001

#if defined(WEBMOUNTER_JMM_LIBRARY)
#  define WEBMOUNTER_JMM_EXPORT Q_DECL_EXPORT
#else
#  define WEBMOUNTER_JMM_EXPORT Q_DECL_IMPORT
#endif

namespace Xml
{
	using namespace Data;
	
	class WEBMOUNTER_JMM_EXPORT JmmXmlParser
	{
	public:
		static bool populateCache(const QString& path, const QString& rootPath, const QString& pluginName );
		static QString parseIdResponse(QString& response);
		static QString parseUploadResponse(QString& response);
		static QString parseResponse(QString& response);
		static QString parseAuthResponse(QString& response);
		static QString parseTextResponse(QString& response);

	private:
		static QString parseIdResponse( TiXmlNode* pParent, QString& response);
		static QString parseUploadResponse( TiXmlNode* pParent, QString& response);
		static QString parseResponse(TiXmlNode* pParent, QString& response);
		static QString parseAuthResponse( TiXmlNode* pParent, QString& response);
		static QString parseTextResponse( TiXmlNode* pParent, QString& response);
		static void populateCache( TiXmlNode* pParent, VFSElementPtr pVFSElem, const QString& rootPath, int& parsingState, const QString& pluginName );
		static ElementType getType(TiXmlElement* pElement);

		class CheckId 
		{
		public:
			CheckId(const QString& id, const QString& pluginName) : testId(id), _pluginName(pluginName) { }
			QString testId;
			QString _pluginName;
			bool operator () (VFSCache::iterator iter)
			{ 
				return (iter->getId() == testId
					&& iter->getType() == VFSElement::DIRECTORY
					&& iter->getPluginName() == _pluginName); 
			}
		};
	};
}
