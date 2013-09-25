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

#ifndef IGUI_H
#define IGUI_H

#include "common_stuff.h"
#include "notification_device.h"

namespace Ui
{
  WmInterface IGui
  {
    virtual ~IGui() {}

    virtual void mounted() = 0;
    virtual void unmounted() = 0;
    virtual NotificationDevice* getNotificationDevice() = 0;
  };
}

#endif // IGUI_H