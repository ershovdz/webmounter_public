TARGET = jmm
TEMPLATE = lib
DESTDIR         = ../../../build/lib/plugins
DEFINES += JMM_LIBRARY

HEADERS         = ./connector/jmm_connector.h \
	./driver/jmm_driver.h \
	./view/jmm_view.h \
	./xml/jmm_xml.h

SOURCES         = ./connector/jmm_connector.cpp \
	./driver/jmm_driver.cpp \
	./view/jmm_view.cpp \
	./xml/jmm_xml.cpp


INCLUDEPATH     += ../../../wm-base ../../../wm-ui

win32 {
	INCLUDEPATH += $(BOOST_DIR) $(LIBCURL_DIR)\include $(TINYXML_DIR)

	QMAKE_LIBDIR += $(LIBCURL_DIR)\lib\DLL-Debug $(TINYXML_DIR)\Debug
	
	LIBS += -ltinyxmld -llibcurld
}
else {
	QMAKE_RPATHDIR += ../../../build/lib /usr/lib/webmounter/base /usr/lib/webmounter/ui
	LIBS += -lcurl -ltinyxml

	LRELEASE = $$[QT_INSTALL_BINS]/lrelease

	updateqm.input = TRANSLATIONS
	updateqm.output = ../../../build/share/webmounter/${QMAKE_FILE_BASE}.qm
	updateqm.commands = $$LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
	updateqm.name = LRELEASE
	updateqm.CONFIG += no_link
	QMAKE_EXTRA_COMPILERS += updateqm
	PRE_TARGETDEPS += compiler_updateqm_make_all

	qmfiles.files = ./lang/*.qm
	qmfiles.path = /usr/share/webmounter

	INSTALLS += qmfiles
}

QMAKE_LIBDIR += ../../../build/lib
LIBS += -lwmbase -lwmui

target.path = /usr/lib/webmounter/plugins

RESOURCES += lib.qrc

CONFIG(release) system(lrelease ./lang/*.ts)
TRANSLATIONS += ./lang/jmm_ru.ts \
	./lang/jmm_en.ts

DEFINES += WEBMOUNTER_JMM_LIBRARY

INSTALLS += target 
