#ifndef __RABBITMQ_CPP_SYNC_CONNECTION_H
#define __RABBITMQ_CPP_SYNC_CONNECTION_H

#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include "connection.h"

namespace rabbitmqcpp
{
  /// Synchonous Connection 
  class SyncConnection : public Connection
  {
    public:
      virtual ~SyncConnection() {}
      virtual void open(char const * host, int port); 
      virtual void close() {}

      void send(char const* exchange, char const* routingkey, char const* message, bool persistent);
    private:
      boost::mutex connMutex_;
  };
}

#endif //__RABBITMQ_CPP_SYNC_CONNECTION_H
