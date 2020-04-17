#!/bin/bash

while :
do
    # Create mqtt string
    temp=$[ ( $RANDOM % 35 )  + 1 ]
    hum=$[ ( $RANDOM % 80 )  + 20 ]
    string="${temp} C° - Humidity: ${hum} %"

    # Payload is between 20 and 22 byte
	
	pub -h 192.168.178.47 -t "test" -m "$string"

    echo "${HOSTNAME} publish $string"
	
done
