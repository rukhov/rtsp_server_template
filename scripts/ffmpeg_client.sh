#!/bin/bash


#ffplay rtsp://10.196.174.46:8554/test

ffplay -v verbose -rtsp_flags prefer_tcp "rtsp://10.196.174.46:8554/test"


