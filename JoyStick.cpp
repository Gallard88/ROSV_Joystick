/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * main.c
 * Copyright (C) Thomas 2012 <thomasburns88@gmail.com>
 *
Radio_Driver is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Radio_Driver is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
	Last change: TB 26/01/2012 9:48:49 AM
 */
using namespace std;

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>
#include <syslog.h>

#include "JoyStick.hpp"
/* ======================== */
#define NUM_AXIS	6
#define NUM_BUT		10

/* ======================== */
JoyStickDriver::JoyStickDriver(const char *device)
{
  DeadZone = 10;
  file_fd = -1;
  Device = new char((strlen(device)));
  strcpy(Device, device);
  Connect();
}

/* ======================== */
JoyStickDriver::~JoyStickDriver()
{
  ClosePort();
}

void JoyStickDriver::ClosePort(void)
{
  delete [] axis;
  delete [] buttons;
  numButtons = -1;
  numAxis = -1;

  if ( file_fd >= 0 ) {
    close(file_fd);
    file_fd = -1;
  }
}

/* ======================== */
int  JoyStickDriver::Connect(void)
{
  if ( file_fd < 0 ) {
    file_fd = open( Device , O_RDONLY);
    if ( file_fd >= 0 ) {
      ioctl( file_fd, JSIOCGNAME(80), &joy_name );

      ioctl( file_fd, JSIOCGAXES,    &numAxis );
      ioctl( file_fd, JSIOCGBUTTONS, &numButtons );

      axis = new int [numAxis];
      buttons = new int [numButtons];

      memset( axis, 0, sizeof(int) * numAxis);
      memset( buttons, 0, sizeof(int) * numButtons);

      syslog(LOG_INFO, "Joystick detected: %s\n", joy_name);
      syslog(LOG_INFO, "Axis: %d, Buttons: %d\n", numAxis, numButtons);
    }
  }
  return GetFileDescript();
}

/* ======================== */
void JoyStickDriver::Run(void)
{
  struct js_event js;
  int rv;

  if ( Connect() < 0 ) {
    return;
  }
  rv = read(file_fd, &js, sizeof(struct js_event));
  if ( rv < 0 ) {
    ClosePort();
    return;
  }

  switch (js.type & ~JS_EVENT_INIT) {
    case JS_EVENT_AXIS:
      if ( abs(js.value) < DeadZone ) {
        js.value = 0;
      }
      if ( js.number < numAxis ) {
        axis[ js.number ] = js.value;
      }
      break;

    case JS_EVENT_BUTTON:
      if ( js.number < numButtons ) {
        buttons [ js.number ] = js.value;
      }
      break;
  }
}

/* ======================== */
int JoyStickDriver::GetNumAxis(void)
{
  return numAxis;
}

/* ------------------------ */
int JoyStickDriver::GetAxis(int axis_num)
{
  if ( axis_num < numAxis )
    return axis[axis_num];
  return 0;
}

/* ======================== */
int JoyStickDriver::GetNumButton(void)
{
  return numButtons;
}

/* ------------------------ */
int JoyStickDriver::GetButton(int button_num)
{
  if ( button_num < numButtons )
    return buttons [ button_num ];
  return 0;
}

/* ======================== */
void JoyStickDriver::SetDeadzone(int value)
{
  if ( value < 0 ) {
    // positive numbers only!
    value *= -1;
  }
  DeadZone = (37262 * value) / 100;
}

/* ======================== */
/* ======================== */

