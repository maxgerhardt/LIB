# pip install paho-mqtt
# https://test.mosquitto.org/ssl/mosquitto.org.crt

import os, sys, time, ssl
from os.path import join
from datetime import datetime
import paho.mqtt.publish as publish
import paho.mqtt.client as mqtt

DIR  = os.path.dirname(os.path.realpath(__file__))
HOST = "test.mosquitto.org"
PORT = 8883

TLS = {
    'ca_certs' : join(DIR, "mosquitto.org.crt"), 
    'tls_version' : ssl.PROTOCOL_TLSv1_2
}

print ("---BEGIN---")
publish.single( 'wizio', 
    payload = "Hello world:" + str(datetime.now()),
    hostname = HOST, port = PORT, 
    protocol  = mqtt.MQTTv311,
    client_id = "CLIENT_ID_" + str( time.time() ),
    tls = TLS
)
print ("----END----")
