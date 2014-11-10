

/* ======================== */
#ifndef __JOYSTICK_HPP__
#define __JOYSTICK_HPP__
/* ======================== */

class JoyStickDriver {
private:
  char *Device;
  char joy_name[256];
  int num_Axis;
  int file_fd;

  int *buttons;
  int numButtons;

  int *axis;
  int numAxis;

  int DeadZone;

  void ClosePort(void);

public:
  JoyStickDriver(const char *device);
  ~JoyStickDriver();

  int Connect(void);
  int GetFileDescript(void) { return file_fd; }

  void Run(void);
  void SetDeadzone(int value);

  int GetNumAxis(void);
  int GetAxis(int axis_num);

  int GetNumButton(void);
  int GetButton(int button_num);
};

/* ======================== */
/* ======================== */
#endif




