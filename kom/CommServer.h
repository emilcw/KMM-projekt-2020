#ifndef COMMSERVER_H
#define COMMSERVER_H

#include <string>
#include <vector>

#include <algorithm>
#include <iterator>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>  /* Needed for getaddrinfo() and freeaddrinfo() */
#include <unistd.h> /* Needed for close() */

using namespace std;

class CommServer
{
public:
	CommServer() = default;

	~CommServer() = default;
	
	int StartServer();
	
	bool Send(string message);
	
	vector<vector<string>*>* Recive();

    bool gethasConnection() { return hasConnection; }

private:
    const int PACKAGE_SIZE = 65536;
	const char* PORTNO = "1337";
	
	void splitString(const std::string& str, vector<string>& cont, char delim = ' ');

	int sockFD = -1;
	addrinfo hints, *res, *p;    // we need 2 pointers, res to hold and p to iterate over
	// structure large enough to hold client's address
	sockaddr_storage client_addr;
	socklen_t client_addr_size = sizeof(client_addr);
	int newFD = -1;
    bool hasConnection = false;
};

#endif
