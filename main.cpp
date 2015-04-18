#include <cstdio>
#include <cstdlib>
#include <cstring>
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

//
#include "JMap_Xbox.h"

using namespace std;
using namespace RealTime;

// -----------------------------
const int RunRate = 10;  // Hz

// -----------------------------
static JoyStickDriver * Joy;
static ControlMode *Control;
static RT_TaskManager TaskMan;
static ControlUpdate *CUpdate;

// -----------------------------
ControlUpdate *CreateFactory(JoyStickDriver * joy)
{
  string name = joy->GetName();
  if ( name == "Generic X-Box pad" ) {
    return new JMap_Xbox(joy);
  }
  return NULL;
}

// -----------------------------
static void Create_JoystickDriver(const char *name, int deadzone)
{
  Joy = new JoyStickDriver(string(name));
  Joy->SetDeadzone(deadzone);

  if ( Joy->Connect() < 0 ) {
    syslog(LOG_NOTICE, "Failed to connect to joystick.");
    exit(-1);
  }

  syslog(LOG_INFO, "Joystick: %s, A%d:B%d\n", Joy->GetName().c_str(), Joy->GetNumAxis(), Joy->GetNumButton());

  CUpdate = CreateFactory(Joy);
  if ( CUpdate == NULL ) {
    syslog(LOG_INFO, "Joystick unrecognised");
    exit(-1);
  }

  RealTimeTask *joyTask = new RealTimeTask("Joystick", (Task_Interface *)CUpdate);
  joyTask->SetFrequency(RunRate);
  joyTask->SetMaxDuration_Ms(5);
  TaskMan.AddTask(joyTask);
}

// -----------------------------
void Start_Client(const char *url)
{
  Control = new ControlMode(string(url), 8090);
  Control->SetCallback(CUpdate);

  RealTimeTask *comsTask = new RealTimeTask("Control", (Task_Interface *)Control);
  comsTask->SetFrequency(RunRate);
  comsTask->SetMaxDuration_Ms(5);
  TaskMan.AddTask(comsTask);
}

// -----------------------------
static void Init_System(void)
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
  int deadzone = (int) json_object_get_number(settings, "Deadzone");
  Create_JoystickDriver(name, ( deadzone <= 0 )? 5: deadzone);

  name = json_object_get_string(settings, "Server");
  if ( name == NULL ) {
    syslog(LOG_EMERG, "JSON settings: No server selected");
    exit(-1);
  }
  Start_Client(name);
}

// -----------------------------
class Main_RT: public Reporting_Interface {
public:
  void Deadline_Missed(const std::string & name) {
    syslog(LOG_ALERT, "%s: Duration Missed", name.c_str());
  }
  void Deadline_Recovered(const std::string & name) {
    syslog(LOG_ALERT, "%s: Deadline Recovered", name.c_str());
  }
  void Duration_Overrun(const std::string & name) {
    syslog(LOG_ALERT, "%s: Duration Overrun", name.c_str());
  }
  void Statistics(const std::string & name, RealTimeTask::Statistics_t stats) {
    syslog(LOG_ALERT, "%s: Min, %u", name.c_str(), stats.Min);
    syslog(LOG_ALERT, "%s: Max, %u", name.c_str(), stats.Max);
    syslog(LOG_ALERT, "%s: Avg, %u", name.c_str(), stats.Avg);
    syslog(LOG_ALERT, "%s: Called, %u", name.c_str(), stats.Called);
  }

};

// -----------------------------
int main (int argc, char *argv[])
{
  openlog("ROSV_Joystick", LOG_PID, LOG_USER);
  syslog(LOG_NOTICE, "ROSV_Joystick online");

  int opt;

  while ((opt = getopt(argc, argv, "dD:")) != -1) {
    switch(opt) {
    case 'd':
      syslog(LOG_EMERG, "Becomming daemon");
      if ( daemon( 1, 0 ) < 0 ) { // keep dir
        syslog(LOG_EMERG, "daemonise failed");
        return -1;
      }
      break;
    }
  }
  // ------------------------------------
  Init_System();
  TaskMan.AddCallback((Reporting_Interface *) new Main_RT());

  // ------------------------------------
  syslog(LOG_NOTICE, "Starting main application");

  while ( 1 ) {
    long time = TaskMan.RunTasks();
    if ( time > 0 ) {
      usleep(time*1000);
    }
  }
  return 0;
}





