import serial
ser = serial.Serial('COM9', 1200, serial.EIGHTBITS, serial.PARITY_EVEN, serial.STOPBITS_ONE)
ser.write(open("incertidumbre.txt","rb").read())