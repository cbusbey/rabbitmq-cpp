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
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

namespace rabbitmqcpp
{
  class AbstractConnection
  {
    public:
      virtual ~AbstractConnection() = 0;
      void open(char const * host, int port); 

    protected:
      boost::mutex connMutex_;
      boost::optional<amqp_connection_state_t> conn_;
  };

  ///
  /// Synchonous Connection 
  ///
  class SyncConnection : public AbstractConnection
  {
    public:
    void send(char const* exchange, char const* routingkey, char const* message, bool persistent = false);

    //TODO: synchonous receive
  };

  ///
  /// Connection relays subscription to async callback
  ///
  class AsyncConnection : public AbstractConnection
  {
    public:
      typedef boost::function<void(char const * exchange, char const * routingkey, char const * message)> TMsgCallback;

      //TODO: allow multiple subscription types?
      void run(const TMsgCallback & cb);
      void stop();
      void operator()();

      AsyncConnection(char const * exchange, char const * bindingkey): 
        exchange_(exchange),
        bindingkey_(bindingkey),
        doRun_(false) {}

    private:
      const std::string exchange_;
      const std::string bindingkey_;

      boost::mutex runMutex_;
      bool doRun_;
      boost::scoped_ptr<boost::thread> pWorkerThread_;
  };
}

#endif //__RABBITMQ_CPP_CLIENT_H
