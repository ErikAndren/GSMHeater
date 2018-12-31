#!/usr/bin/env python2

import socket
import signal
import sys
from threading import Timer, Thread, Event

heartbeat_interval_s = 20.0
max_active_s = 60

relay_state = '0';

stopFlag = Event()
stopFlag2 = Event()

# create an ipv4 (AF_INET) socket object using the tcp protocol (SOCK_STREAM)
client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

def signal_handler(signum, frame):
    stopFlag.set()
    stopFlag2.set()
    client.close()
    sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

class heartbeat_timer(Thread):
    def __init__(self, event):
        Thread.__init__(self)
        self.stopped = event

    def run(self):
        while not self.stopped.wait(heartbeat_interval_s):
            global client

            client.send(relay_state)
            print 'Sending heart beat'

class deactivate_relay_timer(Thread):
    def __init__(self, event):
        Thread.__init__(self)
        self.stopped = event

    def run(self):
        while not self.stopped.wait(max_active_s):
            global client
            global stopFlag2
            print 'Deactivating relay due to max timer'
            relay_state = '0'
            stopFlag2.set()

# connect the client
client.connect(('home.zachrisson.info', 6666))

t = heartbeat_timer(stopFlag)
t.start()

t2 = deactivate_relay_timer(stopFlag2)

client.send(relay_state)

while True:
    response = client.recv(1024)
    if not response:
        print 'Got response without data'
        break

    if response[0] == '0':
        relay_state = '0'
        print 'Turning off relay'
    elif response[0] == '1':
        print 'Turning on relay'
        relay_state = '1'
        t2.start()
    else:
        print response

client.close()
