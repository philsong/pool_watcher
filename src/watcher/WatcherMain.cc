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
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>

#include <fstream>
#include <streambuf>

#include <glog/logging.h>

#include "Watcher.h"
#include "MySQLConnection.h"


ClientContainer *gClientContainer = nullptr;

void handler(int sig) {
  if (gClientContainer) {
    gClientContainer->stop();
  }
}

void usage() {
  fprintf(stderr, "Usage:\n\tpoolwatcher -m \"mysql.json\"\n");
}

int main(int argc, char **argv) {
  char *optMysql = NULL;
  int c;

  if (argc <= 1) {
    usage();
    exit(EXIT_SUCCESS);
  }

  while ((c = getopt(argc, argv, "m:h")) != -1) {
    switch (c) {
      case 'm':
        optMysql = optarg;
        break;
      case 'h': default:
        usage();
        exit(EXIT_SUCCESS);
    }
  }

  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  // Set whether log messages go to stderr instead of logfiles
  FLAGS_logtostderr = true;
  FLAGS_logbuflevel = -1;
  FLAGS_stop_logging_if_full_disk = true;

  signal(SIGTERM, handler);
  signal(SIGINT,  handler);

  try {
    JsonNode j;  // mysql conf json

    // parse mysql.json to dbInfo
    std::ifstream mysqlConf(optMysql);
    string mysqlJsonStr((std::istreambuf_iterator<char>(mysqlConf)),
                        std::istreambuf_iterator<char>());
    if (!JsonNode::parse(mysqlJsonStr.c_str(),
                         mysqlJsonStr.c_str() + mysqlJsonStr.length(), j)) {
      exit(EXIT_FAILURE);
    }

    MysqlConnectInfo dbInfo(j["host"].str(), j["port"].int16(),
                            j["username"].str(), j["password"].str(),
                            j["dbname"].str());
    gClientContainer = new ClientContainer(dbInfo);

    if (gClientContainer->init() == false) {
      LOG(ERROR) << "init failure";
    } else {
      gClientContainer->run();
    }
    delete gClientContainer;
  }
  catch (std::exception & e) {
    LOG(ERROR) << "exception: " << e.what();
    return EXIT_FAILURE;
  }

  google::ShutdownGoogleLogging();
  return EXIT_SUCCESS;
}
