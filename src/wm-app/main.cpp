/* Copyright (c) 2013, Alexander Ershov
 *
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 * Contact e-mail: Alexander Ershov <ershav@yandex.ru>
 */

//#include <common_stuff.h>

#include "webmounter.h"
#include "control_panel.h"
#include "single_application.h"
#include "common_stuff.h"

#include <QtGui>
#include <QtGui/QApplication>
#include <QMutex>
#include <QCryptographicHash>

int main(int argc, char *argv[])
{
	SingleApplication app(argc, argv);

	QTranslator translator;
	translator.load(QString("webmounter_" + QLocale::system().name()));
	QApplication::installTranslator(&translator);

	if(!app.isSingle())
	{
		/*QMessageBox::information(0, QObject::tr("WebMounter"),
		QObject::tr("WebMounter is already running."));*/

		return 1;
	}

	if (!QSystemTrayIcon::isSystemTrayAvailable()) 
	{
		QMessageBox::critical(0, QObject::tr("Systray"),
			QObject::tr("I couldn't detect any system tray "
			"on this system."));
		return 1;
	}

	QApplication::setQuitOnLastWindowClosed(false);
	app.setApplicationName(QObject::tr("WebMounter"));
	app.setApplicationVersion(VERSION);
	app.setOrganizationName("WebMounter");

	QApplication::removeTranslator(&translator);

	Common::WebMounter mainApp;

	Ui::ControlPanel* panel = new Ui::ControlPanel;
	mainApp.startApp( panel );

	QObject::connect(panel, SIGNAL(mount()), &mainApp, SLOT(mount()));
	QObject::connect(panel, SIGNAL(unmount()), &mainApp, SLOT(unmount())); 

	panel->show();

	int res = app.exec();

	delete panel;

	return res;
}
