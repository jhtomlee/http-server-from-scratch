#include <iostream>
#include "libraries/nlohmann/json.hpp"
using json = nlohmann::json;
using namespace std;

typedef struct httpRequest
{
  string method;      // GET, PUT, or HEAD
  string uri;         // e.g. /index.html
  string params;      // uri params if any
  string cookie;      // sessionId cookie if any
  string contentType; // Content-Type: application/json
  json bodyJson;      // Content of the body
} httpRequest;

typedef struct httpResponse
{
  string status;
  string contentType;
  string allow;
  string allowOrigin;
  string allowMethods;
  string allowHeaders;
  json body;
} httpResponse;

/**
 *  HTTP Request related functions
 * /

/* Parse Http Request*/
void parse_http_request(string request, httpRequest *request_parsed);
// helper: Parse GET request parameters
json parse_params(string line);
// helper: Parse uri and params
void parse_uri_and_params(httpRequest *req, string line);

/**
 *  HTTP Response related functions
 * /

/* Common respose headers */
void response_common_headers(httpResponse *response, int status);
/* Stringfy Http Response*/
string stringfy_http_response(httpResponse *response, bool isHeadRequest);