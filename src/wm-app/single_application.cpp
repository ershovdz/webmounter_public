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

#include <qglobal.h>
#ifdef Q_OS_WIN
#include <windows.h>
#endif
#include <QtGui/QMessageBox>
#include "single_application.h"

SingleInstance::SingleInstance( QObject *parent ) : QTcpServer(parent)
{
	setMaxPendingConnections(1);
	listen(QHostAddress::Any, 2222);
}

void SingleInstance::newConnection(int /*socket*/)
{
	//  No need to implement anything!
	//  The parameter gives a warning at compile-time but that's nothing to care about...
}

SingleApplication::SingleApplication(int &argc, char **argv) : QApplication(argc, argv)
{
}

bool SingleApplication::isSingle()
{
#ifdef Q_OS_WIN
	HANDLE hMutex = CreateMutex(0, TRUE, L"WebMounter");

	if(GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(hMutex);
		return false;
	}

	return true;
#endif
#ifdef Q_OS_LINUX
	m_instance = new SingleInstance(this);
	return m_instance->isListening();
#endif
}
