TEMPLATE        = lib
CONFIG         += plugin

HEADERS         = google_plugin.h \
				./view/google_view.h \
				./driver/google_driver.h \
				./connector/google_connector.h \
				./xml/google_xml.h

SOURCES         = google_plugin.cpp \
				./view/google_view.cpp \
				./driver/google_driver.cpp \
				./connector/google_connector.cpp \
				./xml/google_xml.cpp


TARGET          = $$qtLibraryTarget(wm-google-plugin)
DESTDIR         = ../../build/lib/plugins

INCLUDEPATH     += ../../wm-base ../../wm-ui

win32 {
	INCLUDEPATH += $(BOOST_DIR) $(LIBCURL_DIR)\include \
	c:\gtkmm\include\libxml++-2.6\ \
	c:\gtkmm\include\glibmm-2.4 \
	c:\gtkmm\lib\glibmm-2.4\include \
	c:\gtkmm\include\glib-2.0 \
	c:\gtkmm\lib\glib-2.0\include \
	c:\gtkmm\lib\libxml++-2.6\include

	QMAKE_LIBDIR += $(LIBCURL_DIR)\lib\DLL-Debug ..\..\build\lib \
	d:\Users\Yu\Projects\Webdisk\libxml++-2.33.2\MSVC_Net2008\libxml++\Win32\Debug \
	c:\gtkmm\lib
	
	LIBS += -lxml++-vc100-d-2_6 -lglibmm-vc100-d-2_4 -llibcurld -lwmbase -lwmui
}

QMAKE_RPATHDIR += ../../build/lib
QMAKE_LIBDIR += ../../build/lib

unix {
	INCLUDEPATH     += /usr/include/libxml++-2.6 \
				   /usr/include/glibmm-2.4 \
				   /usr/lib/glibmm-2.4/include \
				   /usr/include/glib-2.0 \
				   /usr/lib/glib-2.0/include \
				   /usr/lib/libxml++-2.6/include

	LIBS += -lxml++-2.6 -lglibmm-2.4 -lcurl

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

	INSTALLS +=	qmfiles
}

target.path = /usr/lib/webmounter/plugins

RESOURCES += google.qrc

TRANSLATIONS += ./lang/google_wm_pl_ru.ts \
	./lang/google_wm_pl_en.ts

INSTALLS += target 
