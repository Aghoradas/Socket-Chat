#ifndef CLIENT_RECEIVE_HPP
#define CLIENT_RECEIVE_HPP

#include <string>
#include <vector>
#include <netinet/in.h>

namespace buffer {
  class Receiving {
  public:
    int bytes_received;
    char* data_packet;

    void buffer_receive(int& client_socket, std::vector<std::string>& buffer_history){
      bytes_received = (recv(client_socket, data_packet, sizeof(data_packet), 0));
    
    }

  };
}


#endif
