/******************************************
 * CHUI-FACE
 *
 * This is the version UI equipped client
 ******************************************/

#include <chrono>
#include <cstdio>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <wx/wx.h>
#include <wx/gbsizer.h>

#include <iostream>
#include <thread>

#include "../include/client_buffer.hpp"
typedef receiver::Buffer buf;

int G_PORT = 8080;
int G_HEARTBEAT_PORT = 8081;
std::string G_IP_ADDRESS = "127.0.0.1";



/* HEARTBEAT LISTEN
 ******************/
void heartbeat_listen(const int heartbeat_connection, const int client_connection) {
  wxLogMessage("heartbeat listening");
  char buffer[10];
  const char* pong_message = "PONG";
  int time_out = 0;
  int time_limit = 5;

  while (client_connection > 0) {
    time_out++;
    memset(buffer, 0, sizeof(buffer));
    int bytes_received = recv(heartbeat_connection, buffer, sizeof(buffer), 0);
    if (bytes_received > 0 && strncmp(buffer, "PING", 4) == 0) {
      time_out = 0;
      send(heartbeat_connection, pong_message, strlen(pong_message), 0);
    }
    if (time_out == time_limit) {
      wxLogMessage("-no connection with server");
      shutdown(heartbeat_connection, SHUT_RDWR);
      close(heartbeat_connection);
      break;
    }
  }
  wxLogMessage("-no connection with server");
  shutdown(client_connection, SHUT_RDWR);
  close(client_connection);
}



/* LOGINUSERNAME CLASS
*************************/
class LoginUsername : public wxFrame {
private:

public:
  LoginUsername(wxFrame* parent);
  wxTextCtrl* user_input;
  std::string username;

  void get_username(wxCommandEvent& event) {
    username = user_input->GetValue();
  }
}; // LoginUsername class




/* CLIENTAPP CLASS
 *******************/
class ClientApp : public wxApp {
public:
  virtual bool OnInit();
};



/* MAINFRAME CLASS
*********************/
class MainFrame : public wxFrame {
private:
  DECLARE_EVENT_TABLE();
  int client_socket;
  int heartbeat_socket;

public:
  MainFrame(std::string& init_username);
  buf         client_buffer;
  wxListBox*  chat_screen;
  wxTextCtrl* user_input;
  wxTextCtrl* edit_username;
  std::string username = "someone";
  std::string new_line;

  void close_window(wxCommandEvent& event);
  void create_socket_connection(wxCommandEvent& event);
  void disconnect_connection(wxCommandEvent& event);
  void submit_chat(wxCommandEvent& event);
  
  ~MainFrame() {
    delete chat_screen;
    delete user_input; 
  }

  void get_username(wxCommandEvent& event) {
    username = edit_username->GetValue();
  }


  // EDIT LOGIN
  void edit_login(wxCommandEvent& event) {
    /*
    LoginUsername* login = new LoginUsername(this);
    login->Show();
    */
    return;
  }


  // REFRESH CHAT
  void refresh_chat() {
      chat_screen->Clear();
      if (client_buffer.buffer_empty()) {
        return;
      }
      for (int line = 0; line < client_buffer.buffer_size(); line++) {
        chat_screen->Append(client_buffer.get_buffer(line));
      }
  }


  // RECEIVE FROM SERVER
  void receiving_from_server(const int& client_socket) {
    std::string message = "!username|" + username + "|Client connected...";
    std::cout << "-sent greeting: " << message;
    std::cout << "\n-receiving from server\n";

    wxLogMessage("RECEIVING FROM SERVER");

    // Send initial connection messge to server
    int init_sent = 0;
    init_sent = send(client_socket, message.c_str(), message.length(), 0);
    if (init_sent <= 0) {
      std::cerr << "\n-error sending INITIAL message to server\n";
      return;
    }
    std::cout << "\n-message sent-\n";
    

    // data_packet and data_str
    char        data_packet[1025];
    std::string data_str;
    uint32_t    buffer_size;
    uint32_t    bytes_data;

    memset(data_packet, 0, sizeof(data_packet));

    chat_screen->Clear();
    while (client_socket > 0) {
      buffer_size = 0;
      bytes_data  = 0;
      memset(data_packet, 0, sizeof(data_packet));
      while (buffer_size == 0) {
        recv(client_socket, &buffer_size, sizeof(buffer_size), 0);
      }
      buffer_size = ntohl(buffer_size);
      while (bytes_data < buffer_size) {
        bytes_data = recv(client_socket, data_packet, buffer_size, 0);
      }
      if (buffer_size > 0) {
        data_packet[bytes_data] = '\0';
        data_str = std::string(data_packet);
        chat_screen->Append(data_packet);
        std::cout << "received message stored: " << data_str << std::endl;
      }
    }
  }


  // INITIATE THREADS
  void start_threads() {

    std::thread heartbeat_thread(heartbeat_listen, heartbeat_socket, client_socket);
    heartbeat_thread.detach();

    std::thread([&](){ receiving_from_server(client_socket); }).detach();

    //std::thread([&](){ loop_refresh(client_socket); }).detach();
    wxLogMessage("Connected");
  }

}; // MainFrame class


const int FILE_QUIT  = wxID_EXIT;
const int USERNAME   = 100;
const int LISTBOX    = 101;
const int BUTTON_ID  = 102;
const int CONNECT    = 103;
const int DISCONNECT = 104;
 
BEGIN_EVENT_TABLE(MainFrame, wxFrame)
  EVT_MENU(FILE_QUIT, MainFrame::close_window)
  EVT_MENU(USERNAME, MainFrame::edit_login)
  EVT_MENU(CONNECT, MainFrame::create_socket_connection)
  EVT_MENU(DISCONNECT, MainFrame::disconnect_connection)
END_EVENT_TABLE()


/* CREATE_SOCKET_CONNECTION
 ****************************/
void MainFrame::create_socket_connection(wxCommandEvent& event) {
  // CREATING SOCKETS
  // ----------------
  client_socket = socket(AF_INET, SOCK_STREAM, 0);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  if (client_socket < 0) {
    wxLogMessage("-client socket failed to inititatie");
    return;
  }
  std::cout << "-client socket connected\n";
  heartbeat_socket = socket(AF_INET, SOCK_STREAM, 0);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  if (heartbeat_socket < 0) {
    wxLogMessage("-heartbeat socket failed to initiate");
  }
  std::cout << "\n-heartbeat socket connected\n";

  // SOCKET IP-ADDRESS
  // -----------------
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(G_PORT);
  if (inet_pton(AF_INET, G_IP_ADDRESS.c_str(), &server_address.sin_addr) <= 0) {
    wxLogMessage("-invalid address: address not supported\n");
    return;
  }
  std::cout << "\n-server address prepared\n";
  struct sockaddr_in heartbeat_address;
  heartbeat_address.sin_family = AF_INET;
  heartbeat_address.sin_port = htons(G_HEARTBEAT_PORT);
  if (inet_pton(AF_INET, G_IP_ADDRESS.c_str(), &heartbeat_address.sin_addr) <= 0) {
    wxLogMessage("-invalid heartbeat address: address not supported\n");
    return;
  }
  std::cout << "\n-heartbeat address prepared\n";

  // CONNECTING CLIENT TO SERVER
  // ---------------------------
  if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
    wxLogMessage("-failure to connect: to server");
    return;
  }
  std::cout << "\n-client connected\n";
  if (connect(heartbeat_socket, (struct sockaddr*)&heartbeat_address, sizeof(heartbeat_address)) < 0) {
    wxLogMessage("-failure to connect: to heartbeat");
    return;
  }
  std::cout << "\n-heartbeat connected\n";

  start_threads();

} // create-socket-connection


/* DISCONNECT
 **************/
void MainFrame::disconnect_connection (wxCommandEvent& event) {
  chat_screen->Append("-Disconnecting...");
  std::string close_connect = "*" + MainFrame::username + "|!close";
  send(client_socket, close_connect.c_str(), close_connect.size(), 0);
}



/* MAGIC SPELL
 ***************/
IMPLEMENT_APP(ClientApp);



/* CLIENT APP
 **************/
bool ClientApp::OnInit() {
  system("clear");
  std::cout << "   *****************\n"
               "   *   CHUI-FACE   *\n"
               "   *****************\n"
               "      Chat client\n"
               " ---------------------\n"<< std::endl;
  std::cout << "Username: ";
  std::string init_username;
  std::getline(std::cin, init_username);

  MainFrame* main_window = new MainFrame(init_username);
  main_window->Show();
  return true;
};



/* MAINFRAME
 *************/
MainFrame::MainFrame(std::string& init_username) : wxFrame(NULL, wxID_ANY, "CHUI-FACE") {

  username = init_username;

  // MENU BAR
  wxMenuBar* menu_bar = new wxMenuBar();
  wxMenu* file_menu = new wxMenu();

  file_menu->Append(USERNAME,
      wxT("&Edit Username\tAlt-u"),
      _T("Updates username"));
  file_menu->Append(CONNECT,
      wxT("&Connect\tAlt-c"),
      _T("Connects to server"));
  file_menu->Append(DISCONNECT,
      wxT("&Disconnect\tAlt-d"),
      _T("Disconnects from server"));
  file_menu->Append(FILE_QUIT,
      wxT("E&xit\tAlt-x"),
      _T("Quits the program"));
  menu_bar->Append(file_menu, _T("&File"));
  SetMenuBar(menu_bar);
  wxPanel* panel = new wxPanel(this);

  // Chat Screen listbox
  chat_screen = new wxListBox(panel, LISTBOX,
                              wxDefaultPosition,
                              wxSize(820,450));
   
  
  user_input = new wxTextCtrl(panel, wxID_ANY, "message",
                              wxDefaultPosition,
                              wxSize(700,35), wxTE_PROCESS_ENTER);
  user_input->Bind(wxEVT_TEXT_ENTER, &MainFrame::submit_chat, this);
  wxButton* submit = new wxButton(panel, wxID_ANY, "Submit",
                                  wxDefaultPosition,
                                  wxSize(60, 35));
  submit->Bind(wxEVT_BUTTON, &MainFrame::submit_chat, this);


  wxGridBagSizer* main_box = new wxGridBagSizer(0,0);
  main_box->Add(chat_screen,
                wxGBPosition(1,1),
                wxGBSpan(8,11),
                wxALIGN_CENTER);
  main_box->Add(user_input,
                wxGBPosition(10,1),
                wxGBSpan(1,9));
  main_box->Add(submit,
                wxGBPosition(10,10),
                wxGBSpan(1,1));

  panel->SetSizer(main_box);
  this->SetSize(840, 600);
}



/* LOGIN USERNAME
 ******************
LoginUsername::LoginUsername(wxFrame* parent) : wxFrame(parent, wxID_ANY, "Login") {
    wxPanel* panel = new wxPanel(this);
    user_input = new wxTextCtrl(panel, wxID_ANY, "username",
                                wxDefaultPosition,
                                wxDefaultSize, wxTE_PROCESS_ENTER);
    user_input->Bind(wxEVT_TEXT_ENTER, &MainFrame::get_username, this);
    wxButton* login_button = new wxButton(panel, wxID_ANY, "Submit",
                                wxDefaultPosition,
                                wxSize(60, 35));
    login_button->Bind(wxEVT_BUTTON, &MainFrame::get_username, this);

    wxGridSizer* box = new wxGridSizer(1);
    box->Add(user_input);
    box->Add(login_button);

    panel->SetSizer(box);
    this->SetSize(150,200);
}*/
  



/* SUBMIT CHAT
 ***************/
void MainFrame::submit_chat(wxCommandEvent& event) {
  new_line = user_input->GetValue();
  this->user_input->Clear();
  if (new_line == "") {
    new_line = " ";
  }
  new_line = std::string("*" + username + "|" + new_line);
  send(client_socket, new_line.c_str(), new_line.size(), 0);
  new_line = "";
}



/* CLOSE PROGRAM
*****************/
void MainFrame::close_window(wxCommandEvent& evt) {
  chat_screen->Append("-Disconnecting...");
  std::string close_connect = "*" + MainFrame::username + "|!close";
  send(client_socket, close_connect.c_str(), close_connect.size(), 0);
  close(heartbeat_socket);
  // Client needs to sleep here, in order to ensure that Server-Side
  // has time to properly close client's sockets and remove the
  // client from client-collection.
  std::this_thread::sleep_for(std::chrono::seconds(2));
  Close(true);
}
