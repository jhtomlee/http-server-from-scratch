#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/socket.h>
#include <pthread.h>
#include <deque>
#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include "http.h"
#include "libraries/nlohmann/json.hpp"
using json = nlohmann::json;
using namespace std;

/* Server Class */
class Server
{
public:
  int port;
  string address;
  string addressNPort;
  bool running;
  Server(int p, string adrs)
  {
    address = adrs;
    port = p;
    running = true;
    addressNPort = adrs + ":" + to_string(p);
  }
};

/* Global Variables */
deque<Server> servers;
pthread_mutex_t server_lock;
pthread_mutex_t check_lock;
bool debug_mode = false;

void *check_server(void *arg);
int parse_servers(char *file_input);
string parse_html(string filename);
void send_response(int sockfd, char *buffer, string str_response);

void printdq(deque<Server> servers)
{
  // deque<Server> :: iterator it;
  for (int i = 0; i < servers.size(); i++)
  {
    cout << servers[i].port << endl;
  }
}

/**
 * Main
 */
int main(int argc, char *argv[])
{
  int port = 5000; // default
  int option;
  while ((option = getopt(argc, argv, "p:av")) != -1)
  {
    switch (option)
    {
    case 'p':
      port = atoi(optarg); // set port
      break;
    case 'v':
      debug_mode = true; // debug mode
      break;
    case '?': //used for some unknown options
      printf("Unknown option: %c\nEnding program ...\n", optopt);
      exit(1);
    }
  }

  // Check if address file is given
  if (argv[optind] == NULL)
  {
    fprintf(stderr, "Address file missing\n");
  }

  // Add servers from given address file and get count
  int server_count = parse_servers(argv[optind]);

  // Create socket
  int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
  if (listen_fd < 0)
  {
    fprintf(stderr, "Cannot open socket (%s)\n", strerror(errno));
    exit(1);
  }
  // Optional step to avoid "address already in use"
  int opt = 1;
  setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

  // Bind to port
  struct sockaddr_in servaddr;
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htons(INADDR_ANY);
  servaddr.sin_port = htons(port);
  bind(listen_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));

  // Listen
  listen(listen_fd, 10);
  if (debug_mode)
    printf("Server listening to port %d\n", port);

  while (true)
  {
    struct sockaddr_in clientaddr;
    socklen_t clientaddrlen = sizeof(clientaddr);
    int comm_fd = accept(listen_fd, (struct sockaddr *)&clientaddr, &clientaddrlen);
    if (debug_mode)
    {
      printf("[%d] New connection from (%s, %d)\n", comm_fd, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
      cout << "New connection port: " << clientaddr.sin_port << endl;
    }

    int index = 0;
    pthread_mutex_lock(&server_lock);
    while (!servers.front().running)
    {
      Server front_serv = servers.front();
      servers.pop_front();
      servers.push_back(front_serv);
      index++;
      if (index == servers.size())
        break;
    }
    pthread_mutex_unlock(&server_lock);
    if (index != servers.size())
    {
      Server front_server = servers.front();
      string address = front_server.address + ":" + to_string(front_server.port);

      pthread_mutex_lock(&server_lock);
      servers.pop_front();
      servers.push_back(front_server);
      pthread_mutex_unlock(&server_lock);
      if (debug_mode)
        printdq(servers);
      if (debug_mode)
        cout << "Redirect server to: " << address << endl;

      string str_response = "HTTP/1.1 301 Moved Permanently\r\nLocation:http://" + address + "\r\n\r\n";
      cout << str_response << endl;

      char *serv_response = (char *)malloc(10000);
      strcpy(serv_response, str_response.c_str());
      send_response(comm_fd, serv_response, str_response);
      free(serv_response);
    }
    else
    {
      if (debug_mode)
        printf("No running server\n");
      string str_response = "HTTP/1.1 404 OK\r\nContent-type: text/html\r\n\r\n";
      str_response += parse_html("./html/503.html");
      char *serv_response = (char *)malloc(10000);
      strcpy(serv_response, str_response.c_str());
      send_response(comm_fd, serv_response, str_response);
      free(serv_response);
    }
    close(comm_fd);
  }
  close(listen_fd);
  return 0;
}

/* Parse servers from given text file of servers */
int parse_servers(char *file_input)
{
  string filename = string(file_input);
  ifstream file(filename);
  int server_count = 0;
  string line;

  while (getline(file, line))
  {
    int port = 0;
    char address[128];
    char copy[256];

    strcpy(copy, line.c_str());
    char *token = strtok(copy, ":"); // parse address
    strcpy(address, token);
    token = strtok(NULL, ":"); // parse port
    port = atoi(token);

    // Add server to global deque
    Server server(port, address);
    servers.push_back(server);
    server_count++;
  }

  return server_count;
}

/* Parse HTML file into string*/
string parse_html(string filename)
{
  ifstream file(filename);
  string line;
  string content;

  while (getline(file, line))
  {
    content += line;
  }
  file.close();
  return content;
}

/* Send response to client*/
void send_response(int sockfd, char *buffer, string str_response)
{
  int index = 0;
  int response_len = strlen(str_response.c_str()) + 1;

  while (index < response_len)
  {
    int n = write(sockfd, &buffer[index], response_len - index);
    index += n;
  }
}
