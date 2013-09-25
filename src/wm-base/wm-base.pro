QT += sql network
TARGET = wmbase
TEMPLATE = lib
target.path = /usr/lib/webmounter/base
DESTDIR = ../build/lib
DEFINES += WEBMOUNTER_LIBRARY
QMAKE_CXXFLAGS += -D_FILE_OFFSET_BITS=64 \
        -DFUSE_USE_VERSION=26
HEADERS += common_stuff.h \
	data.h \
	file_proxy.h \
	lvfs_driver.h \
	vfs_cache.h \
	vfs_element.h \
	webmounter.h \
	rvfs_driver.h \
	reg_exp.h \
	plugin_interface.h \
	notification_device.h \
	filesystemtools.h

SOURCES += data.cpp \
	file_proxy.cpp \
	rvfs_driver.cpp \
	vfs_cache.cpp \
	vfs_element.cpp \
	webmounter.cpp \
	reg_exp.cpp \
	filesystemtools.cpp

HEADERS_EXPORT += common_stuff.h \
	data.h \
	vfs_cache.h \
	vfs_element.h \
	webmounter.h \
	rvfs_driver.h \
	reg_exp.h \
	plugin_interface.h \
	notification_device.h \
	lvfs_driver.h \
	file_proxy.h

win32 {
	HEADERS += win_lvfs_driver.h
	SOURCES += win_lvfs_driver.cpp

	INCLUDEPATH += $(BOOST_DIR) $(DOKAN_DIR)

	QMAKE_LIBDIR += $(DOKAN_DIR)
	
	LIBS += -ldokan 
}

unix {
	HEADERS += linux_lvfs_driver.h
	SOURCES += linux_lvfs_driver.cpp

	LIBS += -lfuse

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

	lib.files += libwmbase.pc
	lib.path = /usr/lib/pkgconfig
	
	INSTALLS += qmfiles \
		    lib
}

TRANSLATIONS += ./lang/wmbase_ru.ts \
	./lang/wmbase_en.ts

sources.files += $$HEADERS_EXPORT
sources.path = /usr/include/webmounter

INSTALLS += target \
			sources 
