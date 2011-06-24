#include <stdexcept>
#include "client.h"

using namespace rabbitmqcpp;
using namespace std;

Client::~Client()
{
  if(!conn_)
    return;

  amqp_channel_close(*conn_, 1, AMQP_REPLY_SUCCESS);
  amqp_connection_close(*conn_, AMQP_REPLY_SUCCESS);
  amqp_destroy_connection(*conn_);
}

void Client::connect(char const * host, int port)
{
  if(conn_)
  {
    throw runtime_error("client already connected!");
  }

  conn_ = amqp_new_connection();
  int sockfd = amqp_open_socket(host, port);

  if(sockfd < 0)
  {
    conn_=boost::none;
    throw runtime_error("unable to establish connection");
  }

  amqp_set_sockfd(*conn_, sockfd);
  amqp_login(*conn_, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest");
  amqp_channel_open(*conn_, 1);
  amqp_get_rpc_reply(*conn_);
}

void Client::send(char const* exchange, char const* routingkey, char const* message)
{
  send(exchange, routingkey, message, false);
}

void Client::send(char const* exchange, char const* routingkey, char const* message, bool persistent)
{
  if(!conn_)
    throw runtime_error("client not connected, cannot send message");

  amqp_basic_properties_t props;
  props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
  props.content_type = amqp_cstring_bytes("text/plain");

  if(persistent)
    props.delivery_mode = 2;
  else 
    props.delivery_mode = 1;

  if(amqp_basic_publish(*conn_,
    1,
		amqp_cstring_bytes(exchange),
		amqp_cstring_bytes(routingkey),
		0,
		0,
		&props,
		amqp_cstring_bytes(message)) < 0)
  {
    throw runtime_error("error publishing message");
  }
}
