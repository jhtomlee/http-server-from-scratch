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
int get_index_html(int comm_fd, bool isHeadRequest, string sessionId)
{
  return 0;
}

/* GET /app.js */
int get_main_js(int comm_fd, bool isHeadRequest)
{
  return 0;
}

/* GET /getuser */
int get_user(int comm_fd, string parameter, bool isHeadRequest, string sessionId)
{

  return 0;
}

/* POST /createuser */
int post_create_user(int comm_fd, json body)
{

  return 0;
}
