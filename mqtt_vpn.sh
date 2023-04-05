#!/bin/bash

while true; do
    if ping -c 1 vpn.MQTT_SERVER.com &> /dev/null; then
        echo "VPN is up"
        if ! pgrep mqtt_vpn &> /dev/null; then
            echo "mqtt_vpn is not running, starting it"
            mqtt_vpn -i mqtt0 -a 192.168.XX.YY -b tcp://vpn.YOURMQTT.com:1883 -u MQTT_USER -p MQTT_PASSWORD &
        fi
    else
        echo "VPN is down"
        if pgrep mqtt_vpn &> /dev/null; then
            echo "mqtt_vpn is running, stopping it"
            pkill mqtt_vpn
        fi
    fi
    sleep 10
done
