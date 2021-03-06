/*
 * JournalNodeHttpServer.h
 *
 *  Created on: Sep 17, 2014
 *      Author: psarda
 */

#ifndef JOURNALNODEHTTPSERVER_H_
#define JOURNALNODEHTTPSERVER_H_

#include "../util/mongoose.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <assert.h>
#include "JournalNode.h"
#include "Journal.h"

namespace JournalServiceServer
{

using std::string;

class JournalNodeHttpServer
{
public:
    JournalNodeHttpServer();
    virtual ~JournalNodeHttpServer();

    static string listening_port;
    static volatile int signal_received;

    static int start_httpserver(string port);
    static void* run_httpserver(void* arg);
    static void signal_handler(int sig_num);
};

} /* namespace JournalServiceServer */

#endif /* JOURNALNODEHTTPSERVER_H_ */
