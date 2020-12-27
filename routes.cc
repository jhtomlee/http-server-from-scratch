#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <sstream>
#include <memory>
#include "routes.h"
#include "http.h"
#include "libraries/nlohmann/json.hpp"
using json = nlohmann::json;
using namespace std;

/* Globals */
extern bool debug_mode;
extern int sid;
extern map<string, string> sidUserMapping;

/* GET /index.html */
int get_index_html(int comm_fd, string sessionId)
{
  char *temp = (char *)malloc(2000 * sizeof(char));
  strcpy(temp, "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: %d\n\n");
  FILE *file = fopen("console-webui/index.html", "r");
  char line[100];
  int size = 0;
  while (fgets(line, sizeof(line), file))
  {
    strcat(temp, line);
    size += strlen(line);
  }
  char *response = (char *)malloc(2000 * sizeof(char));
  int n = sprintf(response, temp, size);
  if (debug_mode)
    printf("%s\n", response);
  send(comm_fd, response, n, 0);
  free(temp);
  free(response);
  fclose(file);
  return 0;
}

/* GET /main.js */
int get_main_js(int comm_fd)
{
  FILE *file = fopen("console-webui/main.js", "r");
  char line[10000];
  int i = 0;
  long int fileSize = 0;
  while (fgets(line, sizeof(line), file))
  {
    // printf("%d %ld\n", i, strlen(line));
    i++;
    fileSize += strlen(line);
  }
  fileSize += 100;
  // malloc with the required size
  char *response = (char *)malloc(fileSize * sizeof(char));
  strcpy(response, "HTTP/1.1 200 OK\nContent-Type: text/javascript\n\n");
  file = fopen("console-webui/main.js", "r");
  while (fgets(line, sizeof(line), file))
  {
    strcat(response, line);
  }
  // send to client
  send(comm_fd, response, strlen(response), 0);
  free(response);
  fclose(file);
  return 0;
}

/* GET /static/logo.js */
int get_logo_js(int comm_fd)
{
  char *response = (char *)malloc(1000 * sizeof(char));
  strcpy(response, "HTTP/1.1 200 OK\nContent-Type: image/png\n\n");
  send(comm_fd, response, strlen(response), 0);

  //read jpeg file
  FILE *file = fopen("console-webui/static/logo.png", "rb");
  fseek(file, 0, SEEK_END);
  int fileLength = ftell(file);
  rewind(file);
  char *sendbuf = (char *)malloc(sizeof(char) * fileLength);
  size_t result = fread(sendbuf, 1, fileLength, file);
  // send to client
  send(comm_fd, sendbuf, result, 0);
  free(sendbuf);
  fclose(file);

  free(response);
  return 0;
}

/* GET /favicon.ico */
int get_favicon(int comm_fd){
  char *response = (char *)malloc(1000 * sizeof(char));
  strcpy(response, "HTTP/1.1 200 OK\nContent-Type: image/x-icon\n\n");
  send(comm_fd, response, strlen(response), 0);

  //read jpeg file
  FILE *file = fopen("console-webui/static/favicon.ico", "rb");
  fseek(file, 0, SEEK_END);
  int fileLength = ftell(file);
  rewind(file);
  char *sendbuf = (char *)malloc(sizeof(char) * fileLength);
  size_t result = fread(sendbuf, 1, fileLength, file);
  // send to client
  send(comm_fd, sendbuf, result, 0);
  free(sendbuf);
  fclose(file);
  free(response);
  return 0;

}

/* GET /getuser */
int get_user(int comm_fd, string parameter, string sessionId)
{

  return 0;
}

/* POST /createuser */
int post_create_user(int comm_fd, json body)
{

  return 0;
}
