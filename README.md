# AutoBloomer Sensor Controller

<img src="/images/sensor_pod_image_1.jpg" width="30%">

Full plans and code for a sensor controller capable of publishing multiple sensor data via MQTT

Raspberry Pico source code is [here](pico/src)

PCB schematics, plans etc (KiCad format) are [here](PCB/SensorPod-Board)

A brief overiew is on [YouTube](https://youtu.be/ilkXrpyaba0)

Under the hood the sensor pod uses a Raspberry Pico W to communicate with an MQTT broker and transmit data it collates from the following sensors:
- SCD30 environmental sensor
- Stemma soil sensor
- Battery voltage (for Raspberry Pi RTC)
- Sonar (feed level) sensors

Configuration of the pod is done in two ways:

#### Basic pod configuration
Basic pod configuration of esential runtime variables (wireless SSID/key, broker address, pod location/name etc) is done via serial port connected to the bord's EXT header (3.3v, 57600/8/n/1) and transmitting commands in the format: `<COMMAND><PARAMETER>` (note no space)
- SSID<< wireless ssid >> - Sets the WiFi SSID
- PASS<< password >> - Sets WiFi passkey
- LOCN<< location >> - Sets sensor location (this is used for the MQTT topic path)
- NAME<< sensor group name >> - Sets sensor name (also ussed for the MQTT topic path
- BRKR<< address >> - Sets the broker address

Once these have been set the pod will attempt to connect to the configured broker via the supplied wireless network and begin publishing sensor data to the topic:

```
AutoBloomer/<< sensor location >>/<< sensor name >>
```

#### Runtime sensor calibration
Once connected, the pod will also subscribe to a specific MQTT topic to listen for sensor calibration commands. The topic in question is:
```
AutoBloomer/<< sensor location >>/<< sensor name >>/control
```

The payload consists of a single line detailing the type of calibration to be performed, one of the following:
- TEMP [temperature offset]
- FRC [CO2 field calibration value]
