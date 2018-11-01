#!/usr/bin/env python

import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import json

import socket
import sys
from thread import *

HOST = ''
PORT = 6666

topic = "GSMHeater/ctrl"
available_topic = "GSMHeater/available"

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
try:
    s.bind((HOST, PORT))
except:
    print 'Bind failed. Error Code : ' + str(msg[0]) + ' Message ' + msg[1]
    sys.exit()
print ('Bound to socket')

s.listen(1)

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(topic)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print ("Got: " + msg.payload)
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

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_start()

def clientthread(conn):
    #Sending message to connected client
    conn.send('Welcome to the server. Type something and hit enter\n') #send only takes string

    #infinite loop so that function do not terminate and thread do not end.
    while True:
        #Receiving from client
        data = conn.recv(1024)
        reply = 'OK...' + data
        if not data:
            break

        # conn.sendall(reply)
    #came out of loop
    conn.close()

#now keep talking with the client
while 1:
    global conn
    # wait to accept a connection - blocking call
    conn, addr = s.accept()
    print 'Connected with ' + addr[0] + ':' + str(addr[1])

    #start new thread takes 1st argument as a function name to be run, second is the tuple of arguments to the function.
    start_new_thread(clientthread ,(conn,))

s.close()
