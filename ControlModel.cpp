#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <syslog.h>

#include "ControlModel.h"
#include "TCP_Client.hpp"

using namespace std;

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


ControlMode::ControlMode(const string & server, int port):
  RosvFd(-1), Server(server), Port(port)
{
  memset(VectorRaw, 0, sizeof(VectorRaw));
}

ControlMode::~ControlMode()
{
  syslog(LOG_NOTICE, "Control Mode destructor");
  Disconnect();
}

void ControlMode::Connect(void)
{
  if ( RosvFd < 0 ) {
    syslog(LOG_NOTICE, "Connect: %s:%d", Server.c_str(), Port);
    RosvFd = connect((const char *)Server.c_str(), Port);
    if ( RosvFd >= 0 ) {
      syslog(LOG_NOTICE, "Connected to %s", Server.c_str());
      SendClientId();
    }
  }
}

void ControlMode::Disconnect(void)
{
  if ( RosvFd >= 0 ) {
    syslog(LOG_NOTICE, "Control Mode Disconnect");
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
const string ClientIdInfo =
"{ \"Packet\":\"ClientId\", \"ProtVer\":\"0.1\", \"Name\":\"ROSV_Joystick\" }\r\n";

void ControlMode::SendClientId(void)
{
  int rv = write(RosvFd, ClientIdInfo.c_str(), ClientIdInfo.length());
  if ( rv < 0 ) {
    syslog(LOG_ALERT, "Write Error");
    Disconnect();
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
    sprintf(msg,"{ \"Module\":\"Navigation\", \"Packet\":\"SetVector\", \"Ch\":\"%s\", \"Mode\":\"Raw\", \"Value\": %2.2f }\r\n", VecName[i], (float)VectorRaw[i]);
    rv = write(RosvFd, msg, strlen(msg));
    if ( rv < 0 ) {
      syslog(LOG_ALERT, "Vecotr Update Error");
      Disconnect();
    }
  }
}

// ====================================================
// ====================================================
