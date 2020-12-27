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
  // Get sessionId cookie
  if (sessionId.size() ==0){
		printf("[%d] No cookie detected\n", comm_fd);
		sessionId = to_string(sid);
		sid++; 
	} else{
		printf("[%d] Cookie detected: %s\n", comm_fd, sessionId.c_str());
	}

  // Create http headers
  char *temp = (char *)malloc(2000 * sizeof(char));
  strcpy(temp, "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: %d\nSet-Cookie: sid=%s\n\n");

  // Concat index.html files
  FILE *file = fopen("console-webui/index.html", "r");
  char line[100];
  int size = 0;
  while (fgets(line, sizeof(line), file))
  {
    strcat(temp, line);
    size += strlen(line);
  }

  // Create response
  char *response = (char *)malloc(2000 * sizeof(char));
  int n = sprintf(response, temp, size, sessionId.c_str());
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

/* GET /loginstatus */
int get_login_status(int comm_fd, string sessionId){

  // check session
  string username = "";
  bool isLoggedIn = false;
  if (sidUserMapping.find(sessionId) == sidUserMapping.end() || sidUserMapping[sessionId].compare("") == 0){
    isLoggedIn = false;
  } else {
    isLoggedIn = true;
    username = sidUserMapping[sessionId];
  }
  // create http response
	httpResponse response;
	response_common_headers(&response, 200);
  json resBody;
  if (isLoggedIn){
    resBody["result"] = "found";
    resBody["user"] = username;
    resBody["msg"] = "Logged in user found";
  } else{
    resBody["result"] = "not found";
    resBody["msg"] = "Logged in user not found";
  }
  response.body = resBody;
	string response_str = stringfy_http_response(&response);

  // send response to client
	send(comm_fd, response_str.c_str(), response_str.size(), 0);
	if (debug_mode)printf("%s\n", response_str.c_str());

	return 0;
}

/* POST /signin */
int post_signin(int comm_fd, json body, string sessionId)
{
  // Get username from http body
  string username = body["user"];

  // map sessionId to user
	sidUserMapping[sessionId] = username;

  // create http response
	httpResponse response;
	response_common_headers(&response, 200);
  json resBody;
	resBody["result"] = "success";
	resBody["user"] = username;
  resBody["msg"] = "User has successfully signed in!";
	response.body = resBody;
	string response_str = stringfy_http_response(&response);

  // send response to client
	send(comm_fd, response_str.c_str(), response_str.size(), 0);
	if (debug_mode)printf("%s\n", response_str.c_str());

	return 0;
}

/* POST /signout */
int post_signout(int comm_fd, string sessionId)
{
  // Get username from sessionId
	string username = sidUserMapping[sessionId];

  // Clear user
  sidUserMapping[sessionId] = "";

  // create http response
	httpResponse response;
	response_common_headers(&response, 200);
  json resBody;
	resBody["result"] = "success";
	resBody["user"] = username;
  resBody["msg"] = "User has successfully signed out!";
	response.body = resBody;
	string response_str = stringfy_http_response(&response);

  // send response to client
	send(comm_fd, response_str.c_str(), response_str.size(), 0);
	if (debug_mode)printf("%s\n", response_str.c_str());
  
	return 0;
}
