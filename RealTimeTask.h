#ifndef __REAL_TIME_TASK__
#define __REAL_TIME_TASK__

/***
	RealTimeTaskManager

	A container for managing how often a module should be run.
	Currently offers millisecond resolution.
	Capable of reporting if a deadline has been missed.
	Each task will be run once per period.
	It might be useful to add a function that will return
	how long until the start of the next period.
 */

#include "RTT_Interface.h"
#include <string>

class RealTimeTask {

public:

  RealTimeTask(const string & name, RTT_Interface * task);

  /*
   * SetFrequency()/SetPeriod()
   *
   * Calculates how often this task should be run.
   * SetFrequency()
   * Operates in hertz, useful for tasks that should be run quickly. Min 1Hz, Max 1000Hz.
   *
   * SetPeriod()
   * Sets a period of X ms. Useful for slower tasks. Min 1ms.
   */
  void SetPeriod(int ms);
  void SetFrequency(int hz);

  /*
   * Run()
   * Called at full rate.
   * This function will internally decide when the underlying module should be run.
   */

  void Run(void);

  /*
   *  Returns 1 on the first edge of
   */
  bool DetectDeadlineEdge(void);
  const string & GetName(void) {
    return Name;
  }

private:

  string Name;
  RTT_Interface * Task;
  bool DeadlineMissed;
  bool LastDeadline;
  int Period;
  unsigned long NextEvent;

  void SetNextEvent(void);
};

#endif

