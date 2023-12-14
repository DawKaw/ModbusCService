# ModbusCService
### ModbusCService allows to data exchange between device via Modbus protocol and Mqtt service.

The idea is simple. Query data (individual registers) to modbus slave devices and publish it as a topic on the MQTT server. And vice versa, listen for established topics on the MQTT server and when a message arrives, send the data to the Modbus slave device.
After receiving data from the modbus device, the program checks whether the data has changed. If they have changed, then he publishes them in MQTT.

### How to use the program.
The program can be used in two modes. As a regular **console program** and as **a service**. Before you run it, configure the correct MQTT service (mosquitto) on the server and grant permissions so that you can subscribe and publish topics.
```
Usage: modbusCService -c config file name.
    [-c config] Config file name.
    [-v level] Verbose level:
       0 - silence
       1 - only faults
       2 - faults and a few notice (default)
       3 - more notice
       4 and more full verbose.
    [-V version] Print version.
    [-s user] Run as service as user.
    [-p pifFile] Save PID to file name.
 ````
Example as console program:
```
  modbusCService -c modbuscs.conf
````
To run the program as a service (daemon), follow the steps below.
```
- check your file configuration in /etc/modbuscs.conf and test as console program.
- create a user "modbuscs" with home path in "/var/lib/" without shell
- copy into his home dir scripts "modbus_scr.sh" and "mqtt_scr.sh"
- copy modbusCService.service_example to systemd directory "/lib/systemd/system/" as modbusCService.service
- register new service in systemd and start
- check logs in /var/log/syslog
````

### To compile the program.
To compile the program, you need to have the: cmake, make package,  installed and library: libmodbus, libmosquitto. On Ubuntu type:
```
# sudo apt-get update
# sudo apt-get install build-essential
# sudo apt-get install cmake
# sudo apt-get install libmodbus-dev
# sudo apt-get install libmosquitto-dev
````
Compilation:
```
# cd ModbusCService
# cmake .
# make
# sudo make install
````

### www
In the www folder there are sample files of a website that exchanges data with the MQTT server.

### Scripts
Files modbus_scr.sh and mqtt_scr.sh contain sample scripts that run when data arrives.
