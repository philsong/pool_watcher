#
# Dockerfile
#
# @author zhibiao.pan@bitmain.com
# @copyright btc.com
# @since 2016-08-01
#
#
FROM phusion/baseimage:0.9.19
MAINTAINER PanZhibiao <zhibiao.pan@bitmain.com>

ENV HOME /root
ENV TERM xterm
CMD ["/sbin/my_init"]

# use aliyun source if you are in China
#ADD sources-aliyun.com.list /etc/apt/sources.list

RUN apt-get update
RUN apt-get install -y build-essential bsdmainutils cmake git libevent-dev libmysqlclient-dev supervisor wget

# build
ADD init.sh /root/init.sh
RUN bash /root/init.sh

RUN mkdir -p /root/poolwatcher
ADD supervisord_poolwatcher.conf /etc/supervisor/conf.d/poolwatcher.conf

# remove source & build files
RUN rm -rf ~/source

# clean
RUN apt-get clean && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
