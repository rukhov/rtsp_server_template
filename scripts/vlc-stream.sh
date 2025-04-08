#!/bin/bash


#vlc -vvv /home/rukhov/Downloads/VID_20200309_131705.mp4 --sout '#transcode{vcodec=h264,acodec=mpga}:rtp{sdp=rtsp:/10.196.174.46:8554/live}' --sout-keep --loop

#ffmpeg -re -i /home/rukhov/Downloads/VID_20200309_131705.mp4 -c:v libx264 -f rtsp rtsp://10.196.174.46:8554/test
ffmpeg -re -stream_loop -1 -i /home/rukhov/Downloads/VID_20200309_131705.mp4 -c:v libx264 -preset ultrafast -tune zerolatency -c:a aac -f rtsp rtsp://localhost:8554/test

