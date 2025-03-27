#include <iostream>
#include <chrono>

#include <cstring>
#include <ctime>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string>
#include <vector>
#include <sstream>
#include <thread>

#include "../include/buffer_history.hpp"
#include "../include/server_output.hpp"
#include "../include/server_output.hpp"

int G_PORT = 8080;
int G_HEARTBEAT = 8081;
std::vector<int> G_HEARTBEAT_CLIENTS;
std::vector<int> G_CLIENTS;
const char G_GREET[50] = "Welcone to server.. '!close' to close connection.";

void cls() {
  system("clear");
}

void server_time(std::string& current_time) {
  try {
    std::chrono::time_point<std::chrono::system_clock> now;
    now = std::chrono::system_clock::now();
    std::time_t raw_time = std::chrono::system_clock::to_time_t(now);
    std::tm* current = std::localtime(&raw_time);
    current_time = std::string(current->tm_zone) + " "
                   + std::to_string(current->tm_hour) + ":" + std::to_string(current->tm_min)
                   + " - " + std::to_string(current->tm_mon) + "/" + std::to_string(current->tm_mday)
                   + "/" + std::to_string(current->tm_year);
  } catch(std::runtime_error &error) {
    std::cerr << "-error getting rawtime: " << error.what() << std::endl;
    current_time = "-------";
  }
}

void heartbeat_pulse() {
  while (true) {
    const char* ping_message = "PING";
    if (G_HEARTBEAT_CLIENTS.size() > 0) {
      std::this_thread::sleep_for(std::chrono::seconds(5));
      for (int& socket : G_HEARTBEAT_CLIENTS) {
        send(socket, ping_message, strlen(ping_message), 0);
      }
    }
  }
}

// Sends a PING message to client
void heart_beat_handler(int heartbeat_connection, int client_connection) {
  while(true) {

    char buffer[10] = {0};
    int bytes_received = recv(heartbeat_connection, buffer, sizeof(buffer), MSG_DONTWAIT);

    if (bytes_received == 0) {
      if (strncmp(buffer, "PONG", 4) != 0) {
        std::cerr << "\n-No PONG received, closing connection to: " << client_connection <<std::endl;
        close(heartbeat_connection);
        break;
      }
    }
  }
  close(client_connection);

}

// Takes in received message from client to prepare it to the screen.
std::vector<std::string> message_parser(std::string &to_be_parsed) {
  std::vector<std::string> member_message;
  std::stringstream ss(to_be_parsed);
  std::string part;
  while (std::getline(ss, part, '|')) {
    member_message.emplace_back(part);
  }
  return member_message;
}

// This will process comands for the server
void command_menu(std::vector<std::string> check_command, const int client_connection, std::string& username) {
  check_command[0].erase(0, 1);
  std::cout << "\n" << check_command[0] << std::endl;
  if (check_command[0] == "username") {
    std::cout << "something" << std::endl;
    username = check_command[1];
  } else {
    std::cout << "-something went wrong" << std::endl;
  }
}

int handle_client(const int client_connection, buffer::History& buffer_messages) {
  std::string current_time;
  char data_packet[1025];
  std::string server_to_client;
  std::vector<std::string> check_command;
  int32_t message_size;
  std::string username;
  sending::Output send_buffer;
  std::queue<std::string> lines;


  std::thread([&](){ send_buffer.buffer_send(client_connection, buffer_messages); }).detach();

  while(true) {
    
    server_time(current_time);
    ssize_t bytes_received;
    memset(data_packet, 0, sizeof(data_packet));
    bytes_received = recv(client_connection, data_packet, sizeof(data_packet), 0);
    data_packet[bytes_received] = '\0';
    std::string from_client(data_packet);
    buffer_messages.store_data(from_client);

    // Handling any server commands from clients
    check_command = message_parser(from_client);
    if (check_command.size() > 2) {
      std::cout << std::endl;
    } else if (check_command.at(1) == "!close") {
      std::string temp = check_command.at(0)+" has disconnected";

      buffer_messages.store_data(temp);
      shutdown(client_connection, SHUT_RDWR);
      close(client_connection);
      break;
    } else if (check_command.at(1)[0] == '!') {
      command_menu(check_command, client_connection, username);
    }

    if (bytes_received > 0 && std::string(data_packet) == "Client connected...") {
      send(client_connection, G_GREET, sizeof(G_GREET) - 1, 0);
    } else if (bytes_received > 0) {
      data_packet[bytes_received] = '\0';
      std::cout << "-received: " << check_command[1] << std::endl;
      server_to_client = "*Server|You sent: completed..";
      buffer_messages.store_data(server_to_client);
    } else {
      std::cout << "-client disconnected: ";
      break;
    }
    std::cout << "\n================================================" << std::endl;
    buffer_messages.print_buffer(current_time);
    std::cout << "================================================" << std::endl;


  }
  shutdown(client_connection, SHUT_RDWR);
  close(client_connection);
  return 0;

}


int main() {
  cls();
  int attempts;

  // Creating normal socket
  const int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    std::cerr << "-error openning socket" << std::endl;
    return -1;
  }

  // Creating heartbeat socket
  const int heartbeat_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (heartbeat_fd < 0) {
    std::cerr << "-error openning socket" << std::endl;
    return -1;
  }

  // Keeping socket open for quick reconnection
  int reuse = 1;
  if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    std::cerr << "-socket 'server_fd' option 'SO_REUSEADDR' failed" << std::endl;
  }
  reuse = 1;
  if (setsockopt(heartbeat_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))< 0) {
    std::cerr << "-socket 'heartbeat_fd' option 'SO_REUSEADDR' failed" << std::endl;
  }

  // Setting address to sockets
  struct sockaddr_in server_address;
  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(G_PORT);
  server_address.sin_addr.s_addr = htonl(INADDR_ANY);

  struct sockaddr_in heartbeat_address;
  memset(&heartbeat_address, 0, sizeof(heartbeat_address));
  heartbeat_address.sin_family = AF_INET;
  heartbeat_address.sin_port = htons(G_HEARTBEAT);
  heartbeat_address.sin_addr.s_addr = htonl(INADDR_ANY);

  // Binding address to socket
  if(bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
    std::cerr << "-failed to bind server address" << std::endl;
    close(server_fd);
    return -1;
  }
  if(bind(heartbeat_fd, (struct sockaddr *)&heartbeat_address, sizeof(heartbeat_address)) < 0) {
    std::cerr << "-failed to bind heartbeat address" << std::endl;
    close(heartbeat_fd);
    return -1;
  }

  // Listening to socket
  listen(server_fd, 5);
  listen(heartbeat_fd, 5);
  std::cout << "-listening to port: " << ntohs(server_address.sin_port) << std::endl;
  std::cout << "-listening to port: " << ntohs(heartbeat_address.sin_port) << std::endl;

  struct sockaddr_in client_address;
  socklen_t client_address_length = sizeof(client_address);

  std::thread heart(heartbeat_pulse);
  heart.detach();

  std::string current_time;
  buffer::History buffer_messages;

  while(true) {

    server_time(current_time);
    std::cout << current_time << std::endl;

    // Initiating and accepting client_side connection
    int client_connection = accept(server_fd, (struct sockaddr*)&client_address, &client_address_length);
    if (client_connection < 0) {
      std::cerr << "-error accepting client connection" << std::endl;
      return -1;
    }
    G_CLIENTS.emplace_back(client_connection);
    std::cout << "-connection accepted from: " << inet_ntoa(client_address.sin_addr) << " : "
              << ntohs(client_address.sin_port) << std::endl;

    // Initiating client_side heartbeat connection
    int heartbeat_connection = accept(heartbeat_fd, (struct sockaddr*)&client_address, &client_address_length);
    if (heartbeat_connection < 0) {
      std::cerr << "-error accepting heartbeat connection" << std::endl;
      return -1;
    }
    G_HEARTBEAT_CLIENTS.emplace_back(heartbeat_connection);
    std::cout << "-heartbeat established" << std::endl;

    // Handle client interactions with a multi_thread function
    std::thread client_thread(handle_client, client_connection, std::ref(buffer_messages));
    client_thread.detach();

    // Handle heartbeats PING/PONG
    std::thread heartbeat_thread(heart_beat_handler, heartbeat_connection, client_connection);
    heartbeat_thread.detach();

  }
  std::cout << "-server disconnected" << std::endl;
  sleep(1);
  std::cout << "-goodbye" << std::endl;
  close(server_fd);
  return 0;
}
