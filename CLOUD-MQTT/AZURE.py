#https://ssl-tools.net/certificates/d4de20d05e66fc53fe1a50882c78db2852cae474.pem

import paho.mqtt.publish as publish
import paho.mqtt.client as mqtt
import ssl

auth = {
  'username':"WizIO.azure-devices.net/device_007",
  'password':"SharedAccessSignature sr=WizIO.azure-devices.net&sig=v30n5zoXYSgTkd5lgATnT1AXYk%2BA7z9og8Tnk7H2Zqc%3D&se=1591890374"
}

tls = {
  'ca_certs':"D:\\PYTHON\\cert.pem",
  'tls_version':ssl.PROTOCOL_TLSv1
}

print "BEGIN"
publish.single("devices/WizIO/messages/events/",
  payload="hello world",
  hostname="WizIO.azure-devices.net",
  client_id="device_007",
  auth=auth,
  tls=tls,
  port=8883,
  protocol=mqtt.MQTTv311)
print "END"