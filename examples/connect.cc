#include <client.h>

using namespace rabbitmqcpp;

int main(int argc, char * argv[])
{
  SyncClient c;
  c.connect("localhost", 5672);
//  sleep(2);
// c.connect("localhost", 5672);
}
