#!/usr/bin/env python

import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import json

topic = "tele/sonoff/RESULT/#"

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(topic)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    msg_json = json.loads(str(msg.payload))

    # Verify message to at least some extent
    if msg_json['RfRaw']['Data'][0:4] == "AAA4" and msg_json['RfRaw']['Data'][22:24] == "55":
        publish.single("tele/sonoff/door_sensor", msg_json['RfRaw']['Data'][16:22], hostname = "phobos")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("phobos", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()
