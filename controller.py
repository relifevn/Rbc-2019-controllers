import urllib.request
import threading
import grequests

import sys, termios, tty, os, time

ip = "192.168.10.1"

'''
def sendData(lH, rH, lT, rT, sC):
    threading.Thread(target=threadSendingData, args=(lH,rH,lT,rT,sC)).start()

def threadSendingData(lH,rH,lT,rT,sC):
    try:
        url = "http://" +ip+"/?lH="+str(lH)+"&rH="+str(rH)+"&lT="+str(lT)+"&rT="+str(rT)+"&sC="+str(sC)
        print('[DEBUG] url = '+url)
        contents = urllib.request.urlopen(url).read()
        print('[INFO] contents = ')
        print(contents)
    except:
        print("[WARNING] Server is not responding !") 
'''

def exception_handler(req, e):
    print("[WARNING] Server is not responding !")

def sendData(lH, rH, lT, rT, sC):
    url = "http://" +ip+"/?lH="+str(lH)+"&rH="+str(rH)+"&lT="+str(lT)+"&rT="+str(rT)+"&sC="+str(sC)
    reqs = [grequests.get(url, timeout=10)]
    grequests.map(reqs, exception_handler=exception_handler)
    


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
 
while True:
    char = getch()
 
    if (char == "p"):
        print("Stop!")
        exit(0)
 
    if (char == "a"):
        print("Left pressed")
        sendData(0,2,0,2,0)
        time.sleep(button_delay)
 
    elif (char == "d"):
        print("Right pressed")
        sendData(2,0,2,0,0)
        time.sleep(button_delay)
 
    elif (char == "w"):
        print("Up pressed")
        sendData(2,2,2,2,0)
        time.sleep(button_delay)
 
    elif (char == "s"):
        print("Down pressed")
        sendData(0,0,0,0,0)
        time.sleep(button_delay)

    elif (char == "q"):
        print("Down pressed")
        sendData(1,1,1,1,0)
        time.sleep(button_delay)

    elif (char == "1"):
        print("Number 1 pressed")
        time.sleep(button_delay)

    elif (char == "5"):
        print("Number 5 pressed")
        time.sleep(button_delay)




