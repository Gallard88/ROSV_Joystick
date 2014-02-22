all: ROSV_Joystick

FLAGS=-Wall -O2
CC=g++

ROSV_Joystick: JoyStick.o parson.o main.o TCP_Client.o
	$(CC) $(FLAGS) JoyStick.o parson.o TCP_Client.o main.o -o ROSV_Joystick

main.o: main.cpp
	$(CC) -c $(FLAGS) main.cpp

JoyStick.o: JoyStick.cpp JoyStick.hpp
	$(CC) -c $(FLAGS) JoyStick.cpp

parson.o: parson.c parson.h
	$(CC) -c $(FLAGS) parson.c

TCP_Client.o: TCP_Client.cpp TCP_Client.hpp
	$(CC) -c $(FLAGS) TCP_Client.cpp

style: *.cpp *.c *.h *.hpp
	astyle -A4 -s2 *.c *.cpp *.h *.hpp
	rm *.orig

#install:
#	install TFTP_Server /usr/local/bin
#	install TFTP_Server.sh /etc/init.d
#	update-rc.d TFTP_Server.sh defaults 98 02
	
#uninstall: TFTP_Server
#	update-rc.d -f TFTP_Server.sh remove
#	rm /usr/local/bin/TFTP_Server 
#	rm /etc/init.d/TFTP_Server.sh

clean:
	rm -rf *o *~ ROSV_Joystick
