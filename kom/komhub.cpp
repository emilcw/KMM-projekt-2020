#include "komhub.h"

int STY_port {}; //Styrenhet
int usb1_port {}; //Sensorenhet
using namespace std;
/*This function inits the USB0 port*/
int STY_port_init() {
  int serial_port;
  serial_port = serialOpen("/dev/ttyUSB0", 9600);
  return serial_port;
}


/*This function inits the USB1 port*/
int usb1_port_init() {
   int serial_port;
   serial_port =serialOpen("/dev/ttyUSB1", 9600);
   return serial_port;
 }


/*This functions tells us when there is 
 * data to be read.*/
bool dataOnPort(int usb_port) {
  return serialDataAvail(usb_port);
}


/*This function translates the string from the PC
 * and returns the corresponding enum*/
Commando getCorrespondingEnum(string command) {
  
  if(command.compare("Forward") == 0) {
    return FORWARD;
  }
  else if(command.compare("LeftForward") == 0) {
    return FORWARD_LEFT;
  }
  else if(command.compare("RightForward") == 0) {
    return FORWARD_RIGHT;
  }
  else if(command.compare("TurnLeft") == 0) {
    return LEFT;
  }
  else if(command.compare("TurnRight") == 0) {
    return RIGHT;
  }
  else if(command.compare("CALIBRATE") == 0) {
    return CALIBRATE;
  }
  else if(command.compare("Backward") == 0) {
    return BACK;
}
  else if(command.compare("Stay") == 0){
    return STOP;
}
  else if(command.compare("MoveAuto") == 0){
    return MOVE_AUTO;
}
  else if(command.compare("SetSpeed") == 0){
    return SET_SPEED;
}
  else {
    cout << "Command: " << command << " is not a valid command!" << endl;
    return NO_COMMAND;
  }
}
  
 

/* Simple send-function, likely to be upgraded */
void send(int port, char message) {
  //cout << "Sending message: " << int(message) << ", on port: " << port << endl;
  serialPutchar(port, message);
}


/*This function inits the wiringPiSetup library and opens
 * the USB-ports and makes them ready to communicate with UART.
 * If something is not plugged in to a port when running this it will fail
 * which is fine.*/
void initKomHub(){
  if(wiringPiSetup() == -1){
    cout << "Failed wiringPiSetup";
  }

  /*Initiate/activate ports for communication between modules
  in our case /dev/ttyUSB0 and /dev/ttyUSB1 and /dev/ttyUSB2 */
  if((STY_port = STY_port_init()) < 0 ) {
    cout << "Failed usb0 init" << endl;
  }
  if( (usb1_port = usb1_port_init()) < 0 ) {
    cout << "Failed usb1 init" << endl;
  }
}
/*This function will send message to "sensorenhet". 
"Sensorenhet"'s expected commands to receive can be found in sensor_enhet.h
At this instance I assume the only command that will be sending additional packets to 
"Sensorenhet" is when you want to compare angles (calibrate gyro) */
void sendToSen(Commando command, int argument) {
  //cout << "Sending command to sen: " << command << ", with arg: " << argument << endl;
  int message;
  int high_bit;
  switch(command) {
    case COMPARE_ANGLES:
      message = 0x52;
      send(usb1_port, message);
      if (argument > 255) {
	high_bit = 1;
      } else high_bit=0;
      //high_bit = argument > 255 ? 0x01 : 0x00;
      send(usb1_port, high_bit);
      send(usb1_port, argument);
      break;
    case REQUEST_MODE:
		 message = 0x60;
		send(usb1_port, message);
		 break;
	case REQUEST_ANGLE:
	    cout << "Requesting angle" << endl;
		message = 0x20;
		send(usb1_port, message);
		break;
    default:
      break;
  }
}

/* Function that first checks whether or not there is something on port and if nothing is on port return enum NOT_ON_PORT
	if something is on port then check what it's and return the corresponding enum to the given command. Main-function meant to use 
	this function to check states and need to decide what to do with the given enum */ 
pair<Comfromsen, int> receiveFromSen() {
  if(!dataOnPort(usb1_port)) {
    return make_pair(NOT_ON_PORT, 0);
  }
  else {
    int message;
    message = serialGetchar(usb1_port);
    switch(message) {
      case MANUAL_STATE: {
		return  make_pair(MANUAL, 0);
	      }
      case AUTO_STATE: {
		return  make_pair(AUTO, 0);
	      }
      case MAPPING_DISABLED: {
		return  make_pair(DISABLE_MAPPING, 0);
	      }
      case MAPPING_ENABLED: {
		return  make_pair(ENABLE_MAPPING, 0);
	      }
      case GYRO_DATA: {
		int arg1; 
		int arg2; 
		arg1 = 256*serialGetchar(usb1_port);
		arg2 = serialGetchar(usb1_port); 
		int arg = int(arg1) + int(arg2);
		return  make_pair(GYRO_ANGLE, arg);
	      }
      default: {
	cout << "No applicable command was given" << endl;
	return make_pair(ERROR, 0);
      }
    }
  }
}

/* Function that wait for data on port and then returns the speed with direction */
pair<double, double> receiveFromStyr() {
    int message;
    int arg1;
    int arg2;
    int dir1;
    int dir2;
    double speed1;
    double speed2;
    //cout << "Getting message" << endl;
    message = serialGetchar(STY_port); /* Receive command*/
    //cout << "Got message, getting arg1" << endl;
    arg1 = (serialGetchar(STY_port)); /* Receive first packet*/
    //cout << "Got arg1" << endl;
    dir1 = arg1 & 0x80; /* Dir as 0 or 1. */
    speed1 = (arg1 & 0x7F) / 10; /* Remove direction bit and divide by 10*/
    if(dir1 == 0){
	  speed1 = -speed1;
    }
    arg2 = (serialGetchar(STY_port));  /* Receive second packet */
    dir2 = arg2 & 0x80; /* Dir as 0 or 1*/
    speed2 = (arg2 & 0x7F) / 10; /* Remove direction bit and divide by 10*/
    if(dir2 == 0){
	  speed2 = -speed2;
    }
    return make_pair(speed1, speed2);
}

/*This function takes in the command as a string, e.g. "Forward.
 * It will then translate into the corresponding enum, e.g FORWARD
 * and then output it on the USB0 port to "Styrenhet".*/
void sendToStyr(vector<string>* command){
      int message;
      int arg1, arg2;
      if (command == nullptr) return;
      Commando commando = getCorrespondingEnum((*command)[0]);
      //cout << "got commando: " << commando << endl;
      if (commando == NO_COMMAND) return;
      if (commando == SET_SPEED) {
	arg1 = stoi((*command)[1]);
      }	
      if (commando == MOVE_AUTO) {
	int angle = stoi((*command)[1]);
	arg1 = angle/256;
	arg2 = angle % 256;
      }
      
      switch(commando) {
	case FORWARD:
	  message = 0x10;
	  send(STY_port,message);
	  break;
	case FORWARD_LEFT:
	  message = 0x20;
	  send(STY_port,message);
	  break;
	case FORWARD_RIGHT:
	  message = 0x30;
	  send(STY_port,message);
	  break;
	case LEFT:
	  message = 0x40;
	  send(STY_port,message);
	  break;
	case RIGHT:
	  message = 0x50;
	  send(STY_port,message);
	  break;
	case STOP:
	  message = 0x00;
	  send(STY_port,message);
	  break;
	case BACK:
	  message = 0x60;
	  send(STY_port,message);
	  break;
	case CALIBRATE:
	  message = 0x70;
	  send(STY_port,message);
	  break;
	case MOVE_AUTO:
	  message = 0x82;
	  //cout << " arg1: " << arg1 << ", arg2: " << arg2 << endl;
	  send(STY_port, message);
	  send(STY_port,arg1);
	  send(STY_port,arg2);
	  break;
	case SET_SPEED:
	  message = 0x91;           
	  send(STY_port, message);
	  send(STY_port,arg1);
	  break;
	
	
	default:
	  cout << "No applicable command was given" << endl;
	  break;
    }
}


