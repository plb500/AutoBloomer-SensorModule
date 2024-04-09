# AutoBloomer Sensor Modules

<p align="center">
  <img src="/images/sensor_pod_render.jpg" width="40%" /> 
  <img src="/images/hib-render.jpg" width="40%" />
</p>

<p align="center">
  <img src="/images/sensor_pod_with_board.jpg" width="40%" /> 
  <img src="/images/hib.jpg" width="40%" />
</p>


Full plans and code for a sensor controller capable of publishing multiple sensor data via MQTT

Raspberry Pico source code is [here](pico/src)

PCB schematics (KiCad format), plans plus any 3D prints/CNC files for the different hardware platforms are [here](hardware)

A brief overview is on [YouTube](https://youtu.be/QP_2SgASPRE)

Under the hood the sensor modules use Raspberry Pico Ws to communicate with an MQTT broker and transmit data it collates from the following sensors:
- SCD30 environmental sensor ([link](https://www.sensirion.com/products/catalog/SCD30/))
- Stemma soil sensor ([link](https://www.adafruit.com/product/4026))
- Battery voltage (for Raspberry Pi RTC) (uses an ADC pin on the Pico and custom circuitry on the HIB)
- Sonar (feed level) sensors (Currently using A02YYUW sonars - [link](https://www.dfrobot.com/product-1935.html))

I also intend to add EC and/or pH sensors to this to essentially make this a one-stop hydroponics sensor interface.

The module arranges these sensors into sensor "groups", referred to by their index. Each group effectively publishes a set of data from multiple sensors to a single MQTT topic. This allows the Sensor Pod, for example, to accumulate sensor data from both the SCD30 sensor and the Stemma Soil Sensor into a single topic, and also allows the HIB to publish sensor data to multiple topics (i.e. the RTC battery voltage and each individual connected feed sensor all publish to their own individual topics)

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
The HIB consists of two distinct pieces of hardware. One section provides power and hardware control to the Raspberry Pi powering the [AutoBloomer controller](https://github.com/plb500/AutoBloomer-Controller), along with a RTC module and interfacing to relays. The other section is the sensor module which provides data from feed level sensors along with the current RTC battery module voltage. These are grouped as follows:
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

#### Example commands
- `SSIDMyNetwork` -> Sets the sensor module to connect to the wireless network with the SSID "MyNetwork"
- `PASSthe_key` -> Sets the sensor module to connect to the wireless network using the PSK "the_key"
- `NAMESensorModule` -> Sets the Pico's host/MQTT client name to be "SensorModule"
- `BRKRthebroker.local` -> Tells the sensor module to connect and publish to the MQTT broker with host name "thebroker.local"
- `BRKR192.168.1.50` -> Tells the sensor module to connect and publish to the MQTT broker at address "192.168.1.50"
- `GRPN0Group1Sensor` -> Sets the name of the sensor group at index 0 to "Group1Sensor"
- `GRPL0LeftChamber` -> Sets the location of the sensor group at index 0 to "LeftChamber"

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
- TEMP (temperature offset)
- FRC (CO2 field calibration value)

For example, setting a FRC value of 400 (i.e. recalibrating outside) and using mosquitto_pub the command line would look something like this:

```
mosquitto_pub -h broker.address -m "FRC 400" -t "AutoBloomer/SensorLocation/SensorName/control"
```

For more details on these parameters, see the SCD30 documentation [here](/docs/SCD30_Interface_Description.pdf)
