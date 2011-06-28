#include "client.h"

using namespace rabbitmqcpp;

Client::Client(char const * host, int port): host_(host), port_(port)
{

}

void Client::connect()
{
  conn_.open(host_.c_str(), port_);
}

void Client::send(char const* exchange, char const* routingkey, char const* message, size_t messageLength, bool persistent)
{
  conn_.send(exchange, routingkey, message, messageLength, persistent);
}

void Client::declareExchange(char const * exchange, char const * exchangeType)
{
  conn_.declareExchange(exchange, exchangeType);
}

void Client::subscribe(TMsgCallback & cb, char const * exchange, char const * bindingkey)
{
  boost::shared_ptr< AsyncConnection>  pSub(new AsyncConnection(cb, exchange, bindingkey));
  pSub->open(host_.c_str(), port_);
  subscriptions_.push_back(pSub);
}
