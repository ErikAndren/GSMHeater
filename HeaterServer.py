#!/usr/bin/env python3

import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import json

import socket
import sys

import argparse
import _thread

from threading import Timer

topic = "GSMHeater/ctrl"
available_topic = "GSMHeater/available"
state_topic = "GSMHeater/state"

parser = argparse.ArgumentParser(description = 'Control server')
parser.add_argument('--heartbeat_interval', '-i', dest = 'heartbeat_interval', default = 65.0, help = 'Expected heartbeat interval')
parser.add_argument('--mqtt_server_name', '-ms', dest = 'mqtt_server_name', default = 'mercury', help = 'MQTT server to connect to')
parser.add_argument('--mqtt_server_port', '-mp', dest = 'mqtt_server_port', default = 1883, help = 'MQTT server port to connect to')
parser.add_argument('--server_port', '-sp', dest = 'server_port', default = 6666, help = 'Server port')
parser.add_argument('--server_host', '-sh', dest = 'server_host', default = '', help = 'Server host')

args = parser.parse_args()

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
try:
    s.bind((args.server_host, args.server_port))
except Exception as e:
    print('Bind failed', e)
    sys.exit()

s.listen(1)

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(topic)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print('Got mqtt message: ', msg.payload)
    
    payload = msg.payload.decode('utf-8')
    if payload == "ON":
        print('Received MQTT message to turn on heater')
        conn.sendall(b'1');
    if payload == "OFF":
        print('Received MQTT message to turn off heater')
        conn.sendall(b'0');

# Connect to mqtt server
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(args.mqtt_server_name, args.mqtt_server_port, 60)
client.loop_start()

def conn_timeout(): 
    print ('No heartbeat received within window of ', args.heartbeat_interval, ', setting heater availability to offline')
    client.publish(available_topic, 'offline')

def client_thread(conn):
    #Sending message to connected client
    #conn.send(b'Welcome\n') #send only takes string

    t = Timer(int(args.heartbeat_interval), conn_timeout)
    # infinite loop so that function do not terminate and thread do not end.
    while True:
        # Receiving from client
        try: 
            data = conn.recv(1024)
        except Exception as e:
            print('Exception while waiting for data: ', e)
            return

        if not data:
            print('Did not receive any data. Terminating thread')
            return

        data_str = data.decode('utf-8')
        if data_str[0] == '1' or data_str[0] == '0':
            # FIXME: Later we can use this to inform our switch trigger what the current state is
            print ('Starting timeout timer')
            t.cancel();
            t = Timer(int(args.heartbeat_interval), conn_timeout)
            client.publish(state_topic, data_str)
            client.publish(available_topic, 'online')
            t.start()
        else:
            print(data)

#now keep talking with the client
while True:
    global conn
    # Wait to accept a connection - blocking call

    print('Waiting for connection')
    conn, addr = s.accept()
    print('Connected with ', addr[0], ':', addr[1])

    # Start new thread takes 1st argument as a function name to be run, second is the tuple of arguments to the function.
    try:
        _thread.start_new_thread(client_thread, (conn,))
    except Exception as e:
        print('Failed to start new thread: ', e)

s.close()
