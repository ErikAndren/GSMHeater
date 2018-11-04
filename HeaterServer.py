#!/usr/bin/env python

import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import json

import socket
import sys
from thread import *

from threading import Timer

HOST = ''
PORT = 6666

topic = "GSMHeater/ctrl"
available_topic = "GSMHeater/available"
heartbeat_interval = 65.0

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
try:
    s.bind((HOST, PORT))
except:
    print 'Bind failed'
    sys.exit()

s.listen(1)

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(topic)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    # FIXME: Just send 1 or 0 as MQTT payload to simplify this
    if msg.payload == "ON":
        print("Send message to heater to turn on")
        conn.sendall("1");
    if msg.payload == "OFF":
        print("Send message to heater to turn off")
        conn.sendall("0");

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("phobos", 1883, 60)

client.loop_start()

def conn_timeout():
    print ('No heartbeat received, setting heater availability to offline')
    client.publish(available_topic, 'offline')

def clientthread(conn):
    #Sending message to connected client
    conn.send('Welcome\n') #send only takes string

    t = Timer(heartbeat_interval, conn_timeout)
    #infinite loop so that function do not terminate and thread do not end.
    while True:
        #Receiving from client
        data = conn.recv(1024)

        if not data:
            return

        if data[0] == "1" or data[0] == "0":
            # FIXME: Later we can use this to inform our switch trigger what the current state is
            print ('Starting timeout timer')
            t.cancel();
            t = Timer(heartbeat_interval, conn_timeout)
            client.publish(available_topic, 'online')
            t.start()
        else:
            print ('Got: ' + data)

#now keep talking with the client
while True:
    global conn
    # wait to accept a connection - blocking call
    conn, addr = s.accept()
    print 'Connected with ' + addr[0] + ':' + str(addr[1])

    #start new thread takes 1st argument as a function name to be run, second is the tuple of arguments to the function.
    start_new_thread(clientthread ,(conn,))

s.close()
