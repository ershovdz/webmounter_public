QT += network webkit
TEMPLATE        = lib
CONFIG         += plugin

INCLUDEPATH     += ../../wm-base ../../wm-ui

HEADERS         = yaf_plugin.h \
	./driver/yaf_driver.h \
	./view/yaf_view.h \
	./connector/yaf_connector.h \
	./view/yaf_oauth.h

SOURCES         = yaf_plugin.cpp \
	./driver/yaf_driver.cpp \
	./view/yaf_view.cpp \
	./connector/yaf_connector.cpp \
	./view/yaf_oauth.cpp 

win32 {
	INCLUDEPATH += $(BOOST_DIR) $(LIBCURL_DIR)\include

	QMAKE_LIBDIR += $(LIBCURL_DIR)\lib\DLL-Release 

	LIBS += -llibcurl_imp -lwmbase -lwmui

	CONFIG(debug, debug|release) {
		LIBS += -L$(BOOST_DIR)\bin.v2\libs\date_time\build\msvc-10.0\debug\link-static\threading-multi \ 
		-L$(BOOST_DIR)\bin.v2\libs\thread\build\msvc-10.0\debug\link-static\threading-multi \ 
		-llibboost_date_time-vc100-mt-gd-1_45 \
		-llibboost_thread-vc100-mt-gd-1_45
	} else {
		LIBS += -L$(BOOST_DIR)\bin.v2\libs\date_time\build\msvc-10.0\release\link-static\threading-multi  \ 
		-L$(BOOST_DIR)\bin.v2\libs\thread\build\msvc-10.0\release\link-static\threading-multi \
		-llibboost_date_time-vc100-mt-1_45 \
		-llibboost_thread-vc100-mt-1_45
	}
}
else {
	LIBS += -lcurl

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

TARGET          = $$qtLibraryTarget(wm-yandex-plugin)
DESTDIR         = ../../build/lib/plugins

target.path = /usr/lib/webmounter/plugins

RESOURCES += yaf.qrc

TRANSLATIONS += ./lang/yaf_wm_pl_ru.ts \
	./lang/yaf_wm_pl_en.ts

INSTALLS += target

