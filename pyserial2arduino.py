from time import sleep
import serial
import urllib.request
import threading
import grequests
import sys, termios, tty, os, time

ser = serial.Serial('/dev/ttyUSB'+str(input('[INPUT] ttyUSB? ')), 57600) # Establish the connection on a specific port

def getch():
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    try:
        tty.setraw(sys.stdin.fileno())
        ch = sys.stdin.read(1)
 
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
    return ch
 
button_delay = 0.2

def sendData(val):
	'''
		val = '1111' stands for:
			lH = 1
			rH = 1
			lT = 1
			rT = 1
	'''
	ser.write(val.encode()) # Convert the decimal number to ASCII then send it to the Arduino: type bytes, b'Hello'
	# print(ser.readline()) # Read the newest output from the Arduino
	sleep(.01) # Delay for one tenth of a second
 
while True:
    char = getch()
 
    if (char == "p"):
        print("Stop!")
        exit(0)
 
    if (char == "a"):
        print("Left pressed")
        sendData('P0202')
        time.sleep(button_delay)
 
    elif (char == "d"):
        print("Right pressed")
        sendData('P2020')
        time.sleep(button_delay)
 
    elif (char == "w"):
        print("Up pressed")
        sendData('P2222')
        time.sleep(button_delay)
 
    elif (char == "s"):
        print("Down pressed")
        sendData('P0000')
        time.sleep(button_delay)

    elif (char == "q"):
        print("Stop pressed")
        sendData('P1111')
        time.sleep(button_delay)
