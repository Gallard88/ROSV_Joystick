#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <netdb.h>
#include <fcntl.h>
#include <syslog.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include "ControlModel.h"

using namespace std;

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
  Connect();
}

ControlMode::~ControlMode()
{
  Disconnect();
}

void ControlMode::Connect(void)
{
  if ( RosvFd >= 0 ) {
    return;
  }
  struct sockaddr_in serv_addr;

  if((RosvFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    syslog(LOG_CRIT, "Error : Could not create socket");
    exit(-1);
  }

  memset(&serv_addr, '0', sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(Port);

  if(inet_pton(AF_INET, (const char *)Server.c_str(), &serv_addr.sin_addr)<=0) {
    syslog(LOG_CRIT, "inet_pton error occured");
    exit(-1);
  }
  if( connect(RosvFd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    RosvFd = -1;
    return ;
  }
  syslog(LOG_NOTICE, "Client: connected %s", Server.c_str());
  SendClientId();
}

void ControlMode::Disconnect(void)
{
  if ( RosvFd >= 0 ) {
    syslog(LOG_NOTICE, "Client: Disconnected");
    close(RosvFd);
    RosvFd = -1;
  }
}

void ControlMode::Run_Task(void)
{
  if ( RosvFd < 0 ) {
    Connect();

  } else {
    int rv;

    do {
      rv = Read_Data();

    } while ( rv > 0 );

    SendVectorUpdate();
  }
}

// ====================================================
int ControlMode::Read_Data(void)
{
  fd_set readFD;
  struct timeval timeout;
  timeout.tv_sec  = 0;
  timeout.tv_usec = 1;

  FD_ZERO(&readFD);
  FD_SET(RosvFd, &readFD);

  if ( select(RosvFd+1, &readFD, NULL, NULL, &timeout) > 0 ) {

    if ( FD_ISSET(RosvFd, &readFD) ) {
      char buffer[2048];
      int  rv = read(RosvFd, buffer, sizeof(buffer));

      if ( rv <= 0 ) {
        Disconnect();
        return 0;
      }
      return 1;
    }
  }
  return 0;
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
  char msg[256];

  if( RosvFd < 0 ) {
    return;
  }
  for ( int i = 0; i < NUM_VECTORS; i ++ ) {
    float power = Callback->GetVectorValue((ControlVectors) i);

    sprintf(msg,"{ \"Module\":\"Navigation\", \"Packet\":\"SetVector\", \"Ch\":\"%s\", \"Mode\":\"Raw\", \"Value\": %2.2f }\r\n", VecName[i], power);

    int rv = write(RosvFd, msg, strlen(msg));
    if ( rv < 0 ) {
      syslog(LOG_ALERT, "Vector Update Error");
      Disconnect();
    }
  }
}

// ====================================================
// ====================================================
