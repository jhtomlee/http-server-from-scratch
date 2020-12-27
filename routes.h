#include <string>
#include "libraries/nlohmann/json.hpp"
using namespace std;
using json = nlohmann::json;

/* GET /index.html */
int get_index_html(int comm_fd, string sessionId);

/* GET /AppJS */
int get_main_js(int comm_fd);

/* GET /static/logo.js */
int get_logo_js(int comm_fd);

/* GET /favicon.ico */
int get_favicon(int comm_fd);

/* GET /getuser */
int get_user(int comm_fd, string parameter, string sessionId);

/* POST /createuser */
int post_create_user(int comm_fd, json body);