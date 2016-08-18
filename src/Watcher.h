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

#include "utilities_js.hpp"


class StratumClient;
class ClientContainer;


///////////////////////////////// ClientContainer //////////////////////////////
class ClientContainer {
  atomic<bool> running_;

  string poolsFile_;

  vector<StratumClient *> clients_;

  // libevent2
  struct event_base *base_;
  struct event *signal_event_;

public:
  ClientContainer(const string &poolsFile);
  ~ClientContainer();

  size_t initPoolClients();
  void run();
  void stop();

  static void readCallback (struct bufferevent *, void *ptr);
  static void eventCallback(struct bufferevent *, short, void *ptr);
};


///////////////////////////////// StratumClient //////////////////////////////
class StratumClient {
  struct bufferevent *bev_;

  string  poolHost_;
  int16_t poolPort_;
  string  workerName_;

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

public:
  StratumClient(struct event_base *base, ClientContainer *container,
                const string &poolHost, const int16_t poolPort,
                const string &workerName);
  ~StratumClient();

  bool connect();

  void recvData();
  void sendData(const char *data, size_t len);
  inline void sendData(const string &str) {
    sendData(str.data(), str.size());
  }

  // means auth success and got at least stratum job
  bool isAvailable();
};

#endif
