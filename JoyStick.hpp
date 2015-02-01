#ifndef __JOYSTICK_HPP__
#define __JOYSTICK_HPP__

#include <string>

class JoyStickDriver {

private:
  std::string Device;
  int DeadZone;
  int file_fd;

  std::string Name;
  int num_Axis;
  int num_Buttons;

  int *buttons;
  int *axis;

  void ClosePort(void);

public:
  JoyStickDriver(const std::string & device);
  ~JoyStickDriver();

  std::string GetName() {
    return Name;
  }

  int Connect(void);
  int GetFileDescript(void) {
    return file_fd;
  }

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




