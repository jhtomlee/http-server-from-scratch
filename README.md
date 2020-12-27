# HTTP Web Server in C++

(Work in progress)

A multithreaded HTTP web server implemented using TCP socket programming with features such as load balancing, session management, and support for GET/POST requests.

## Components / Features
- [X] HTTP request parser
- [X] Web server that implements the basic functionalities of the [HTTP protocol](https://www.ietf.org/rfc/rfc2616.txt)
- [X] HTTP routes for GET /loginstatus, POST /signin, and POST /signout 
- [X] Cookie handling for session management
- [X] Console Web-UI to test the HTTP server and its features
- [ ] Load balancer node that receives initial request from clients and redirects them to an available web server

## Tech Stack
C++, TCP socket programming, POSIX threads (pthreads), HTTP protocol, HTML

## About
* Created by [jhtomlee](https://github.com/jhtomlee)
