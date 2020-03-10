import serial
import sys
ser = serial.Serial("/dev/ttyUSB0",9600)
while 1:
	sys.stdout.write( ser.readline())