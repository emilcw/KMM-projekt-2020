/*
 * Implements the tcp server communication protocoll
 */

#include "CommServer.h"

#include <cstring>    // sizeof()
#include <iostream>
#include <string>
#include <fcntl.h>

// headers for socket(), getaddrinfo() and friends
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <unistd.h>    // close()

// Mostly copied from https://riptutorial.com/cplusplus/example/23999/hello-tcp-server
int CommServer::StartServer()
{
	if (newFD != 0) close(newFD);
	if (sockFD != 0) close(sockFD);
	if (res != nullptr) freeaddrinfo(res);

	auto &portNum = PORTNO;
	const unsigned int backLog = 8;  // number of connections allowed on the incoming queue

	memset(&hints, 0, sizeof(hints));

	// for more explanation, man socket
	hints.ai_family   = AF_UNSPEC;    // don't specify which IP version to use yet
	hints.ai_socktype = SOCK_STREAM;  // SOCK_STREAM refers to TCP, SOCK_DGRAM will be?
	hints.ai_flags    = AI_PASSIVE;


	// man getaddrinfo
	int gAddRes = getaddrinfo(NULL, portNum, &hints, &res);
	if (gAddRes != 0) {
		std::cerr << gai_strerror(gAddRes) << "\n";
		return -2;
	}

	//std::cout << "Detecting addresses" << std::endl;

	unsigned int numOfAddr = 0;
	char ipStr[INET6_ADDRSTRLEN];    // ipv6 length makes sure both ipv4/6 addresses can be stored in this variable


	// Now since getaddrinfo() has given us a list of addresses
	// we're going to iterate over them and ask user to choose one
	// address for program to bind to
	for (p = res; p != NULL; p = p->ai_next) {
	void *addr;
	std::string ipVer;

	// if address is ipv4 address
	if (p->ai_family == AF_INET) {
	  ipVer             = "IPv4";
	  sockaddr_in *ipv4 = reinterpret_cast<sockaddr_in *>(p->ai_addr);
	  addr              = &(ipv4->sin_addr);
	  ++numOfAddr;
	}

	// if address is ipv6 address
	else {
		ipVer              = "IPv6";
		sockaddr_in6 *ipv6 = reinterpret_cast<sockaddr_in6 *>(p->ai_addr);
		addr               = &(ipv6->sin6_addr);
		++numOfAddr;
	}

	// convert IPv4 and IPv6 addresses from binary to text form
	inet_ntop(p->ai_family, addr, ipStr, sizeof(ipStr));
	//std::cout << "(" << numOfAddr << ") " << ipVer << " : " << ipStr << std::endl;
	}

	// if no addresses found :(
	if (!numOfAddr) {
		std::cerr << "Found no host address to use\n";
		return -3;
	}

	p = res;

	// let's create a new socket, socketFD is returned as descriptor
	// man socket for more information
	// these calls usually return -1 as result of some error
	sockFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
	if (sockFD == -1) {
		std::cerr << "Error while creating socket\n";
		freeaddrinfo(res);
		return -4;
	}

	// Let's bind address to our socket we've just created
	int bindR = bind(sockFD, p->ai_addr, p->ai_addrlen);
	if (bindR == -1) {
		std::cerr << "Error while binding socket\n";

		// if some error occurs, make sure to close socket and free resources
		close(sockFD);
		freeaddrinfo(res);
		return -5;
	}

	cout << "Listening for client" << endl;
	// finally start listening for connections on our socket
	int listenR = listen(sockFD, backLog);
	if (listenR == -1) {
		std::cerr << "Error while Listening on socket\n";

		// if some error occurs, make sure to close socket and free resources
		close(sockFD);
		freeaddrinfo(res);
		return -6;
	}

	// accept call will give us a new socket descriptor
	newFD = accept(sockFD, (sockaddr *) &client_addr, &client_addr_size);
	if (newFD == -1) {
		std::cerr << "Error while Accepting on socket\n";
	}

	cout << "Client found. Accepting client." << endl;
    hasConnection = true;
	fcntl(newFD, F_SETFL, O_NONBLOCK);

	return 0;
}

// Mostly copied from https://riptutorial.com/cplusplus/example/24000/hello-tcp-client
bool CommServer::Send(string message)
{
    if (hasConnection) {
	    //cout << "Sending message: " << message << endl;
	    // send call sends the data you specify as second param and it's length as
	    // 3rd param, also returns how many bytes were actually sent
	    auto bytes_sent = send(newFD, message.data(), message.length(), 0);
        hasConnection = bytes_sent != -1; 
        if (!hasConnection) cout << "\033[31mConnection was just lost with message: " << message << "\033[0m" << endl;
    } else {
        cout << "\033[31mConnection was lost! Cannot send message: " << message << "\033[0m" << endl;
    }
	return hasConnection;
}

vector<vector<string>*>* CommServer::Recive()
{
	vector<vector<string>*>* commandsWithArguments = new vector<vector<string>*>();
    if (hasConnection) {    
        char padding = '~';
	    std::string reply(PACKAGE_SIZE, padding);

	    // recv() call tries to get the response from server
	    // BUT there's a catch here, the response might take multiple calls
	    // to recv() before it is completely received
	    auto bytes_recv = recv(newFD, &reply.front(), reply.size(), 0);
		//hasConnection = bytes_recv != -1;
		//if (!hasConnection) cout << "Connection was just lost while recieving data" << endl;
	    // Split strings https://www.fluentcpp.com/2017/04/21/how-to-split-a-string-in-c/
	    vector<string> commands;
	    splitString(reply, commands, ';');

	    for(auto command : commands)
	    {
		    if (command.back() != padding)
		    {
			    vector<string>* splitCommand = new vector<string>();
			    splitString(command, *splitCommand, ',');
			    commandsWithArguments->push_back(splitCommand);
		    }
	    }
    }
	return commandsWithArguments;

}

// Split string http://www.martinbroadhurst.com/how-to-split-a-string-in-c.html
void CommServer::splitString(const std::string& str, vector<string>& cont,
              char delim)
{
    std::size_t current, previous = 0;
    current = str.find(delim);
    while (current != std::string::npos) {
        cont.push_back(str.substr(previous, current - previous));
        previous = current + 1;
        current = str.find(delim, previous);
    }
    cont.push_back(str.substr(previous, current - previous));
}
