QT += network webkit
TEMPLATE        = lib
CONFIG         += plugin

INCLUDEPATH     += ../../wm-base ../../wm-ui

HEADERS         = facebook_plugin.h \
	./driver/facebook_driver.h \
	./connector/graphapi.h \
	./connector/qfacebookreply.h \
	./connector/facebook_connector.h \
	./view/facebook_view.h \
	./view/facebook_oauth.h

SOURCES         = facebook_plugin.cpp \
	./driver/facebook_driver.cpp \
	./connector/facebook_connector.cpp \
	./connector/graphapi.cpp \
	./connector/qfacebookreply.cpp \
	./view/facebook_view.cpp \
	./view/facebook_oauth.cpp 

win32 {
	INCLUDEPATH += $(BOOST_DIR) $(LIBCURL_DIR)\include $(QJSON_DIR)\include

	QMAKE_LIBDIR += $(LIBCURL_DIR)\lib\DLL-Release $(QJSON_DIR)\lib

	LIBS += -llibcurl_imp -lwmbase -lwmui -lqjson
}
else {
	LIBS += -lcurl -lqjson

	CONFIG(release) system(lrelease ./lang/*.ts)
	LRELEASE = $$[QT_INSTALL_BINS]/lrelease

	updateqm.input = TRANSLATIONS
	updateqm.output = ../../build/share/webmounter/${QMAKE_FILE_BASE}.qm
	updateqm.commands = $$LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
	updateqm.name = LRELEASE
	updateqm.CONFIG += no_link
	QMAKE_EXTRA_COMPILERS += updateqm
	PRE_TARGETDEPS += compiler_updateqm_make_all

	qmfiles.files = ./lang/*.qm
	qmfiles.path = /usr/share/webmounter

	INSTALLS += qmfiles
}

QMAKE_RPATHDIR += ../../build/lib
QMAKE_LIBDIR += ../../build/lib

TARGET          = $$qtLibraryTarget(wm-facebook-plugin)
DESTDIR         = ../../build/lib/plugins

target.path = /usr/lib/webmounter/plugins

RESOURCES += facebook.qrc

TRANSLATIONS += ./lang/facebook_wm_pl_ru.ts \
	./lang/facebook_wm_pl_en.ts

INSTALLS += target 
