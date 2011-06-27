#include "client.h"

using namespace rabbitmqcpp;

Client::Client(char const * host, int port): host_(host), port_(port)
{

}

void Client::connect()
{
  conn_.open(host_.c_str(), port_);
}

void Client::send(char const* exchange, char const* routingkey, char const* message, bool persistent)
{
  conn_.send(exchange, routingkey, message, persistent);
}
