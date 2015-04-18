#ifndef __CONTROL_MODEL__
#define __CONTROL_MODEL__

#include <string>
#include <RTT_Interface.h>

#define NUM_VECTORS 4

typedef enum {

  vecForward = 0,
  vecStrafe = 1,
  vecDepth = 2,
  vecTurn = 3

} ControlVectors;

class ControlUpdate: public RealTime::Task_Interface {

public:
  virtual float GetVectorValue(ControlVectors vector) = 0;

};

class ControlMode: public RealTime::Task_Interface {


public:

  ControlMode(const std::string & server, int port);
  ~ControlMode();
  void Connect(void);
  void Disconnect(void);

  void SetCallback(ControlUpdate *cb) {
    Callback = cb;
  }

  void Run_Task(void);
  int GetFD(void) {
    return RosvFd;
  }

private:
  int RosvFd;
  std::string Server;
  int Port;

  ControlUpdate *Callback;

  void SendClientId(void);
  void SendVectorUpdate(void);
  int Read_Data(void);

};

#endif

