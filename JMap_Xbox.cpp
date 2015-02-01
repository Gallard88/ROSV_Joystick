#include <unistd.h>
#include <fcntl.h>

#include "JMap_Xbox.h"

using namespace std;

/*
 * Generix X-Box pad.
 * 8x Axis
 * 11x Buttons
 *
 */

JMap_Xbox::JMap_Xbox(JoyStickDriver *driver):
  Joy(driver), Depth(0),
  IncEdge(false), DecEdge(false)
{
}

float JMap_Xbox::GetVectorValue(ControlVectors vector)
{
  switch ( vector ) {
  case vecForward:
    return (Joy->GetAxis(1) * -100.0) / 32767;

  case vecTurn:
    return (Joy->GetAxis(0) * 100) / 32767;

  case vecStrafe:
    return (Joy->GetAxis(3) * 100.0) / 32767;

  case vecDepth:
    return (float)Depth;
  }
  return 0;
}

void JMap_Xbox::Run_Task(void)
{
  int rv = 1;

  while ( rv > 0 ) {
    fd_set readFD;
    struct timeval timeout;

    timeout.tv_sec = 0;
    timeout.tv_usec = 1;

    int j_fd = Joy->GetFileDescript();

    FD_ZERO(&readFD);
    FD_SET(j_fd, &readFD);

    rv = select(j_fd+1, &readFD, NULL, NULL, &timeout);
    if ( rv > 0 ) {
      if ( FD_ISSET(j_fd, &readFD) ) {
        Joy->Run();
      }
    }
    CalcDepth();
  }
}

void JMap_Xbox::CalcDepth(void)
{
  bool edge = Joy->GetButton(5);
  if (( IncEdge == false ) &&
      ( edge    == true ) &&
      ( Depth   <  100 )) {

    Depth += 10;
  }
  IncEdge = edge;

  edge = Joy->GetButton(4);
  if (( DecEdge == false ) &&
      ( edge    == true ) &&
      ( Depth   >  0 )) {

    Depth -= 10;
  }
  DecEdge = edge;
}


