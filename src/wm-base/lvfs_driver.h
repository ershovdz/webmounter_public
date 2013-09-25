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

#ifndef ILVFS_DRIVER_H
#define ILVFS_DRIVER_H

#define VLFS_CALLBACK __stdcall

#include "data.h"
#include <QThread>

namespace LocalDriver
{
	using namespace Data;
	class ILVFSDriver : public QThread
	{
		Q_OBJECT
			public slots:
				virtual void mount(Data::GeneralSettings& generalSettings) = 0;
				virtual void unmount() = 0;

signals:
				void mounted();
				void unmounted();
	};
};

#endif