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
#include "Watcher.h"

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>

static
bool tryReadLine(string &line, struct bufferevent *bufev) {
  line.clear();
  struct evbuffer *inBuf = bufferevent_get_input(bufev);

  // find eol
  struct evbuffer_ptr loc;
  loc = evbuffer_search_eol(inBuf, NULL, NULL, EVBUFFER_EOL_LF);
  if (loc.pos == -1) {
    return false;  // not found
  }

  // copies and removes the first datlen bytes from the front of buf
  // into the memory at data
  line.resize(loc.pos + 1);  // containing "\n"
  evbuffer_remove(inBuf, (void *)line.data(), line.size());

  return true;
}

static
bool resolve(const string &host, struct	in_addr *sin_addr) {
  struct evutil_addrinfo *ai = NULL;
  struct evutil_addrinfo hints_in;
  memset(&hints_in, 0, sizeof(evutil_addrinfo));
  // AF_INET, v4; AF_INT6, v6; AF_UNSPEC, both v4 & v6
  hints_in.ai_family   = AF_UNSPEC;
  hints_in.ai_socktype = SOCK_STREAM;
  hints_in.ai_protocol = IPPROTO_TCP;
  hints_in.ai_flags    = EVUTIL_AI_ADDRCONFIG;

  int err = evutil_getaddrinfo(host.c_str(), NULL, &hints_in, &ai);
  if (err != 0) {
    LOG(ERROR) << "evutil_getaddrinfo err: " << err << ", " << evutil_gai_strerror(err);
    return false;
  }
  if (ai == NULL) {
    LOG(ERROR) << "evutil_getaddrinfo res is null";
    return false;
  }

  // only get the first record, ignore ai = ai->ai_next
  if (ai->ai_family == AF_INET) {
    struct sockaddr_in *sin = (struct sockaddr_in*)ai->ai_addr;
    *sin_addr = sin->sin_addr;
  } else if (ai->ai_family == AF_INET6) {
    // not support yet
    LOG(ERROR) << "not support ipv6 yet";
    return false;
  }
  evutil_freeaddrinfo(ai);
  return true;
}

static
int32_t getBlockHeightFromCoinbase(const string &coinbase1) {
  // https://github.com/bitcoin/bips/blob/master/bip-0034.mediawiki
  const string a = coinbase1.substr(86, 2);
  const string b = coinbase1.substr(88, 2);
  const string c = coinbase1.substr(90, 2);
  const string heightHex = c + b + a;  // little-endian

  return atoi(heightHex.c_str());
}


//
// input  : 89c2f63dfb970e5638aa66ae3b7404a8a9914ad80328e9fe0000000000000000
// output : 00000000000000000328e9fea9914ad83b7404a838aa66aefb970e5689c2f63d
static
string convertPrevHash(const string &prevHash) {
  assert(prevHash.length() == 64);
  string hash;
  for (int i = 7; i >= 0; i--) {
    uint32_t v = (uint32_t)strtoul(prevHash.substr(i*8, 8).c_str(), nullptr, 16);
    hash.append(Strings::Format("%08x", v));
  }
  return hash;
}



///////////////////////////////// ClientContainer //////////////////////////////
ClientContainer::ClientContainer(const string &poolsFile,
                                 const MysqlConnectInfo &dbInfo)
:running_(true) , poolsFile_(poolsFile), dbInfo_(dbInfo), db_(dbInfo_)
{
}

ClientContainer::~ClientContainer() {

}

void ClientContainer::run() {

}

void ClientContainer::stop() {

}

size_t ClientContainer::initPoolClients() {
  // read pools text file
  return 0;
}


// static func
void ClientContainer::readCallback (struct bufferevent *bev, void *ptr) {

}

// static func
void ClientContainer::eventCallback(struct bufferevent *bev,
                                    short events, void *ptr) {
  StratumClient *client = static_cast<StratumClient *>(ptr);
//  ClientContainer *container = client->container_;

  if (events & BEV_EVENT_CONNECTED) {
    client->state_ = StratumClient::State::CONNECTED;

    // do subscribe
    string s = Strings::Format("{\"id\":1,\"method\":\"mining.subscribe\""
                               ",\"params\":[\"%s\"]}\n", BTCCOM_WATCHER_AGENT);
    client->sendData(s);
    return;
  }

  if (events & BEV_EVENT_EOF) {
    LOG(INFO) << "upsession closed";
  }
  else if (events & BEV_EVENT_ERROR) {
    LOG(ERROR) << "got an error on the upsession: "
    << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR());
  }
  else if (events & BEV_EVENT_TIMEOUT) {
    LOG(INFO) << "upsession read/write timeout, events: " << events;
  }
  else {
    LOG(ERROR) << "unhandled upsession events: " << events;
  }
}


///////////////////////////////// StratumClient //////////////////////////////
StratumClient::StratumClient(struct event_base *base, ClientContainer *container,
                             const string &poolName,
                             const string &poolHost, const int16_t poolPort,
                             const string &workerName)
: container_(container), poolName_(poolName),
poolHost_(poolHost), poolPort_(poolPort), workerName_(workerName)
{
  bev_ = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);

  bufferevent_setcb(bev_,
                    ClientContainer::readCallback,  NULL,
                    ClientContainer::eventCallback, this);
  bufferevent_enable(bev_, EV_READ|EV_WRITE);

  clientInfo_ = Strings::Format("[%s:%d,%s]", poolHost_.c_str(),
                                poolPort_, workerName_.c_str());
}

StratumClient::~StratumClient() {
  bufferevent_free(bev_);
}

bool StratumClient::connect() {
  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port   = htons(poolPort_);
  if (!resolve(poolHost_, &sin.sin_addr)) {
    return false;
  }

  // bufferevent_socket_connect(): This function returns 0 if the connect
  // was successfully launched, and -1 if an error occurred.
  int res = bufferevent_socket_connect(bev_, (struct sockaddr *)&sin, sizeof(sin));
  if (res == 0) {
    state_ = CONNECTED;
    return true;
  }

  return false;
}

bool StratumClient::handleMessage() {
  string line;
  if (tryReadLine(line, bev_)) {
    handleStratumMessage(line);
    return true;
  }

  return false;
}


void StratumClient::handleStratumMessage(const string &line) {
  DLOG(INFO) << clientInfo_ << " UpStratumClient recv(" << line.size() << "): " << line;

  JsonNode jnode;
  if (!JsonNode::parse(line.data(), line.data() + line.size(), jnode)) {
    LOG(ERROR) << "decode line fail, not a json string";
    return;
  }
  JsonNode jresult = jnode["result"];
  JsonNode jerror  = jnode["error"];
  JsonNode jmethod = jnode["method"];

  if (jmethod.type() == Utilities::JS::type::Str) {
    JsonNode jparams  = jnode["params"];
    auto jparamsArr = jparams.array();

    if (jmethod.str() == "mining.notify") {
      const string prevHash = convertPrevHash(jparamsArr[1].str());
      
      // stratum job prev block hash changed
      if (lastPrevBlockHash_ != prevHash) {
        const int32_t  blockHeight = getBlockHeightFromCoinbase(jparamsArr[2].str());
        const uint32_t blockTime   = jparamsArr[7].uint32_hex();
        container_->insertBlockInfoToDB(poolName_, poolHost_, poolPort_,
                                        (uint32_t)time(nullptr), blockHeight,
                                        prevHash, blockTime);
        lastPrevBlockHash_ = prevHash;
        LOG(INFO) << clientInfo_ << " prev block changed, height: " << blockHeight
        << ", prev_hash: " << prevHash;
      }
    }
    else {
      // ignore other messages
    }
    return;
  }

  if (state_ == AUTHENTICATED) {
    //
    // {"error": null, "id": 2, "result": true}
    //
    if (jerror.type()  != Utilities::JS::type::Null ||
        jresult.type() != Utilities::JS::type::Bool ||
        jresult.boolean() != true) {
      LOG(ERROR) << clientInfo_ <<  " auth fail";
    }
    return;
  }

  if (state_ == CONNECTED) {
    //
    // {"id":1,"result":[[["mining.set_difficulty","01000002"],
    //                    ["mining.notify","01000002"]],"01000002",8],"error":null}
    //
    if (jerror.type() != Utilities::JS::type::Null) {
      LOG(ERROR) << clientInfo_ << " json result is null, err: " << jerror.str();
      return;
    }
    std::vector<JsonNode> resArr = jresult.array();
    if (resArr.size() < 3) {
      LOG(ERROR) << clientInfo_ << " result element's number is less than 3: " << line;
      return;
    }

    extraNonce1_     = resArr[1].uint32_hex();
    extraNonce2Size_ = resArr[2].uint32();
    LOG(INFO) << clientInfo_ << " extraNonce1: " << extraNonce1_
    << ", extraNonce2 Size: " << extraNonce2Size_;

    // subscribe successful
    state_ = SUBSCRIBED;

    // do mining.authorize
    string s = Strings::Format("{\"id\": 1, \"method\": \"mining.authorize\","
                               "\"params\": [\"%s\", \"\"]}\n",
                               workerName_.c_str());
    sendData(s);
    return;
  }

  if (state_ == SUBSCRIBED && jresult.boolean() == true) {
    // authorize successful
    state_ = AUTHENTICATED;
    LOG(INFO) << clientInfo_ << " auth success, name: \"" << workerName_;
    return;
  }
}
