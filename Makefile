all: ROSV_Joystick

FLAGS=-Wall -O2 --pedantic
LDADD=\
  /usr/local/lib/libparson.a \
  /usr/local/lib/libRealTime.a

CC=g++

ROSV_Joystick: $(LDADD) JoyStick.o main.o ControlModel.o JMap_Xbox.o
	$(CC) $(FLAGS) -o ROSV_Joystick JoyStick.o ControlModel.o JMap_Xbox.o main.o $(LDADD)

main.o: main.cpp
	$(CC) -c $(FLAGS) main.cpp

ControlModel.o: ControlModel.cpp ControlModel.h
	$(CC) -c $(FLAGS) ControlModel.cpp

JoyStick.o: JoyStick.cpp JoyStick.hpp
	$(CC) -c $(FLAGS) JoyStick.cpp

JMap_Xbox.o: JMap_Xbox.cpp JMap_Xbox.h
	$(CC) -c $(FLAGS) JMap_Xbox.cpp

style: *.cpp *.h *.hpp
	astyle -A4 -s2 *.cpp *.h *.hpp
	rm *.orig

install:
	install ROSV_Joystick.json /etc
	touch /var/log/ROSV_Joystick
	install ROSV_Joystick /usr/local/bin
	install ROSV_Joystick.sh /etc/init.d
	update-rc.d ROSV_Joystick.sh defaults 98 02
	cp ROSV_Logrotate /etc/logrotate.d

uninstall:
	rm /usr/local/bin/ROSV_Joystick
	rm /etc/ROSV_Joystick.json
	update-rc.d -f ROSV_Joystick.sh remove
	rm /etc/init.d/ROSV_Joystick.sh
	rm /etc/logrotate.d/ROSV_Logrotate

clean:
	rm -rf *o *~ ROSV_Joystick
