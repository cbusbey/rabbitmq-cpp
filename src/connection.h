/// NOTE: The below is from the Threading heading of the rabbitmq-c README

/*
   You cannot share a socket, an `amqp_connection_state_t`, or a channel
   between threads using `librabbitmq`. The `librabbitmq` library is
   built with event-driven, single-threaded applications in mind, and
   does not yet cater to any of the requirements of `pthread`ed
   applications.

  Your applications instead should open an AMQP connection (and an
  associated socket, of course) per thread. If your program needs to
  access an AMQP connection or any of its channels from more than one
  thread, it is entirely responsible for designing and implementing an
  appropriate locking scheme. It will generally be much simpler to have
  a connection exclusive to each thread that needs AMQP service.
*/

/// OOF.  
///
/// Deriving from the below class are two types of connections. One Synchronous 
/// connection that makes blocking calls to publish/subscribe to RabbitMQ. 
/// The other is a asynch connection that relays subscription callbacks on a separate thread.

#ifndef RABBITMQ_CPP_CONNECTION_H
#define RABBITMQ_CPP_CONNECTION_H

#include <stdlib.h>
#include <stdint.h>

#include <amqp.h>
#include <amqp_framing.h>

#include <boost/optional.hpp>

namespace rabbitmqcpp
{
  class Connection
  {
    public:
      virtual ~Connection() = 0;

      virtual void open(char const * host, int port) = 0; 
      virtual void close() = 0;

    protected:
      void declareExchangeInner(char const * exchange, char const * routingkey);
      void connect(char const * host, int port); 
      boost::optional<amqp_connection_state_t> conn_;
  };
}

#endif //RABBITMQ_CPP_CONNECTION_H
