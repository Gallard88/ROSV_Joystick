using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <syslog.h>

#include "parson.h"
#include "JoyStick.hpp"
#include "ControlModel.h"

// -----------------------------
static JoyStickDriver *Joy;
static ControlMode *Control;

// -----------------------------
static void ReadSettings(void)
{
  JSON_Value *val = json_parse_file("/etc/ROSV_Joystick.json");
  if ( val == NULL ) {
    syslog(LOG_EMERG, "JSON failed to open");
    exit(-1);
  }
  int rv = json_value_get_type(val);
  if ( rv != JSONObject ) {
    syslog(LOG_EMERG, "JSON parse failed");
    exit( -1);
  }

  JSON_Object *settings = json_value_get_object(val);
  if ( settings == NULL ) {
    syslog(LOG_EMERG, "JSON settings == NULL");
    exit( -1);
  }
  const char *name = json_object_get_string(settings, "Joystick");
  if ( name == NULL ) {
    syslog(LOG_EMERG, "JSON settings: no Joystick");
    exit(-1);
  }
  Joy = new JoyStickDriver(name);

  name = json_object_get_string(settings, "Server");
  if ( name == NULL ) {
    syslog(LOG_EMERG, "JSON settings: No server selected");
    exit(-1);
  }
  Control = new ControlMode(name, 8090);
}

// -----------------------------
static void Send_Packet(void)
{
  Control->SetVectorRaw(VEC_FORWARD, ((float)Joy->GetAxis(1) * -100) / 32767);
  Control->SetVectorRaw(VEC_STRAFE, (Joy->GetAxis(0) * 100) / 32767);
  Control->SetVectorRaw(VEC_DEPTH, ((Joy->GetAxis(3) * -1) + 32767) / 655);
  Control->SetVectorRaw(VEC_TURN, (Joy->GetAxis(2) * 100) / 32767);
  Control->SendVectorUpdate();
}

// -----------------------------
static void ReadData(void)
{
  fd_set readFD;
  struct timeval timeout;

  timeout.tv_sec = 0;
  timeout.tv_usec = 100000;

  FD_ZERO(&readFD);

  FD_SET(Control->GetFD(), &readFD);
  FD_SET(Joy->GetFileDescript(), &readFD);
  int max = (Control->GetFD() > Joy->GetFileDescript())? Control->GetFD(): Joy->GetFileDescript();


  if ( select(max+1, &readFD, NULL, NULL, &timeout) > 0 ) {
    if ( FD_ISSET(Control->GetFD(), &readFD) ) {
      Control->Run();
    }
    if ( FD_ISSET(Joy->GetFileDescript(), &readFD) ) {
      Joy->Run();
    }
  }
}

// -----------------------------
int main (int argc, char *argv[])
{

  // open settings file and read data.
  ReadSettings();

  openlog("ROSV_Joystick", LOG_PID, LOG_USER);
  syslog(LOG_NOTICE, "ROSV_Joystick online");

  // ------------------------------------
  if ( daemon( 1, 0 ) < 0 ) { // keep dir
    syslog(LOG_EMERG, "daemonise failed");
    return -1;
  }

  // run main logic.
  while ( 1 ) {

    // If NOT TCP
    // Try to connect
    if ( Control->GetFD() < 0 ) {
      Control->Connect();
    }

    // If not joystick
    // Try to connect.
    while (( Joy->GetFileDescript() >= 0 ) &&
           ( Control->GetFD() >= 0 )) {

      // read data from both.
      ReadData();

      // Read Data
      // Update Model
      // Send new data
      Send_Packet();
    }

    // if we get here we have either lost the joystick OR
    // We can no longer talk to the ROSV.
    // Best option is to close the ROSV socket, and go back to polling.
    if ( Control->GetFD() >= 0 ) {
      syslog(LOG_NOTICE, "ROSV Connection lost");
      Control->Disconnect();
    }
    sleep (120);
  }
  return 0;
}
