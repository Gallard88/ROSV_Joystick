#ifndef __JMAP_XBOX__
#define __JMAP_XBOX__

#include "ControlModel.h"
#include "JoyStick.hpp"


class JMap_Xbox: public ControlUpdate {

public:
  JMap_Xbox(JoyStickDriver *driver);
  float GetVectorValue(ControlVectors vector);
  void Run_Task(void);


private:
  JoyStickDriver *Joy;
  int Depth;
  bool IncEdge;
  bool DecEdge;

  void CalcDepth(void);

};


#endif
