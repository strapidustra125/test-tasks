#include <iostream>
#include "zmq.hpp"

using namespace std;

int main(int argc, char *argv[])
{
  cout << "Ну здарова, отец..." << endl;


  zmq::context_t ctx;
  zmq::socket_t sock(ctx, zmq::socket_type::push);

  sock.bind("inproc://test");
  sock.send(zmq::str_buffer("Hello, world"), zmq::send_flags::dontwait);

  return 0;
}
