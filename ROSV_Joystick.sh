#!/bin/bash
# Provides:          ROSV_Joystick
# Required-Start:    $syslog
# Required-Stop:     $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Example ROSV_Joystick
# Description:       Start/Stops ROSV_Joystick

start() {
  # Start TFTP Server
  echo "ROSV_Joystick: Start"
  /usr/local/bin/ROSV_Joystick
}

stop() {
  # Stop TFTP_Server
  echo "ROSV_Joystick: Stop"
  pkill ROSV_Joystick
}

case "$1" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    retart)
        stop
        start
        ;;
    *)
echo "Usage: $0 {start|stop|restart}"
esac
exit 0
