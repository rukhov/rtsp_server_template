#!/bin/bash

for i in `seq 2 100000`
do
    echo "Iteration: $i"
    ffplay -timeout 2000000 rtsp://127.0.0.1:8554/custom_video_src
    echo "Paused..."
    sleep 2
done


