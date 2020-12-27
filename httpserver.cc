#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <pthread.h>
#include <vector>
#include <cstdlib>
#include <signal.h>
#include <algorithm>
#include <cerrno>
#include <thread>
#include <csignal>
#include "libraries/nlohmann/json.hpp"
#include "http.h"
#include "routes.h"

using json = nlohmann::json;
using namespace std;

/* Global Variables */
bool VB = true;
bool debug_mode = false;            // debug_mode = true will print important info
vector<pthread_t *> threads;        // stores all initiazlied threads
int sid = 0;                        // sessionId
map<string, string> sidUserMapping; // sid:user

/* Handler to exit gracefully when ctrl+c is signaled*/
void signal_ctrl_c_handler(int sig)
{
  for (int i = 0; i < threads.size(); i++)
  {
    // send signal to all threads (both main and worker threads)
    pthread_kill(*(threads.at(i)), SIGUSR1);
  }
}
/* Handler for user defined signal */
void signal_user1_handler(int sig) {}
/* worker thread initializer */
void *worker(void *arg);
/* request handler */
int handle_request(char *command, int comm_fd);

/**
 * Main
 */
int main(int argc, char *argv[])
{
  int port = 5000; //default
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

  //add main thread
  pthread_t mainTID = pthread_self();
  threads.push_back(&mainTID);

  // initialize signals thread
  siginterrupt(SIGINT, true);
  siginterrupt(SIGUSR1, true);
  signal(SIGINT, signal_ctrl_c_handler);
  signal(SIGUSR1, signal_user1_handler);

  // create socket
  int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
  // optional step to avoid "address already in use"
  int opt = 1;
  setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

  // bind to port
  struct sockaddr_in servaddr;
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htons(INADDR_ANY);                    //bind to IP address (any interface)
  servaddr.sin_port = htons(port);                                 //bind to this port
  bind(listen_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)); //binds the port# and IP address to the file descriptor

  // listen
  listen(listen_fd, 10); //10 = queue for pending connections
  if (debug_mode)
    printf("Server listening to port %d\n", port);

  /* ACEEPTING NEW CONNECTIONS */
  struct sockaddr_in clientaddr;
  socklen_t clientaddrlen = sizeof(clientaddr);
  int *comm_fd;
  while (true)
  {
    comm_fd = (int *)malloc(sizeof(int));
    *comm_fd = accept(listen_fd, (struct sockaddr *)&clientaddr, &clientaddrlen);
    if (*comm_fd == -1)
    {
      if (errno == EINTR)
      {
        break;
      }
    }
    if (debug_mode)
    {
      printf("[%d] New connection from (%s, %d)\n", *comm_fd, inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
    }
    // initialize new worker thread
    pthread_t *thread = (pthread_t *)malloc(sizeof(pthread_t));
    threads.push_back(thread);
    pthread_create(thread, NULL, worker, comm_fd);
  }

  /* TERMINATING SERVER - joins terminated threads*/
  for (int i = 1; i < threads.size(); i++)
  {
    if (pthread_join(*(threads.at(i)), NULL) != 0)
    {
      if (debug_mode)
      {
        printf("\nError terminating thread %d\n", i);
      }
      exit(1);
    }
    else
    {
      if (debug_mode)
      {
        printf("Successfully joined terminated thread %d\n", i);
      }
      free(threads.at(i));
    }
  }
  //free latest comm_fd
  free(comm_fd);
  //close listener socket
  close(listen_fd);
  return 0;
}

/* worker thread initializer */
void *worker(void *arg)
{
  // Get fd
  int *comm_fd_pointer = (int *)arg;
  int comm_fd = *(int *)arg;
  fd_set set;
  struct timeval timeout;
  int rv;
  char buffer[1025];
  bool endOfRequest = false;
  size_t bytes_received = 0;

  FD_ZERO(&set);         /* clear the set */
  FD_SET(comm_fd, &set); /* add our file descriptor to the set */
  timeout.tv_sec = 0;
  timeout.tv_usec = 100000;

  char *request = (char *)malloc(sizeof(char) * 20000000);

  while (!endOfRequest)
  {
    rv = select(comm_fd + 1, &set, NULL, NULL, &timeout);
    if (rv == -1)
    {
      printf("ERROR\n"); /* an error accured */
      endOfRequest = true;
      break;
    }
    else if (rv == 0)
    {
      printf("TIMEOUT\n"); /* a timeout occured */
      endOfRequest = true;
      break;
    }
    else
    {
      int n = read(comm_fd, &request[bytes_received], 1025 - 1);
      if (n == 0)
      {
        break;
      }
      fprintf(stderr, "%s", buffer);
      bytes_received += n;
      bzero(buffer, 1025);
    }
  }
  request[bytes_received] = '\0';

  int ret = handle_request(request, comm_fd);
  close(comm_fd);
  free(comm_fd_pointer);
  free(request);
  if (debug_mode)
  {
    printf("[%d] Connection closed\n", comm_fd);
  }
  return NULL;
}

int handle_request(char *request, int comm_fd)
{
  if (debug_mode)
    printf("\n[%d] ----------------- Received request: %s\n", comm_fd, request);
  string request_str = request;
  httpRequest request_parsed;
  parse_http_request(request_str, &request_parsed);

  // HTTP parsing done!
  cout << "[S] HTTP Request parsed! : " << endl;
  cout << "\t Method: " << request_parsed.method << endl;
  cout << "\t URI: " << request_parsed.uri << endl;
  cout << "\t Params: " << request_parsed.params << endl;
  cout << "\t Cookie: " << request_parsed.cookie << endl;
  cout << "\t Content-Type: " << request_parsed.contentType << endl;
  cout << "\t BodyJson: " << request_parsed.bodyJson << endl;

  // get sessionId from Cookies
  string sessionId = "";
  if (request_parsed.cookie.size() != 0)
  {
    stringstream tokenize(request_parsed.cookie);
    string token;
    vector<string> tokens;
    while (getline(tokenize, token, '='))
    {
      tokens.push_back(token);
    }
    sessionId = tokens[1];
    cout << "SESSION ID: " << sessionId << endl;
  }

  // Route request
  if (request_parsed.uri.compare("/index.html") == 0)
  {
    if (get_index_html(comm_fd, sessionId) != 0)
    {
      if (debug_mode)
      {
        printf("[%d] Request %s failed\n", comm_fd, request_parsed.uri.c_str());
      }
    }
  }
  if (request_parsed.uri.compare("/favicon.ico") == 0)
  {
    if (get_favicon(comm_fd) != 0)
    {
      if (debug_mode)
      {
        printf("[%d] Request %s failed\n", comm_fd, request_parsed.uri.c_str());
      }
    }
  }
  else if (request_parsed.uri.compare("/main.js") == 0)
  {
    if (get_main_js(comm_fd) != 0)
    {
      if (debug_mode)
      {
        printf("[%d] Request %s failed\n", comm_fd, request_parsed.uri.c_str());
      }
    }
  }
  else if (request_parsed.uri.compare("/static/logo.png") == 0)
  {
    if (get_logo_js(comm_fd) != 0)
    {
      if (debug_mode)
      {
        printf("[%d] Request %s failed\n", comm_fd, request_parsed.uri.c_str());
      }
    }
  }
  else if (request_parsed.uri.compare("/loginstatus") == 0)
  {
    if (get_login_status(comm_fd, sessionId) != 0)
    {
      if (debug_mode)
      {
        printf("[%d] Request %s failed\n", comm_fd, request_parsed.uri.c_str());
      }
    }
  }
  else if (request_parsed.uri.compare("/signin") == 0)
  {
    if (post_signin(comm_fd, request_parsed.bodyJson, sessionId) != 0)
    {
      if (debug_mode)
      {
        printf("[%d] Request %s failed\n", comm_fd, request_parsed.uri.c_str());
      }
    }
  }
  else if (request_parsed.uri.compare("/signout") == 0)
  {
    if (post_signout(comm_fd, sessionId) != 0)
    {
      if (debug_mode)
      {
        printf("[%d] Request %s failed\n", comm_fd, request_parsed.uri.c_str());
      }
    }
  }
  else
  {
    if (get_index_html(comm_fd, sessionId) != 0)
    {
      if (debug_mode)
      {
        printf("[%d] Request %s failed\n", comm_fd, request_parsed.uri.c_str());
      }
    }
  }

  return 0;
}
