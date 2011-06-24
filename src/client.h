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
/// Below are two types of clients. One Synchronous client that makes blocking
/// calls to publish/subscribe to RabbitMQ. The other is a asynch client that relays
/// subscription callbacks on a separate thread.

#ifndef __RABBITMQ_CPP_CLIENT_H
#define __RABBITMQ_CPP_CLIENT_H

#include <stdlib.h>
#include <stdint.h>

#include <amqp.h>
#include <amqp_framing.h>

#include <boost/optional.hpp>
#include <boost/function.hpp>

namespace rabbitmqcpp
{
  class AbstractClient
  {
    public:
      virtual ~AbstractClient() = 0;
      void connect(char const * host, int port); 

    protected:
      boost::optional<amqp_connection_state_t> conn_;
  };

  ///
  /// Synchonous Client 
  ///
  class SyncClient : public AbstractClient
  {
    public:

    /// sends message w/o persistence
    void send(char const* exchange, char const* routingkey, char const* message);
    void send(char const* exchange, char const* routingkey, char const* message, bool persistent);

    //TODO: synchonous receive
  };

  ///
  /// Client relays subscription to async callback
  ///
  class AsyncClient : public AbstractClient
  {
    public:
      typedef boost::function<void(char const * exchange, char const * routingkey, char const * message)> TMsgCallback;

      //TODO: allow multiple subscription types?
      void run(char const * exchange, char const * bindingKey, const TMsgCallback & cb);
      void stop();
  };
}

#endif //__RABBITMQ_CPP_CLIENT_H
