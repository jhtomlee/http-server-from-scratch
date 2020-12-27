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

/* GET /loginstatus */
int get_login_status(int comm_fd, string sessionId);

/* POST /signin */
int post_signin(int comm_fd, json body, string sessionId);

/* POST /signout */
int post_signout(int comm_fd, string sessionId);