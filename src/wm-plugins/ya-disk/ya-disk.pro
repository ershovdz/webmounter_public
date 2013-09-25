QT += network webkit xml
TEMPLATE        = lib
CONFIG         += plugin

INCLUDEPATH     += ../../wm-base ../../wm-ui

HEADERS         = yandex_disk_plugin.h \
	./driver/yandex_disk_driver.h \
	./view/yandex_disk_view.h \
	./connector/yandex_disk_connector.h \
	./view/yandex_disk_oauth.h

SOURCES         = yandex_disk_plugin.cpp \
	./driver/yandex_disk_driver.cpp \
	./view/yandex_disk_view.cpp \
	./connector/yandex_disk_connector.cpp \
	./view/yandex_disk_oauth.cpp 

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

TARGET          = $$qtLibraryTarget(wm-ya-disk-plugin)
DESTDIR         = ../../build/lib/plugins

target.path = /usr/lib/webmounter/plugins

RESOURCES += ya-disk.qrc

TRANSLATIONS += ./lang/yandex_disk_wm_pl_ru.ts \
	./lang/yandex_disk_wm_pl_en.ts

INSTALLS += target

