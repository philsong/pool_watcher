Docker for PoolWatcher
=======================

* OS: `Ubuntu 14.04 LTS`
* Docker Image OS: `Ubuntu 16.04 LTS`

## Install Docker

```
# Use 'curl -sSL https://get.daocloud.io/docker | sh' instead of this line
# when your server is in China.
wget -qO- https://get.docker.com/ | sh

service docker start
service docker status
```

## Build Docker Images

```
cd /work

git clone https://github.com/btccom/pool_watcher.git
cd docker

# If your server is in China, please check "Dockerfile" and uncomment some lines

# build
docker build -t poolwatcher:0.1 .

# mkdir for poolwatcher
mkdir -p /work/run_poolwatcher

# mysql conf json
cp /work/pool_watcher/mysql.json /work/run_poolwatcher
# edit mysql.json
vim /work/run_poolwatcher/mysql.json
```

## Start Docker Container

```
# start docker
docker run -it -v /work/run_poolwatcher:/root/poolwatcher --name poolwatcher --restart always -d poolwatcher:0.1

# login
docker exec -it poolwatcher /bin/bash
```
