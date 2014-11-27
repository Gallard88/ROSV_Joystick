// ----------------------------------

using namespace std;

#include <sys/time.h>

#include "RealTimeTask.h"

// ----------------------------------
const int DEFAULT_PERIOD = 1000; // milliseconds, 1Hz.
const int MAX_HERTZ = 1000;      // largest allowed frequency.
const int MIN_PERIOD = 1;        // milliseconds, 1000Hz.
// smallest allowed period.

// ----------------------------------
RealTimeTask::RealTimeTask(const string & name, RTT_Interface * task):
  Name(name), Task(task), Period(DEFAULT_PERIOD)
{
  SetNextEvent();
}

void RealTimeTask::SetNextEvent(void)
{
  struct timeval  tv;
  gettimeofday(&tv, NULL);
  NextEvent = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; // convert tv_sec & tv_usec to millisecond
}

void RealTimeTask::Run(void)
{
  struct timeval  tv;
  gettimeofday(&tv, NULL);
  unsigned long current = (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000 ; // convert tv_sec & tv_usec to millisecond

  if (( current  >= NextEvent ) && ( Task != NULL )) {
    Task->Run_Task();
    NextEvent += Period; // update the base time.
  }

  // Missed period detection.
  // Once we update the base timer, it should now be ahead of the current time.
  // If it is not, then we know we have missed a deadline at some point. If this is
  // a transient thing, the module will catch up, otherwise it will continue to fall
  // behind, and action must be taken.
  // Once we drag NextEvent forward, current should be less than it.
  // If it is not, then it is a sign that we have missed at least one deadline.
  DeadlineMissed = ( current > NextEvent )? true: false;
}



/*
 * Here we do edge detection. This is not tied to a specific call rate.
 * It will simply report true on the first edge of missing a deadline.
 * This is useful for limiting error reporting to when the event occurs.
 */

bool RealTimeTask::DetectDeadlineEdge(void)
{
  bool rv = false;

  if (( DeadlineMissed == true ) && ( LastDeadline == false )) {
    rv = true;
  }
  LastDeadline = DeadlineMissed;
  return rv;
}

/*
 *  When we update the period, there is a chance it will report a missed
 *  deadline based on what the previous Period was. Thus for safety when
 *  we update the Period, we also update the NextEvent variable.
 */

void RealTimeTask::SetPeriod(int ms)
{
  Period = ( ms < MIN_PERIOD ) ? MIN_PERIOD: ms;
  SetNextEvent();
}

void RealTimeTask::SetFrequency(int hz)
{
  if ( hz <= 0 ) {
    Period = DEFAULT_PERIOD;
  } else  if ( hz >= MAX_HERTZ ) {
    Period = MIN_PERIOD;
  } else {
    Period = 1000 / hz;
  }
  SetNextEvent();
}


