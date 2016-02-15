#!/usr/bin/python
import serial
import syslog
import time

ser = serial.Serial('/dev/ttyACM0')

time.sleep(5)
ser.flush()
print(ser.readline())
ser.flush()
ser.write('T'+str(int(time.time())-10800))
while True:
	s = ser.readline()
	print s
