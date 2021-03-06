#include <sys/select.h>
#include "async_connection.h"

using namespace rabbitmqcpp;
using namespace std;

void AsyncConnection::close()
{
  {
    boost::mutex::scoped_lock(runMutex_);
    doRun_ = false;
  }

  if(pWorkerThread_)
  {
    pWorkerThread_->join();
  }
}

void AsyncConnection::open(char const * host, int port)
{
  boost::mutex::scoped_lock(runMutex_);
  if(doRun_)
    throw runtime_error("alreading running");

  connect(host, port);

  if(exchangeType_)
    declareExchangeInner(exchange_.c_str(), (*exchangeType_).c_str());

  doRun_ = true;
  pWorkerThread_.reset(new boost::thread(boost::ref(*this)));
}

bool interpretRMQReply(amqp_rpc_reply_t x, char const *context) {
  switch (x.reply_type) 
  {
    case AMQP_RESPONSE_NORMAL:
      return true;

    case AMQP_RESPONSE_NONE:
      std::cerr << context << ": missing RPC reply type!" << std::endl;
      break;

    case AMQP_RESPONSE_LIBRARY_EXCEPTION:
      std::cerr << context << ": " << amqp_error_string(x.library_error) << std::endl;
      break;

    case AMQP_RESPONSE_SERVER_EXCEPTION:
      switch (x.reply.id) 
      {
        case AMQP_CONNECTION_CLOSE_METHOD: 
        {
          amqp_connection_close_t *m = (amqp_connection_close_t *) x.reply.decoded;
          std::cerr << context << ": server connection error " << m->reply_code << ", message: " << (char*) m->reply_text.bytes << std::endl;
          break;
        }
        case AMQP_CHANNEL_CLOSE_METHOD: 
        {
          amqp_channel_close_t *m = (amqp_channel_close_t *) x.reply.decoded;
          std::cerr << context << ": server channel error " << m->reply_code << ", message: " << (char*) m->reply_text.bytes << std::endl;
          break;
        }
        default:
          std::cerr << context << ": unknown server error, method id " <<  x.reply.id << std::endl;
          break;
      }
  }

  return false;
}    


void AsyncConnection::operator()()
{
  if(!conn_)
    throw runtime_error("cannot subscribe, not connected");

  amqp_queue_declare_ok_t *r = amqp_queue_declare(*conn_, 1, amqp_empty_bytes, 0, 0, 0, 1, amqp_empty_table);
  if(!interpretRMQReply(amqp_get_rpc_reply(*conn_), "Queue Declare"))
    return;

  amqp_bytes_t queuename = amqp_bytes_malloc_dup(r->queue);
  if(queuename.bytes == NULL) 
  {
    throw runtime_error("out of memory while copying queue name");
  }

  amqp_queue_bind(*conn_, 1, queuename, amqp_cstring_bytes(exchange_.c_str()), amqp_cstring_bytes(bindingkey_.c_str()), amqp_empty_table);
  if(!interpretRMQReply(amqp_get_rpc_reply(*conn_), "Queue Bind"))
    return;

  amqp_basic_consume(*conn_, 1, queuename, amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
  if(!interpretRMQReply(amqp_get_rpc_reply(*conn_), "Basic Consume"))
    return;

  amqp_frame_t frame;
  int result;

  amqp_basic_deliver_t *d;
  size_t body_target;
  size_t body_received;

  while (true) 
  {
    char * pExchange = NULL;
    char * pRoutingKey = NULL;
    char * pPayload = NULL;

    amqp_maybe_release_buffers(*conn_);

    //wait method frame
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockfd_, &readfds);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000; //half a second timeout

    int rv = select(sockfd_+1, &readfds, NULL, NULL, &tv);

    if (rv == -1) 
    {
      std::cerr << "Select error in RMQ ASyncConnection" << std::endl;
      goto clean_up_and_exit;
    } else if (rv == 0) //timeout
    {
      //check for shutdown
      boost::mutex::scoped_lock(runMutex_);
      if(!doRun_)
        goto clean_up_and_exit;

      continue;
    } 

    result = amqp_simple_wait_frame(*conn_, &frame);
    if (result < 0)
      goto clean_up_and_exit;

    if (frame.frame_type != AMQP_FRAME_METHOD)
      continue;

    if (frame.payload.method.id != AMQP_BASIC_DELIVER_METHOD)
      continue;

    d = (amqp_basic_deliver_t *) frame.payload.method.decoded;

    pExchange = new char[d->exchange.len+1];
    strncpy(pExchange, (char *)d->exchange.bytes, d->exchange.len);
    pExchange[d->exchange.len]='\0';

    pRoutingKey = new char[d->routing_key.len+1];
    strncpy(pRoutingKey, (char *)d->routing_key.bytes, d->routing_key.len);
    pRoutingKey[d->routing_key.len]='\0';

    //wait on header frame
    result = amqp_simple_wait_frame(*conn_, &frame);
    if (result < 0)
      goto clean_up_and_exit;

    if (frame.frame_type != AMQP_FRAME_HEADER) 
    {
      delete[] pExchange;
      delete[] pRoutingKey;

      throw runtime_error("expected header");
    }

    body_target = frame.payload.properties.body_size;
    body_received = 0;

    pPayload = new char[body_target];

    //read in frame content
    while (body_received < body_target) 
    {
      result = amqp_simple_wait_frame(*conn_, &frame);
      if (result < 0)
        break;

      if (frame.frame_type != AMQP_FRAME_BODY) 
      {
        delete[] pExchange;
        delete[] pRoutingKey;
        delete[] pPayload;
        throw runtime_error("expected body!");
      }	  

      strncpy(pPayload + body_received, (char *)frame.payload.body_fragment.bytes, frame.payload.body_fragment.len);
      body_received += frame.payload.body_fragment.len;
      assert(body_received <= body_target);
    }

    if (body_received != body_target) 
    {
      /* Can only happen when amqp_simple_wait_frame returns <= 0 */
      /* We break here to close the connection */
      goto clean_up_and_exit;
    }

    cb_(pExchange, pRoutingKey, pPayload, body_target);

    continue;

clean_up_and_exit:
    if(pExchange)
      delete[] pExchange;
    if(pRoutingKey)
      delete[] pRoutingKey;
    if(pPayload)
      delete[] pPayload;

    break;
  }

}
