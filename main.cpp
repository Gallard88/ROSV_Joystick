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
#include <time.h>

#include <RT_TaskManager.h>
#include <RTT_Interface.h>
#include <RealTimeTask.h>

#include "parson.h"
#include "JoyStick.hpp"
#include "ControlModel.h"

// -----------------------------
#define UPDATE_RATE_HZ(x)	(1000000 / x)

// -----------------------------
static JoyStickDriver *Joy;
static ControlMode *Control;
static RT_TaskManager TaskMan;

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

  int deadzone = (int) json_object_get_number(settings, "Deadzone");
  if ( deadzone <= 0 ) {
    deadzone = 5;
  }
  Joy->SetDeadzone(deadzone);

  name = json_object_get_string(settings, "Server");
  if ( name == NULL ) {
    syslog(LOG_EMERG, "JSON settings: No server selected");
    exit(-1);
  }
  Control = new ControlMode(name, 8090);
}

// -----------------------------
class MainTask: public RTT_Interface {

public:
  void Run_Task(void);

};

void MainTask::Run_Task(void)
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
  timeout.tv_usec = UPDATE_RATE_HZ(10);

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

class Main_RT: public RT_TaskMan_Interface {
public:
  void Deadline_Missed(const std::string & name)
  {
    syslog(LOG_EMERG, "%s: Duration Missed", name.c_str());
  }
  void Deadline_Recovered(const std::string & name)
  {
    syslog(LOG_EMERG, "%s: Deadline Recovered", name.c_str());
  }

  void Duration_Overrun(const std::string & name)
  {
    syslog(LOG_EMERG, "%s: Duration Overrun", name.c_str());
  }
};

// -----------------------------
int main (int argc, char *argv[])
{
  RealTimeTask *comsTask = new RealTimeTask("Main", new MainTask());
  comsTask->SetFrequency(5);
  comsTask->SetMaxDuration(10);
  TaskMan.AddTask(comsTask);
  TaskMan.AddCallback((RT_TaskMan_Interface *) new Main_RT());

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

      // Read Data,  Update Model, Send new data
      TaskMan.RunTasks();
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
