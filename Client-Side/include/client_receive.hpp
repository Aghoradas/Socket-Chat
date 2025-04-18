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
    uint32_t buffer_size = 0;
    std::vector<std::string> buffer_history;

    void buffer_receive(int& client_socket){
      // Size of incoming vector
      while(true) {
        recv(client_socket, &buffer_size, sizeof(buffer_size), MSG_WAITALL);
        buffer_size = ntohl(buffer_size);
        bytes_received = (recv(client_socket, data_packet, sizeof(data_packet), 0));
        if (bytes_received > 0) {
          buffer_history.emplace_back(data_packet);
        }
      }
    }
  };
}
#endif
