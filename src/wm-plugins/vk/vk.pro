QT += network webkit
TEMPLATE        = lib
CONFIG         += plugin

INCLUDEPATH     += ../../wm-base ../../wm-ui

HEADERS         = vk_plugin.h \
	./connector/vk_connector.h \
	./driver/vk_driver.h \
	./view/vk_view.h \
	./view/vk_oauth.h

SOURCES         = vk_plugin.cpp \
	./connector/vk_connector.cpp \
	./driver/vk_driver.cpp \
	./view/vk_view.cpp \
	./view/vk_oauth.cpp 

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

TARGET          = $$qtLibraryTarget(wm-vk-plugin)
DESTDIR         = ../../build/lib/plugins

target.path = /usr/lib/webmounter/plugins

RESOURCES += vk.qrc

TRANSLATIONS += ./lang/vk_wm_pl_ru.ts \
	./lang/vk_wm_pl_en.ts

INSTALLS += target 
