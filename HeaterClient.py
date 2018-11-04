#!/usr/bin/env python

import socket
from threading import Timer, Thread, Event

heartbeat_interval_s = 20.0
max_active_s = 60

relay_state = '0';

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

# create an ipv4 (AF_INET) socket object using the tcp protocol (SOCK_STREAM)
client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# connect the client
client.connect(('home.zachrisson.info', 6666))

stopFlag = Event()
t = heartbeat_timer(stopFlag)
t.start()

stopFlag2 = Event()
t2 = deactivate_relay_timer(stopFlag2)

client.send(relay_state)

while True:
    response = client.recv(1)

    if response[0] == '0':
        relay_state = '0'
        print 'Turning off relay'
    elif response[0] == '1':
        print 'Turning on relay'
        relay_state = '1'
        t2.start()
