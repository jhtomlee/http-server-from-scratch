#include <string>
#include "libraries/nlohmann/json.hpp"
using namespace std;
using json = nlohmann::json;

/* GET /index.html */
int get_index_html(int comm_fd, bool isHeadRequest, string sessionId);

/* GET /AppJS */
int get_main_js(int comm_fd, bool isHeadRequest);

/* GET /getuser */
int get_user(int comm_fd, string parameter, bool isHeadRequest, string sessionId);

/* POST /createuser */
int post_create_user(int comm_fd, json body);