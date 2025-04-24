#ifndef SERVER_OUTPUT_HPP

#include <iostream>
#include <string>
#include <netinet/in.h>

#include "buffer_history.hpp"
#include "client_collection.hpp"

#define SERVER_OUTPUT_HPP


namespace sending {
  class Output {
  public:
    uint32_t bytes_sent;
    std::string data_packet;

    void buffer_send(buffer::History& buffer_messages) {
      printf("\n-buffer_send has started...\n");
      while (true) {
        if (buffer_messages.queue_size() > 0) {
          data_packet = buffer_messages.get_data();
          uint32_t size = data_packet.length();
          for (auto& client : G_CLIENTS.get_clients()) {
            bytes_sent = 0;
            size = htonl(size);
            send(client.connection(), &size, sizeof(size), 0);
            size = ntohl(size);
            bytes_sent = send(client.connection(), data_packet.c_str(), data_packet.length(), 0);
            if (bytes_sent < 0) {
              std::cerr << "-error sending data: " << data_packet << std::endl;
            }
          }
      }
    }
  }
  };
}
#endif
