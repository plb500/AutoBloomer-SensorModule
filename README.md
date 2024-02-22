# AutoBloomer Sensor Module

<img src="/images/sensor_pod_image_1.jpg" width="30%">

Full plans and code for a sensor controller capable of publishing multiple sensor data via MQTT

Raspberry Pico source code is [here](pico/src)

PCB schematics (KiCad format), plans plus any 3D prints/CNC files for the different hardware platforms are [here](hardware)

A brief overiew is on [YouTube](https://youtu.be/ilkXrpyaba0)

Under the hood the sensor module uses a Raspberry Pico W to communicate with an MQTT broker and transmit data it collates from the following sensors:
- SCD30 environmental sensor
- Stemma soil sensor
- Battery voltage (for Raspberry Pi RTC)
- Sonar (feed level) sensors

I also intend to add EC and/or pH sensors to this to essentially make this a fully-fledged hydroponics sensor interface.

The module arranges these sensors into sensor "groups". Each group effectively publishes a set of data from multiple sensors to a single MQTT topic. This allows the Sensor Pod, for example, to accumulate sensor data from both the SCD30 sensor and the Stemma Soil Sensor into a single topic, and also allows the HIB to publish sensor data to multiple topics (i.e. the RTC battery voltage and each individual connected feed sensor all publish to their own individual topics)

## Hardware platforms
Currently there are two different hardware platforms defined:
- Sensor Pod
- Hardware Interface Board (HIB)

### Sensor Pod
The Sensor Pod consists of a single sensor group providing data from the following sensors:
- Group 0
  - SCD30
    - CO2
    - Humidity
    - Temperature
  - Stemma Soil Sensor
    - Soil moisture
 
### Hardware Interface Board
The HIB consists of two distinct pieces of hardware. One section provides power and hardware control to the Raspberry Pi powering the AutoBloomer controller, along with a RTC module and interfacing to relays. The other section is the sensor module which provides data from feed level sensors along with the current RTC battery module. These are grouped as follows:
- Group 0
  - RTC battery level
- Group 1
  - Feed sensor
- Group 2
  - Feed sensor
 
## Module configuration
Configuration of each module is done in two ways:

### Basic module configuration
Basic pod configuration of essential runtime variables (wireless SSID/key, broker address, module location/name etc) is done via serial port connected to the bord's EXT header (3.3v, 57600/8/n/1) and transmitting commands in the format: `<COMMAND><SENSOR_GROUP><PARAMETER>` (note no space)
- SSID<< wireless ssid >> - Sets the WiFi SSID
- PASS<< password >> - Sets WiFi passkey
- NAME<< module >> - Sets sensor module name
- BRKR<< address >> - Sets the broker address
- GRPN<< sensor group index>><< sensor group name >> - Sets the name of the sensor group at the supplied index (this is used for the MQTT topic path)
- GRPL<<sensor group index>><< sensor group location >> - Sets the location of the sensor group at the supplied index (this is used for the MQTT topic path)

Once these have been set the pod will attempt to connect to the configured broker via the supplied wireless network and begin publishing sensor data to the topic:

```
AutoBloomer/<< sensor group location >>/<< sensor group name >>
```

### Runtime sensor calibration
Once connected, the pod will also subscribe to a specific MQTT topic to listen for sensor calibration commands. The topic in question is:
```
AutoBloomer/<< sensor group location >>/<< sensor group name >>/control
```

The payload consists of a single line detailing the type of calibration to be performed. Currently the only sensor capable of (and requiring) calibration is the SCD30 on the Sensor Pod.
Calibration commands are one of the following:
- TEMP [temperature offset]
- FRC [CO2 field calibration value]
