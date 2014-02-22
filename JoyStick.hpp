

/* ======================== */
#ifndef __JOYSTICK_HPP__
#define __JOYSTICK_HPP__
/* ======================== */

#include <string>

class JoyStickDriver {
private:
  char joy_name[256];
  int num_Axis;
  int file_fd;

  int *buttons;
  int numButtons;

  int *axis;
  int numAxis;

  void OpenPort(void);
  void ClosePort(void);
  bool Data_Ready;


public:
  JoyStickDriver();
  ~JoyStickDriver();

  void Connect(const char *device);
  int IsConnected(void);

  void Run(void);
  bool NewData(void);

  int GetNumAxis(void);
  int GetAxis(int axis_num);

  int GetNumButton(void);
  int GetButton(int button_num);

  string DeviceName;
};

/* ======================== */
/* ======================== */
#endif




