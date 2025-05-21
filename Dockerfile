FROM ubuntu:24.10

WORKDIR /home

COPY . .

RUN <<EOF
apt-get --fix-missing update
apt-get --yes install autoconf automake autoconf-archive pkg-config 
apt-get --yes install nasm 
apt-get --yes install libgstreamer* libgstrtspserver*
apt-get --yes install gcc
EOF
