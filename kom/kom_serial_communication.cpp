#include <wiringSerial.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <bits/stdc++.h>
using namespace std;
/*
	UART communication on Raspberry Pi using C (WiringPi Library)
	http://www.electronicwings.com

  Compile does not work on any other machine other then RPI3
  since Wiringpi is developed for rpi and should also already be installed on rpi3
*/



/*/dev/ttyS0 refers to the mini uart, the primary uart on the pi3
  /dev/ttyAMA0 refers to the first PL011(UART0) check this in the documentation
  on pi website
  The primary UART for RPI3 is the mini UART. As I understand it paritet understand
  throughput is not supported by this. The secondary UART on RPI3 is connected to bluetooth
  and needs to be disabled

  VIKTIGT. Värt att stänga bluetooth från AMA0 då det kanske kan spöka

  Allt detta är ett första förslag till hur det kan fungera när vi har spridd
  funktionalitet. Vi tänker för nuvarande att det måste finnas ett main program
  någonstans som initierar portar för att kunna ta emot data från det olika modulerna
  , men som också lyssnar till kommandon/portarna för att kunna ta emot data
  eller skicka. Funktionalitet i detta dokument ska bistå josef´s programkod där
  han beroende på information från pc vill skicka ut lämplig data till olika
  enheter

*/



/*------------------------------------------------------------------------------
Funktionalitet för informationsfördelning utifrån kommandon från pc
Fjärrstyrning

send()- utifrån ett visst kommando från pc så kommer tolkade värden att
vidarbefodras till respektive enhet med hjälp av send() och tillhörande
hjälpfunktioner.

------------------------------------------------------------------------------*/

 /* Initiering av specifierad port */
 int portInit(string port) {
   if((serialPort = serialOpen(port, 9600))< 0) /*opening the serial port */
   {
     fprintf(stderr, "Unable to open serial device: %s\n", sterror (errno));
     return 1;
   }
   return serialPort;
  }
/*----------------------------------------------------------------------------*/

/*Code wont work if we don't initialize this */
int initWiringPi() {
    if (wiringPiSetup() == -1)					/* initializes wiringPi setup */
     {
       fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
       return 1 ;
     }
     return 0;
  }
/*----------------------------------------------------------------------------*/

/* Skickar data till specifierad plats. Vi kan även med denna funktion begära
att data skickas från en specifierad plats. Detta kan göras genom att själva
meddelandet(message) då för klargöra att den begära information och såna fall
vilken informations.*/

string send(string sendLoc, string message) {
   if(initWiringPi() == 1) {
     return "Couldn't setup wiringPi";
   };
   int serialPort;
   string usbPort;
   char chars = splitToChar(message);
   switch(sendloc) {
     case "styrenhet";
     usbPort = "/dev/ttyUSB0"; /* Detta beror på hur vi kopplat de olika
                                  enheterna till rpi3, 'ttyUSB0' kan komma att ändras */
     break;
     case "sensorenhet":
     usbPort = "/dev/ttyUSB1";
     break;
     default:
   }  
   if(serialPort = portInit(usbPort)< 0) {
     return "failed to open port";
   }
   for(int i = 0, i < message.length(), i++) {
     fflush(stdout);
     serialPutchar(serialPort, chars[i]);
   }
   serialClose(serialPort);
   return "success";
 }

/* Uppdelning av meddelandet för att kunna skickas ut på usb-port */
char splitToChar(string message) {
   int n = message.length();
   char charArray[n+1];
   strcpy(charArray, message.c_str());
   return charArray;
 }




 /*This is a simple test to verify that the port on the pi is working accordingly
 and can recieve/transmit data, this will later change to allow for serial communication
 between devices and will also incorporate interupt handling if necessary */

int main() {
  int serial_port;
  char dat;

  if((serial_port = serialOpen("/dev/ttyS0", 9600))< 0) /*opening the serial port */
  {
    fprintf(stderr, "Unable to open serial device: %s\n", sterror (errno));
    return 1;
  }

  if (wiringPiSetup() == -1)					/* initializes wiringPi setup */
   {
     fprintf (stdout, "Unable to start wiringPi: %s\n", strerror (errno)) ;
     return 1 ;
   }

   while(1){

 if(serialDataAvail (serial_port) )
 {
   dat = serialGetchar (serial_port);		/* receive character serially*/
   printf ("%c", dat) ;
   fflush (stdout) ;
   serialPutchar(serial_port, dat);		/* transmit character serially on port */
     }
 }

}
