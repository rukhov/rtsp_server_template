#!/bin/bash


#ffplay rtsp://10.196.174.46:8554/test


for i in `seq 2 100000`
do
    echo "Iteration: $i"
    ffplay -timeout 5000000 "rtsp://10.196.174.46:8554/test"
#    ffplay -timeout 10000000 -v 9 -loglevel 99 rtsp://10.196.174.46:8554/test
#    ffplay -v verbose rtsp://10.196.174.12:554
#    vlc -v --open rtsp://10.196.174.46:8554/test
    echo "Paused..."
    sleep 3
done


