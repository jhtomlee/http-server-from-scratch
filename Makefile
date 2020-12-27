TARGETS = httpserver loadbalancer routes.o http.o
CXX =/usr/local/bin/g++-10
all: $(TARGETS)

http.o: http.cc
	$(CXX) -c -o http.o http.cc

routes.o: routes.cc
	$(CXX) -c -o routes.o routes.cc

httpserver: httpserver.cc routes.o http.o 
	$(CXX) $^ -lpthread -o $@ 

loadbalancer: loadbalancer.cc http.o 
	$(CXX) $^ -lpthread -o $@ 

clean:
	rm -f $(TARGETS) *~ *.o
