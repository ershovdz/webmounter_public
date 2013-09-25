TEMPLATE        = lib
CONFIG         += plugin

HEADERS         = jmm_article_plugin.h

SOURCES         = jmm_article_plugin.cpp

TARGET          = $$qtLibraryTarget(wm-jmm-article-plugin)
DESTDIR         = ../../../build/lib/plugins
INCLUDEPATH     += ../../../wm-base ../../../wm-ui ../lib
QMAKE_RPATHDIR  += ../../../build/lib/plugins ../../../build/lib /usr/lib/webmounter/plugins
QMAKE_LIBDIR    += ../../../build/lib/plugins ../../../build/lib
LIBS            += -ljmm

win32 {
	INCLUDEPATH += $(BOOST_DIR) $(LIBCURL_DIR)\include 
	LIBS            += -lwmbase -lwmui
}

target.path = /usr/lib/webmounter/plugins
INSTALLS += target

RESOURCES += ../lib/lib.qrc
