#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include "libraries/nlohmann/json.hpp"
#include "http.h"

using namespace std;
using json = nlohmann::json;

void parse_http_request(string request, httpRequest *parsed_request)
{
  // prepare tokennize
  stringstream tokenize(request);
  string line;
  int i = 0;
  string jsonBody = "";
  bool jsonDetected = false;

  // for each line in the http request
  while (getline(tokenize, line, '\n'))
  {
    if (line.rfind("GET", 0) == 0)
    {
      parsed_request->method = "GET";
      parse_uri_and_params(parsed_request, line);
    }
    else if (line.rfind("POST", 0) == 0)
    {
      parsed_request->method = "POST";
      parse_uri_and_params(parsed_request, line);
    }
    else if (line.rfind("HEAD", 0) == 0)
    {
      parsed_request->method = "HEAD";
      parse_uri_and_params(parsed_request, line);
    }
    else if (line.rfind("Content-Type", 0) == 0)
    {
      string type = line.substr(13);
      parsed_request->contentType = type;
    }
    else if (line.rfind("Cookie", 0) == 0)
    {
      string cookie = line.substr(8);
      parsed_request->cookie = cookie;
    }
    else if (!jsonDetected && (line.compare("{") == 0 || line.find('{') != string::npos))
    {
      jsonDetected = true;
      jsonBody += line;
    }
    else if (jsonDetected)
    {
      jsonBody += line;
    }
  }

  //parse jsonBody
  json j;
  if (jsonBody.size() > 1)
  {
    j = json::parse(jsonBody);
  }
  else
  {
    j = {};
  }
  parsed_request->bodyJson = j;
}

void parse_uri_and_params(httpRequest *req, string line)
{
  stringstream tokenize(line);
  string token;
  vector<string> tokens;

  while (getline(tokenize, line, ' '))
  {
    tokens.push_back(line);
  }

  //check for parameters
  string addr_str = tokens[1];
  string params = "";
  string uri = addr_str;
  if (addr_str.find('?') != std::string::npos)
  {
    stringstream tokenize(addr_str);
    string token;
    vector<string> tokens;
    while (getline(tokenize, token, '?'))
    {
      tokens.push_back(token);
    }
    string new_addr = tokens[0];
    params = tokens[1];
    uri = new_addr;
  }
  req->uri = uri;
  req->params = params;
}

void response_common_headers(httpResponse *response, int status)
{
  if (status == 200)
  {
    response->status = "HTTP/1.1 200 OK\n";
    response->contentType = "Content-Type: application/json\n";
    response->allow = "Allow: GET, POST, OPTIONS\n";
    response->allowOrigin = "Access-Control-Allow-Origin: *\n";
    response->allowMethods = "Access-Control-Allow-Methods: GET, POST, OPTIONS\n";
    response->allowHeaders = "Access-Control-Allow-Headers: Content-Type\n";
  }
  else if (status == 404)
  {
    response->status = "HTTP/1.1 404 Not Found\n";
  }
  else if (status == 400)
  {
    response->status = "HTTP/1.1 400 Bad Request\n";
  }
  else if (status == 500)
  {
    response->status = "HTTP/1.1 500 Internal Server Error\n";
  }
}

string stringfy_http_response(httpResponse *response, bool isHeadRequest)
{
  string contentLength = "Content-Length: ";
  contentLength += to_string(response->body.dump().size());
  contentLength += "\n";

  string response_str = "";
  response_str += response->status;
  response_str += response->contentType;
  response_str += contentLength;
  response_str += response->allow;
  response_str += response->allowOrigin;
  response_str += response->allowMethods;
  response_str += response->allowHeaders;
  response_str += "\n";
  if (!isHeadRequest)
  {
    response_str += response->body.dump();
  }
  return response_str;
}