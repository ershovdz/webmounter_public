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

#ifndef SINGLE_APPLICATION_H
#define SINGLE_APPLICATION_H

#include <QtNetwork/QTcpServer>
#include <QtGui/QApplication>

class SingleInstance : public QTcpServer
{
	Q_OBJECT
public:
	SingleInstance( QObject *parent=0 );
	void newConnection( int );
};

class SingleApplication : public QApplication
{
	Q_OBJECT
public:
	SingleApplication(int &argc, char **argv);
	bool isSingle();
private:
#ifdef Q_OS_LINUX
	SingleInstance* m_instance;
#endif
};


#endif // SINGLE_APPLICATION_H
