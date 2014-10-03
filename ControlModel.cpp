/*


*/
using namespace std;

#include "ControlModel.h"
#include "TCP_Client.hpp"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

#define NUM_VECTORS 4
/*  0 Forward
 *  1 Side
 *  2 Depth
 *  3 Yaw
 */
// -----------------------------
static const char *VecName[NUM_VECTORS] = {
  "Forward",
  "Strafe",
  "Dive",
  "Turn"
};


ControlMode::ControlMode(const char *server, int port)
{
  RosvFd = -1;
  memset(VectorRaw, 0, sizeof(VectorRaw));
  Server = new char[strlen(server)];
  strcpy(Server, server);
  Port = port;
}

ControlMode::~ControlMode()
{
  Disconnect();
  delete Server;
}

void ControlMode::Connect(void)
{
  if ( RosvFd < 0 ) {
    syslog(LOG_NOTICE, "Connect: %s:%d", Server, Port);
    RosvFd = connect((const char *)Server, Port);
    if ( RosvFd >= 0 ) {
      syslog(LOG_NOTICE, "Connected to %s", Server);
      SendClientId();
    }
  }
}

void ControlMode::Disconnect(void)
{
  if ( RosvFd >= 0 ) {
    close(RosvFd);
    RosvFd = -1;
  }
}


void ControlMode::SetVectorRaw(int vec, float power)
{
  if ( vec < NUM_VECTORS ) {
    VectorRaw[vec] = power;
  }
}

//  void IncDepthPrec(void);
//  void DecDepthPrec(void);

void ControlMode::Run(void)
{
  char buffer[2048];
  int  rv = read(RosvFd, buffer, sizeof(buffer));
  if ( rv < 0 ) {
    close(RosvFd);
    RosvFd = -1;
  } else {
    buffer[rv] = 0; // terminate buffer
    FILE *fp = fopen("/var/log/ROSV_Joystick", "a+");
    if ( fp != NULL ) {
      fwrite( buffer, 1, rv, fp);
      fclose(fp);
    }
  }
}

// ====================================================
void ControlMode::SendClientId(void)
{
  char msg[256];

  strcpy(msg,"{ \"Packet\":\"ClientId\", \"ProtVer\":\"0.1\", \"Name\":\"ROSV_Joystick\" }\r\n");
  int rv = write(RosvFd, msg, strlen(msg));
  if ( rv < 0 ) {
    Disconnect();
    return;
  }
}

// ====================================================
void ControlMode::SendVectorUpdate(void)
{
  int i, rv;
  char msg[256];
  if( RosvFd < 0 ) {
   return;
  }
  for ( i = 0; i < NUM_VECTORS; i ++ ) {
    sprintf(msg,"{ \"Packet\":\"SetVector\", \"Ch\":\"%s\", \"Mode\":\"Raw\", \"Value\": %2.2f }\r\n", VecName[i], (float)VectorRaw[i]);
    rv = write(RosvFd, msg, strlen(msg));
    if ( rv < 0 ) {
      Disconnect();
      return;
    }
  }
}

// ====================================================
// ====================================================
