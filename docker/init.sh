#!/bin/bash
#
# init install script for PoolWatcher
#
# OS: Ubuntu 14.04 LTS
#
# @copyright btc.com
# @author Kevin Pan
# @since 2016-08
#

CPUS=`lscpu | grep '^CPU(s):' | awk '{print $2}'`

# glog-v0.3.4
mkdir -p /root/source && cd /root/source
wget https://github.com/google/glog/archive/v0.3.4.tar.gz
tar zxvf v0.3.4.tar.gz
cd glog-0.3.4
./configure && make -j $CPUS && make install

# PoolWatcher
mkdir -p /root/source && cd /root/source
git clone https://github.com/btccom/pool_watcher.git
cd pool_watcher && mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j $CPUS
cp poolwatcher /usr/bin

