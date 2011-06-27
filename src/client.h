#ifndef RABBITMQ_CPP_CLIENT_H
#define RABBITMQ_CPP_CLIENT_H

#include <string>
#include "sync_connection.h"

namespace rabbitmqcpp
{
  class Client
  {
    public:
      Client(char const * host, int port);
      void connect();

      //TODO: synchonous receive
      void send(char const* exchange, char const* routingkey, char const* message, bool persistent = false);

    private:
      const std::string host_;
      const int port_;
      SyncConnection conn_;
  };
}

#endif //RABBITMQ_CPP_CLIENT_H
