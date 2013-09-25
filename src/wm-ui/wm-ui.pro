TARGET = wmui
TEMPLATE = lib
target.path = /usr/lib/webmounter/ui
DESTDIR = ../build/lib

DEFINES += WEBMOUNTER_UI_LIBRARY

QMAKE_CXXFLAGS += -D_FILE_OFFSET_BITS=64

INCLUDEPATH += ../wm-base

HEADERS += general_view.h \
	plugin_view.h \
	view.h

SOURCES += general_view.cpp \
	plugin_view.cpp

HEADERS_EXPORT += notification_device.h \
	plugin_view.h \
	view.h

win32 {
	INCLUDEPATH += $(BOOST_DIR) $(DOKAN_DIR)
}


sources.files += $$HEADERS_EXPORT
sources.path = /usr/include/webmounter

QMAKE_RPATHDIR += ../build/lib /usr/lib/webmounter/base
QMAKE_LIBDIR += ../build/lib

LIBS += -lwmbase

TRANSLATIONS += ./lang/wmui_ru.ts \
	./lang/wmui_en.ts

unix {
	CONFIG(release) system(lrelease ./lang/*.ts)
	LRELEASE = $$[QT_INSTALL_BINS]/lrelease

	updateqm.input = TRANSLATIONS
	updateqm.output = ../build/share/webmounter/${QMAKE_FILE_BASE}.qm
	updateqm.commands = $$LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
	updateqm.name = LRELEASE
	updateqm.CONFIG += no_link
	QMAKE_EXTRA_COMPILERS += updateqm
	PRE_TARGETDEPS += compiler_updateqm_make_all

	qmfiles.files = ./lang/*.qm
	qmfiles.path = /usr/share/webmounter

	lib.files += libwmui.pc
	lib.path = /usr/lib/pkgconfig

	INSTALLS += sources \
				qmfiles \
				lib
}

INSTALLS += target
