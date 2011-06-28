#ifndef RABBITMQ_CPP_CLIENT_H
#define RABBITMQ_CPP_CLIENT_H

#include <string>
#include <vector>

#include "sync_connection.h"
#include "async_connection.h"

namespace rabbitmqcpp
{
  class Client
  {
    public:
      Client(char const * host, int port);
      void connect();

      //TODO: synchonous receive
      void send(char const* exchange, char const* routingkey, char const* message, size_t messageLength, bool persistent = false);

      typedef AsyncConnection::TMsgCallback TMsgCallback;
      void subscribe(TMsgCallback& cb, char const * exchange, char const * bindingkey);

    private:
      const std::string host_;
      const int port_;
      SyncConnection conn_;

      std::vector< boost::shared_ptr<AsyncConnection> > subscriptions_;
  };
}

#endif //RABBITMQ_CPP_CLIENT_H
