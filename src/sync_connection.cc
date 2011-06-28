#include <stdexcept>
#include "sync_connection.h"

using namespace rabbitmqcpp;
using namespace std;

void SyncConnection::open(char const * host, int port)
{
  boost::mutex::scoped_lock(connMutex_);
  connect(host, port);
}

void SyncConnection::declareExchange(char const * exchange, char const * exchangeType)
{
  boost::mutex::scoped_lock(connMutex_);
  declareExchangeInner(exchange, exchangeType);
}

void SyncConnection::send(char const* exchange, char const* routingkey, char const* message, size_t length, bool persistent)
{
  boost::mutex::scoped_lock(connMutex_);
  if(!conn_)
    throw runtime_error("client not connected, cannot send message");

  amqp_basic_properties_t props;
  props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
  props.content_type = amqp_cstring_bytes("text/plain");

  if(persistent)
    props.delivery_mode = 2;
  else 
    props.delivery_mode = 1;

  amqp_bytes_t msgBytes;
  msgBytes.len = length;
  msgBytes.bytes = (void *) message;

  if(amqp_basic_publish(*conn_,
    1,
		amqp_cstring_bytes(exchange),
		amqp_cstring_bytes(routingkey),
		0,
		0,
		&props,
		msgBytes) < 0)
  {
    throw runtime_error("error publishing message");
  }
}
