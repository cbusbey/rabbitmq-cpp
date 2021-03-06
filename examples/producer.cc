#include <iostream>
#include <client.h>

using namespace rabbitmqcpp;

int main(int argc, char * argv[])
{
  if(argc < 5)
  {
    std::cout << "usage: producer <host> <port> <exchange> <routing key> <message>" << std::endl;
    return 1;
  }

  char const * host = argv[1];
  int port = atoi(argv[2]);  
  char const * exchange = argv[3];
  char const * routingkey = argv[4];
  char const * message = argv[5];

  Client c(host, port);
  c.connect();

  c.send(exchange, routingkey, message, strlen(message));
}
