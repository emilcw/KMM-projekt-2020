// kom.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "CommServer.h"
#include <random>
#include <string.h>
#include <unistd.h>

using namespace std;

int main()
{
	CommServer tcp_server = CommServer();
	while(true){
        // Waits for a client to successfully connect
		int response;
		do {
			response = tcp_server.StartServer();
		} while (response != 0);

		cout << "Connected to client" << endl;
		cout << "Sending messages" << endl;

		double lower_bound = -0.5;
		double upper_bound = 0.6;
		std::uniform_real_distribution<double> unif(lower_bound,upper_bound);
		std::default_random_engine re;
		double a_random_double = 5;
		int messagenumber = 0;
		while (true)
		{
			usleep(250000);
			a_random_double += unif(re);
			//cout << a_random_double << endl;
			messagenumber++;
			cout << "Sending message " << messagenumber << endl;

			if(tcp_server.Send("wheel1," + to_string(a_random_double) + ";")) break;
			
			//int bytes_sent = tcp_server.Send("wheel1," + to_string(a_random_double) + ";");
			//if(bytes_sent == -1) break;

			//tcp_server.Send("mode," + to_string(a_random_double) + ";");
			auto reply = tcp_server.Recive();
			//vector<vector<string>*>* reply = new vector<vector<string>*> {new vector<string> { "hej"} };
			if (reply->size() != 0)
			{
				cout << "Recieved data:"; 
				for(auto command : *reply)
				{
					for(auto arg : *command)
					{
						cout << arg << ' ';
					}
				}
				cout << endl;
				for(auto command : *reply)
				{
					string backData = "";
					for(auto arg : *command)
					{
						backData += arg + ",";
					}
					cout << "Sending data: " << backData << endl;

					if(tcp_server.Send(backData + ";")) break;
					delete command;
				}
			}
			delete reply;
		}
	}
	return 0;
}


  
  












