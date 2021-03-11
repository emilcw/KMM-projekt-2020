#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <bits/stdc++.h>
#include <iostream>
#include "komhub.h"


using namespace std;

int main(){

initKomHub();
string command;

bool running = true;
vector<string> *vect = new vector<string>;
vect->push_back("AutoLeft");
vect->push_back("90");

while(running){
  sendToStyr(vect);
  running = false;
}  
  
}



