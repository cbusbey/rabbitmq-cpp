#include <connection.h>

using namespace rabbitmqcpp;

int main(int argc, char * argv[])
{
  SyncConnection c;
  c.open("localhost", 5672);
//  sleep(2);
// c.open("localhost", 5672); //throws exception (rightfully so)
}
