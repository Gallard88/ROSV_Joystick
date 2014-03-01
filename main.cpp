using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <syslog.h>

#include "parson.h"
#include "JoyStick.hpp"
#include "TCP_Client.hpp"

// -----------------------------
static JoyStickDriver Joy;
static string Server;
static int TCP_fd;

// -----------------------------
static void ReadSettings(void)
{
  JSON_Value *val = json_parse_file("ROSV_Joystick.json");
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
  Joy.Connect(name);

  name = json_object_get_string(settings, "Server");
  if ( name == NULL ) {
    syslog(LOG_EMERG, "JSON settings: No server selected");
    exit(-1);
  }
  Server = string(name);
}

// -----------------------------
#define NUM_VECTORS	4
/* 	0	Forward
 * 	1	Side
 * 	2	Depth
 * 	3	Yaw
 */
// -----------------------------
void Send_Vector(void)
{
  int vector[NUM_VECTORS];
  char buf[32];
  string msg;

  vector[0] = (Joy.GetAxis(1) * -100) / 32767;
  vector[1] = (Joy.GetAxis(0) * 100) / 32767;
  vector[2] = ((Joy.GetAxis(3) * -1) + 32767) / 655;
  vector[3] = (Joy.GetAxis(2) * 100) / 32767;

  msg = "{ \"Module\":\"Motor\", ";
  msg += " \"RecordType\":\"Velocity\", ";
  msg += "\"Vector\": [";
  for ( int i = 0; i < NUM_VECTORS; i ++ ) {
    sprintf(buf, " %d", vector[i]);
    msg += string(buf);
    if ( i != (NUM_VECTORS-1) ) {
      msg += ",";
    }
  }
  msg += "]}\r\n";
  int rv = write(TCP_fd, msg.c_str(), msg.length());
  if ( rv < 0 ) {
    close(TCP_fd);
    TCP_fd = -1;
  }
}

// -----------------------------
static void Log_ReceivedData(void)
{
  fd_set readFD;
  struct timeval timeout;
  char buffer[4096];
  int rv;

  timeout.tv_sec = 0;
  timeout.tv_usec = 100;

  FD_ZERO(&readFD);
  FD_SET(TCP_fd, &readFD);

  if ( select(TCP_fd+1, &readFD, NULL, NULL, &timeout) > 0 ) {
    if ( FD_ISSET(TCP_fd, &readFD) ) {
      rv = read(TCP_fd, buffer, sizeof(buffer));
      if ( rv < 0 ) {
        close(TCP_fd);
        TCP_fd = -1;
        return;
      }
      buffer[rv] = 0;	// terminate buffer
      FILE *fp = fopen("/var/log/ROSV_Joystick", "a+");
      if ( fp != NULL ) {
        int length = fwrite( buffer, 1, rv, fp);
        if ( length < rv ) {
          syslog(LOG_WARNING, "Rx: Incomplete write: %d vs %d", length, rv);
        }
        fclose(fp);
      }
    }
  }
}

// -----------------------------
int main (int argc, char *argv[])
{

  // open settings file and read data.
  ReadSettings();
  TCP_fd = -1;

  openlog("ROSV_Joystick", LOG_PID, LOG_USER);
  syslog(LOG_NOTICE, "ROSV_Joystick online");

// ------------------------------------
  if ( daemon( 1, 0 ) < 0 ) { // keep dir
    syslog(LOG_EMERG, "daemonise failed");
    return -1;
  }
  Joy.Run();
  if ( !Joy.IsConnected() ) {
    syslog(LOG_ALERT, "No joystick found attached on startup");
  }

  // run main logic.
  while ( 1 ) {

    // Run Joystick.
    Joy.Run();
    if ( !Joy.IsConnected() ) {
      sleep(10);
      continue;
    }
    if ( TCP_fd < 0 ) {
      TCP_fd = connect((const char *)Server.c_str(), 8090);
      if ( TCP_fd > 0 ) {
        syslog(LOG_NOTICE, "Connected to %s", Server.c_str());
      }
      continue;
    }

    if ( Joy.NewData()) {
      Send_Vector();
    }
    Log_ReceivedData();
  }
  return 0;
}
