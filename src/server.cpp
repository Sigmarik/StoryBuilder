#include "server.h"

#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#include <future>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <vector>

#include "config.h"
#include "console/io.h"
#include "logger/debug.h"
#include "logger/logger.h"

int as_server() { return EXIT_SUCCESS; }
