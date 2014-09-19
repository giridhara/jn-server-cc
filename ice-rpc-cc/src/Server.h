/*
 * Server.h
 *
 *  Created on: Sep 1, 2014
 *      Author: psarda
 */

#ifndef SERVER_H_
#define SERVER_H_

#include <Ice/Ice.h>
#include <string>


using namespace Ice;
using namespace std;

namespace icerpc
{

class Server : public Ice::Application
{
    static const string server_config;
public:
    Server(const ObjectPtr instance, const string& protocol, const string bind_address, int port, int num_handlers)
        :adapter(0),
         instance(instance),
         protocol(protocol),
         bind_address(bind_address),
         port(port),
         num_handlers(num_handlers),
         running(false)
    {}
    virtual ~Server();
    int createConfigFile();
    bool deleteConfigFile();
    void start();
    void stop();
    void join();
    int run(int argc, char*[]);
private:
    ObjectAdapterPtr adapter;
    ObjectPtr instance;
    string protocol;
    string bind_address;
    int port;
    int num_handlers;
    bool running;
};

} /* namespace icerpc */

#endif /* SERVER_H_ */
