TEMPLATE = app
TARGET = webmounter
DESTDIR = ../build/bin
BINDIR = /usr/bin
target.path = $$BINDIR

QT += network

INCLUDEPATH += ../wm-base \
    ../wm-ui

QMAKE_LIBDIR += ../build/lib

HEADERS += control_panel.h \
    tray_notification_device.h \
	single_application.h

SOURCES += main.cpp \
    control_panel.cpp \
    tray_notification_device.cpp \
	single_application.cpp

win32 {
	INCLUDEPATH += $(BOOST_DIR) $(DOKAN_DIR)
}

TRANSLATIONS += ./lang/webmounter_ru.ts \
	./lang/webmounter_en.ts

unix { 
    CONFIG(release) system(lrelease ./lang/*.ts)
    LRELEASE = $$[QT_INSTALL_BINS]/lrelease

    updateqm.input = TRANSLATIONS
    updateqm.output = ../build/share/webmounter/${QMAKE_FILE_BASE}.qm
    updateqm.commands = $$LRELEASE \
        ${QMAKE_FILE_IN} \
        -qm \
        ${QMAKE_FILE_OUT}
    updateqm.name = LRELEASE
    updateqm.CONFIG += no_link
    QMAKE_EXTRA_COMPILERS += updateqm
    PRE_TARGETDEPS += compiler_updateqm_make_all

    qmfiles.files = ./lang/*.qm
    qmfiles.path = /usr/share/webmounter

    desktop.files += webmounter.desktop
    desktop.path = /usr/share/applications

    pixmaps.files += ./resources/drive.png
    pixmaps.path = /usr/share/pixmaps

    ldconf.files += webmounter.conf
    ldconf.path = /etc/ld.so.conf.d

    libpath.path = .
    libpath.commands = "echo $${INSTALL_PREFIX}/webmounter > /etc/ld.so.conf.d/webmounter.conf" && "ldconfig"

    INSTALLS += qmfiles \
        desktop \
        pixmaps \
        ldconf \
        libpath
}

LIBS += -lwmbase -lwmui

OBJECTS_DIR += ../build/bin

RESOURCES += controlpanel.qrc
INSTALLS += target
# include autoupdater
#!include("./updater/Fervor.pri") {
#    error("Unable to include autoupdater.")
#}
