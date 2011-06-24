#include <stdexcept>

#include "connection.h"

using namespace rabbitmqcpp;
using namespace std;

AbstractConnection::~AbstractConnection()
{
  if(!conn_)
    return;

  amqp_channel_close(*conn_, 1, AMQP_REPLY_SUCCESS);
  amqp_connection_close(*conn_, AMQP_REPLY_SUCCESS);
  amqp_destroy_connection(*conn_);
}

void AbstractConnection::open(char const * host, int port)
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

void SyncConnection::send(char const* exchange, char const* routingkey, char const* message)
{
  send(exchange, routingkey, message, false);
}

void SyncConnection::send(char const* exchange, char const* routingkey, char const* message, bool persistent)
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

void AsyncConnection::stop()
{

}

void AsyncConnection::run(char const * exchange, char const * bindingkey, const TMsgCallback & cb)
{
  if(!conn_)
    throw runtime_error("cannot subscribe, not connected");

  amqp_queue_declare_ok_t *r = amqp_queue_declare(*conn_, 1, amqp_empty_bytes, 0, 0, 0, 1, amqp_empty_table);
  amqp_get_rpc_reply(*conn_);

  amqp_bytes_t queuename = amqp_bytes_malloc_dup(r->queue);
  if(queuename.bytes == NULL) 
  {
    throw runtime_error("out of memory while copying queue name");
  }

  amqp_queue_bind(*conn_, 1, queuename, amqp_cstring_bytes(exchange), amqp_cstring_bytes(bindingkey), amqp_empty_table);
  amqp_get_rpc_reply(*conn_);

  amqp_basic_consume(*conn_, 1, queuename, amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
  amqp_get_rpc_reply(*conn_);

  amqp_frame_t frame;
  int result;

  amqp_basic_deliver_t *d;
  amqp_basic_properties_t *p;
  size_t body_target;
  size_t body_received;

  while (true) 
  {
    amqp_maybe_release_buffers(*conn_);

    //wait method frame
    result = amqp_simple_wait_frame(*conn_, &frame);
    if (result < 0)
      break;

    if (frame.frame_type != AMQP_FRAME_METHOD)
      continue;

    if (frame.payload.method.id != AMQP_BASIC_DELIVER_METHOD)
      continue;

    d = (amqp_basic_deliver_t *) frame.payload.method.decoded;
    //printf("Delivery %u, exchange %.*s routingkey %.*s\n",
    //  (unsigned) d->delivery_tag,
    //  (int) d->exchange.len, (char *) d->exchange.bytes,
	  //  (int) d->routing_key.len, (char *) d->routing_key.bytes);

    //wait on header frame
    result = amqp_simple_wait_frame(*conn_, &frame);
    if (result < 0)
      break;

    if (frame.frame_type != AMQP_FRAME_HEADER) 
    {
      throw runtime_error("expected header");
    }

    p = (amqp_basic_properties_t *) frame.payload.properties.decoded;
    if (p->_flags & AMQP_BASIC_CONTENT_TYPE_FLAG) 
    {
    //  printf("Content-type: %.*s\n",
	   //    (int) p->content_type.len, (char *) p->content_type.bytes);
    }
    //printf("----\n");

    body_target = frame.payload.properties.body_size;
    body_received = 0;

    //read in frame content
    while (body_received < body_target) 
    {
      result = amqp_simple_wait_frame(*conn_, &frame);
      if (result < 0)
        break;

      if (frame.frame_type != AMQP_FRAME_BODY) 
      {
        throw runtime_error("expected body!");
      }	  

      body_received += frame.payload.body_fragment.len;
      assert(body_received <= body_target);

      //amqp_dump(frame.payload.body_fragment.bytes,
		  //frame.payload.body_fragment.len);
    }

    if (body_received != body_target) 
    {
      /* Can only happen when amqp_simple_wait_frame returns <= 0 */
      /* We break here to close the connection */
      break;
    }
  }
}
