#!/bin/bash
mkdir -p img
c=0
for line in $(cat $1) 
do
    ./webkit2png.py "$line" -w 1 --feature=javascript -o "img/$(echo $line | sed -e "s/[^/]*\/\/\([^@]*@\)\?\([^:/]*\).*/\2/").$c.png"
    let "c+=1"

done
