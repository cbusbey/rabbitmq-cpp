#include <iostream>

#include <client.h>

using namespace rabbitmqcpp;

void onMessage(char const * exchange, char const * routingKey, char const * message)
{
  std::cout << exchange << ": <" << routingKey << "> " << message << std::endl; 
}

int main(int argc, char * argv[])
{
  if(argc < 4)
  {
    std::cout << "usage: consumer <host> <port> <exchange> <binding key>" << std::endl;
    return 1;
  }

  char const * host = argv[1];
  int port = atoi(argv[2]);  
  char const * exchange = argv[3];
  char const * bindingkey = argv[4];


  Client c(host, port);
  c.connect();

  Client::TMsgCallback cb = boost::bind( onMessage,  _1, _2, _3);
  c.subscribe(cb, exchange, bindingkey);

  //sit and spin
  while(true) 
  {
    sleep(2);
  }
}
