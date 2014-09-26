#ifndef __CONTROL_MODEL__
#define __CONTROL_MODEL__

#define NUM_VECTORS 4

#define VEC_FORWARD	0
#define VEC_STRAFE	1
#define VEC_DEPTH		2
#define VEC_TURN		3

class ControlMode {

public:

  ControlMode(const char *server, int port);
  ~ControlMode();
  void Connect(void);
  void Disconnect(void);

  void SetVectorRaw(int vec, float power);

//  void IncDepthPrec(void);
//  void DecDepthPrec(void);

  void Run(void);
  void Send(void);
  int GetFD(void) { return RosvFd; }

private:
  int RosvFd;
  float VectorRaw[NUM_VECTORS];
  char *Server;
  int Port;


};

#endif

