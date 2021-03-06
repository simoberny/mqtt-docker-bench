# mqtt-docker-bench 
[![Build Status](https://travis-ci.org/simoberny/mqtt-docker-bench.svg?branch=master)](https://travis-ci.org/simoberny/mqtt-docker-bench)

MQTT protocol bench test based on Docker simulated sensor and Eclipse Mosquitto Broker

Objective of the bench is to estimate the typical bandwitdh need of a MQTT protocol implementation. 
Th structure is based on a virtual net composed of sensor and sinks (as publishers), a broker based on Mosquitto and a series of client (as subscribers).

<img src="https://github.com/simoberny/mqtt-docker-bench/blob/master/_report/images/struttura.png" width="450">

### Clients
Based on minimal Alpine image that run a C program simulating the sensor behaviour. 
Also a python and Java implementation, but C was choosen for resource reason. 

- Python: Apline apk installation require almost a GigaByte on disc
- Java: 20 MByte container RAM usage due to virtual machine
- C implementation: 0,9 MByte (!!!) container RAM occupation. 100 clients takes only 100MByte

The C implementation use the MQTT-C library: https://github.com/LiamBindle/MQTT-C/

### Broker
Alpine with mosquitto running on a Virtualbox virtual machine due to IP network reason. With Wireshark is easier to capture and classify traffic.

### Sinks
Sinks is some simulated node that aggregate MQTT message in the same TCP payload. This makes the MQTT be less resources-angry due to dramatic decrease in protocol header overhead. 


## Instructions
Before everything, change the IP to which client connects in file ``clients/C/publisher/simple_publisher.c``
Also change other settings, like QoS level, etc...

Compile

```bash
cd build  
cmake ..  
make  
mv publish ../../  
```

The last command move the new executable in the folder of Dockerfile, which will copy it on the container  
The executable takes three parameter in:

```
./publish {boolean} {boolean} {integer} {integer}
```
>args 1: 0 for a message every second, 1 for realistic random behaviour  
>args 2: 0 if is a client, 1 for sink functionalities  
>agrs 3: number of client behind a sink    
>agrs 4: payload length to send random string 

To start the docker's client: 

```bash
docker-compose up -d
```


### MQTT Overhead

![MQTT Overhead](https://github.com/simoberny/mqtt-docker-bench/blob/master/_report/images/payload.png)

