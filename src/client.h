#ifndef __RABBITMQ_CPP_CLIENT_H
#define __RABBITMQ_CPP_CLIENT_H

#include <stdlib.h>
#include <stdint.h>

#include <amqp.h>
#include <amqp_framing.h>

#include <boost/optional.hpp>

namespace rabbitmqcpp
{
  class Client
  {
    public:
      void connect(char const * host, int port); 
      virtual ~Client();

      /// sends message w/o persistence
      void send(char const* exchange, char const* routingkey, char const* message);

      void send(char const* exchange, char const* routingkey, char const* message, bool persistent);

    private:
      boost::optional<amqp_connection_state_t> conn_;
  };
}

#endif //__RABBITMQ_CPP_CLIENT_H
