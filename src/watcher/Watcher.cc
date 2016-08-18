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

#include <glog/logging.h>

#include "Watcher.h"

ClientContainer *gClientContainer = nullptr;

void handler(int sig) {
  if (gClientContainer) {
    gClientContainer->stop();
  }
}

void usage() {
  fprintf(stderr, "Usage:\n\tpoolwatcher -p \"pools.json\" -l \"log_dir\"\n");
}

int main(int argc, char **argv) {
  char *optLogDir = NULL;
  char *optPools  = NULL;
  int c;

  if (argc <= 1) {
    usage();
    return 1;
  }

  while ((c = getopt(argc, argv, "p:l:h")) != -1) {
    switch (c) {
      case 'p':
        optPools = optarg;
        break;
      case 'l':
        optLogDir = optarg;
        break;
      case 'h': default:
        usage();
        exit(0);
    }
  }

  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);
  FLAGS_log_dir = string(optLogDir);
  FLAGS_max_log_size = 50;  // max log file size 50 MB
  FLAGS_logbuflevel = -1;
  FLAGS_stop_logging_if_full_disk = true;

  signal(SIGTERM, handler);
  signal(SIGINT,  handler);

  try {
    gClientContainer = new ClientContainer(string(optPools));

    if (gClientContainer->initPoolClients() == 0) {
      LOG(FATAL) << "init pools failure";
    } else {
      gClientContainer->run();
    }
    delete gClientContainer;
  }
  catch (std::exception & e) {
    LOG(FATAL) << "exception: " << e.what();
    return 1;
  }

  google::ShutdownGoogleLogging();
  return 0;
}
