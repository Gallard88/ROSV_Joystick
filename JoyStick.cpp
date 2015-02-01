#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>

#include "JoyStick.hpp"
/* ======================== */
using namespace std;

/* ======================== */
JoyStickDriver::JoyStickDriver(const string & device):
  Device(device), DeadZone(10), file_fd(-1),
  num_Axis(-1), num_Buttons(-1)

{
  buttons = axis = NULL;
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
  num_Buttons = -1;
  num_Axis = -1;

  if ( file_fd >= 0 ) {
    close(file_fd);
    file_fd = -1;
  }
}

/* ======================== */
int  JoyStickDriver::Connect(void)
{
  if ( file_fd < 0 ) {
    file_fd = open( Device.c_str() , O_RDONLY);
    if ( file_fd >= 0 ) {
      char joy_name[100];
      ioctl( file_fd, JSIOCGNAME(80), &joy_name );
      Name = string(joy_name);

      char number;
      ioctl( file_fd, JSIOCGAXES,    &number );
      num_Axis = number;

      ioctl( file_fd, JSIOCGBUTTONS, &number );
      num_Buttons = number;

      if ( num_Axis < 0 || num_Buttons < 0 ) {
        return -1;
      }

      axis = new int [num_Axis];
      buttons = new int [num_Buttons];

      memset( axis, 0, sizeof(int) * num_Axis);
      memset( buttons, 0, sizeof(int) * num_Buttons);

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
    if ( js.number < num_Axis ) {
      axis[ js.number ] = js.value;
//      printf("Axis: %d:%d\n", js.number, js.value);
    }
    break;

  case JS_EVENT_BUTTON:
    if ( js.number < num_Buttons ) {
      buttons [ js.number ] = js.value;
//      printf("Button: %d:%d\n", js.number, js.value);
    }
    break;
  }
}

/* ======================== */
int JoyStickDriver::GetNumAxis(void)
{
  return num_Axis;
}

/* ------------------------ */
int JoyStickDriver::GetAxis(int axis_num)
{
  if ( axis_num < num_Axis )
    return axis[axis_num];
  return 0;
}

/* ======================== */
int JoyStickDriver::GetNumButton(void)
{
  return num_Buttons;
}

/* ------------------------ */
int JoyStickDriver::GetButton(int button_num)
{
  if ( button_num < num_Buttons )
    return buttons [ button_num ];
  return 0;
}

/* ======================== */
void JoyStickDriver::SetDeadzone(int value)
{
  DeadZone = (37262 * abs(value)) / 100;
}

/* ======================== */
/* ======================== */

