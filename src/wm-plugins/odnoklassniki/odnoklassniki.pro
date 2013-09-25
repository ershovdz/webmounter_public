TEMPLATE        = lib
CONFIG         += plugin

HEADERS         = odn_plugin.h

SOURCES         = odn_plugin.cpp 

TARGET          = $$qtLibraryTarget(wm-odn-plugin)
DESTDIR         = ../../build/lib/plugins
INCLUDEPATH     += ../../wm-base ../../wm-ui
QMAKE_LIBDIR += ../../build/lib

win32 {
	INCLUDEPATH += $(BOOST_DIR) $(LIBCURL_DIR)\include

}

LIBS += -lwmui -lwmbase

target.path = /usr/lib/webmounter/plugins
INSTALLS += target
