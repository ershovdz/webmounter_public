QT += network webkit
TEMPLATE        = lib
CONFIG         += plugin

INCLUDEPATH     += ../../wm-base ../../wm-ui

HEADERS         = yanarod_plugin.h \
	./driver/yandex_narod_driver.h \
	./view/yandex_narod_view.h \
	./connector/yandex_narod_connector.h \
	./connector/cp_rsa.h \
	./connector/base64.h

SOURCES         = yanarod_plugin.cpp \
	./driver/yandex_narod_driver.cpp \
	./view/yandex_narod_view.cpp \
	./connector/yandex_narod_connector.cpp \
	./connector/cp_rsa.cpp \
	./connector/base64.cpp

win32 {
	INCLUDEPATH += $(BOOST_DIR) $(LIBCURL_DIR)\include

	QMAKE_LIBDIR += $(LIBCURL_DIR)\lib\DLL-Release 

	LIBS += -llibcurl_imp -lwmbase -lwmui
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

TARGET          = $$qtLibraryTarget(wm-ya-narod-plugin)
DESTDIR         = ../../build/lib/plugins

target.path = /usr/lib/webmounter/plugins

RESOURCES += ya-narod.qrc

TRANSLATIONS += ./lang/yanarod_wm_pl_ru.ts \
	./lang/yanarod_wm_pl_en.ts

INSTALLS += target

