#include <stdexcept>

#include "connection.h"

using namespace rabbitmqcpp;
using namespace std;

Connection::~Connection()
{
  if(!conn_)
    return;

  amqp_channel_close(*conn_, 1, AMQP_REPLY_SUCCESS);
  amqp_connection_close(*conn_, AMQP_REPLY_SUCCESS);
  amqp_destroy_connection(*conn_);
}

void Connection::connect(char const * host, int port)
{
  if(conn_)
  {
    throw runtime_error("already connected!");
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
