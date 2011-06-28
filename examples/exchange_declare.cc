#include <iostream>
#include <client.h>

using namespace rabbitmqcpp;

int main(int argc, char * argv[])
{
  if(argc < 4)
  {
    std::cout << "usage: exchange_declare <host> <port> <exchange> <exchange_type>" << std::endl;
    return 1;
  }

  char const * host = argv[1];
  int port = atoi(argv[2]);  
  char const * exchange = argv[3];
  char const * exchange_type = argv[4];

  Client c(host, port);
  c.connect();

  c.declareExchange(exchange, exchange_type);
}
