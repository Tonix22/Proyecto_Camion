import serial
ser = serial.Serial("/dev/ttyUSB0",76800)
while 1:
	print ser.readline()