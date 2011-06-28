#ifndef __RABBITMQ_CPP_ASYNC_CONNECTION_H
#define __RABBITMQ_CPP_ASYNC_CONNECTION_H

#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/scoped_ptr.hpp>

#include "connection.h"

namespace rabbitmqcpp
{
  /// Connection relays subscription to async callback
  class AsyncConnection : public Connection
  {
    public:
      //NOTE: responsibility of callback to free up exchange, routingkey, and message (delete[])
      typedef boost::function<void(char const * exchange, char const * routingkey, char const * message, size_t messageLen)> TMsgCallback;

      //exchangeType, if given, will be used to declare the exchange on opening the connection
      AsyncConnection(TMsgCallback & cb, char const * exchange, char const * bindingkey, boost::optional<std::string> exchangeType =
          boost::optional<std::string>()): 
        cb_(cb),
        exchange_(exchange),
        bindingkey_(bindingkey),
        exchangeType_(exchangeType),
        doRun_(false) {}

      virtual ~AsyncConnection() {close();}

      virtual void open(char const * host, int port);
      virtual void close();

      void operator()();

    private:
      TMsgCallback& cb_;
      const std::string exchange_;
      const std::string bindingkey_;
      const boost::optional<std::string> exchangeType_;

      boost::mutex runMutex_;
      bool doRun_;
      boost::scoped_ptr<boost::thread> pWorkerThread_;
  };
}

#endif //__RABBITMQ_CPP_ASYNC_CONNECTION_H
