#!/bin/bash


#ffplay rtsp://10.196.174.46:8554/test


for i in `seq 2 100000`
do
    echo "$i"
    sleep 3
#    ffplay -v verbose -rtsp_flags prefer_tcp "rtsp://10.196.174.46:8554/test"
#    ffplay -v 9 -loglevel 99 rtsp://10.196.174.46:8554/test
#    ffplay -v verbose rtsp://10.196.174.12:554
    vlc -v rtsp://10.196.174.46:8554/test
done


