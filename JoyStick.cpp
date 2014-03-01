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

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>
#include <string>
#include <iostream>
#include <string.h>
#include <string>
#include <syslog.h>

#include "JoyStick.hpp"
/* ======================== */
#define NUM_AXIS	6
#define NUM_BUT		10

/* ======================== */
JoyStickDriver::JoyStickDriver()
{
  file_fd = -1;
}

/* ======================== */
JoyStickDriver::~JoyStickDriver()
{
  delete [] axis;
  delete [] buttons;

  close(file_fd);
}

/* ======================== */
void JoyStickDriver::Connect(const char *device)
{
  DeviceName = string(device);
}

/* ======================== */
int JoyStickDriver::IsConnected(void)
{
  return ( file_fd >= 0) ? 1: 0;
}

/* ======================== */
void JoyStickDriver::OpenPort(void)
{
  if ( !IsConnected()) {

    if( ( file_fd = open( DeviceName.c_str() , O_RDONLY)) < 0 ) {
      syslog(LOG_WARNING, "Couldn't open joystick\n" );
      return;
    }
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

/* ======================== */
void JoyStickDriver::ClosePort(void)
{
  delete axis;
  delete buttons;
  numButtons = 0;
  numAxis = 0;
  close(file_fd);
  file_fd = -1;
}

/* ======================== */
bool JoyStickDriver::NewData(void)
{
  bool rv;

  rv = Data_Ready;
  Data_Ready = false;
  return rv;
}


/* ======================== */
void JoyStickDriver::Run(void)
{
  struct js_event js;
  int rv;

  if ( file_fd < 0 ) {
    OpenPort();
    return;
  }

  fd_set readFD;
  struct timeval timeout;

  timeout.tv_sec = 0;
  timeout.tv_usec = 100000;

  FD_ZERO(&readFD);
  FD_SET(file_fd, &readFD);

  if ( select(file_fd+1, &readFD, NULL, NULL, &timeout) > 0 ) {
    if ( FD_ISSET(file_fd, &readFD) ) {
      rv = read(file_fd, &js, sizeof(struct js_event));
      if ( rv < 0 ) {
        ClosePort();
        return;
      }


      switch (js.type & ~JS_EVENT_INIT) {
      case JS_EVENT_AXIS:
        if ( js.number < numAxis ) {
          Data_Ready = true;
          axis[ js.number ] = js.value;
        }
        break;

      case JS_EVENT_BUTTON:
        if ( js.number < numButtons ) {
          Data_Ready = true;
          buttons [ js.number ] = js.value;
        }
        break;
      }
    }
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
/* ======================== */
/* ======================== */

