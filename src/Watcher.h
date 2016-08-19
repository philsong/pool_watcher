/*
 Mining Pool Watcher

 Copyright (C) 2016  BTC.COM

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef POOL_WATCHER_H_
#define POOL_WATCHER_H_

#include "Common.h"
#include "Utils.h"

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>

#include <bitset>
#include <map>
#include <set>

#include "MySQLConnection.h"
#include "utilities_js.hpp"


class StratumClient;
class ClientContainer;


///////////////////////////////// ClientContainer //////////////////////////////
class ClientContainer {
  atomic<bool> running_;
  vector<StratumClient *> clients_;

  // libevent2
  struct event_base *base_;
  struct event *signal_event_;

  MysqlConnectInfo dbInfo_;
  MySQLConnection db_;

public:
  ClientContainer(const MysqlConnectInfo &dbInfo);
  ~ClientContainer();

  bool init();
  void run();
  void stop();

  void removeAndCreateClient(StratumClient *client);

  bool insertBlockInfoToDB(const string &poolName,
                           uint64_t jobRecvTimeMs, int32_t blockHeight,
                           const string &blockPrevHash, uint32_t blockTime);

  static void readCallback (struct bufferevent *bev, void *ptr);
  static void eventCallback(struct bufferevent *bev, short events, void *ptr);
};


///////////////////////////////// StratumClient //////////////////////////////
class StratumClient {
  struct bufferevent *bev_;

  uint32_t extraNonce1_;
  uint32_t extraNonce2Size_;

  string lastPrevBlockHash_;

  bool handleMessage();
  void handleStratumMessage(const string &line);
  bool handleExMessage(struct evbuffer *inBuf);

public:
  enum State {
    INIT          = 0,
    CONNECTED     = 1,
    SUBSCRIBED    = 2,
    AUTHENTICATED = 3
  };
  State state_;
  ClientContainer *container_;

  string  poolName_;
  string  poolHost_;
  int16_t poolPort_;
  string  workerName_;

public:
  StratumClient(struct event_base *base, ClientContainer *container,
                const string &poolName,
                const string &poolHost, const int16_t poolPort,
                const string &workerName);
  ~StratumClient();

  bool connect();

  void recvData();
  void sendData(const char *data, size_t len);
  inline void sendData(const string &str) {
    sendData(str.data(), str.size());
  }
};

#endif
