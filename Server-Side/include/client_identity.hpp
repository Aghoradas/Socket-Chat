#ifndef CLIENT_IDENTITY_HPP
  #include <string>
#define CLIENT_IDENTITY_HPP

namespace defined {
  
  class ClientIdentity {
  public:

    ClientIdentity(std::string time, int client, int heartbeat)
      :login_time(time)
      ,client_connection(client)
      ,heartbeat_connection(heartbeat) {} 

    void get_username(std::string id) {
      username = id;
    }

    std::string client_id() {
      return username;
    }

    int connection() {
      return client_connection;
    }

    int beat_connect() {
      return heartbeat_connection;
    }

    std::string time() {
      return login_time;
    }

  private:

    std::string username;
    std::string login_time;
    int client_connection;
    int heartbeat_connection;

  }; 

}
#endif
