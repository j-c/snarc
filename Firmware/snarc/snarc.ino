// Buzz 28 Mar 2012
// see https://github.com/davidbuzz/snarc for more info


// flashing GREEN LED means operating correctly!
// flashing RED LED means "in programming mode"

//LED related variables:
#define LED_PIN1 5  //GREEN HIGH
#define LED_PIN2 6  //RED HIGH
enum LEDSTATES { RED , GREEN , OFF } ;
int ledState = RED;  // toggle to turn the LEDS on/off/etc
long previousMillis = 0;
long interval = 1000;           // interval at which to blink (milliseconds)


#define THISSNARC_OPEN_PIN 54  //54 on the mega, or 14 on the normal arduino
#define THISSNARC_RFID_ENABLE_PIN 2


#include <SPI.h> //needed by Ethernet.h
#include <EEPROM.h>

#include <Ethernet.h>

#define CLIENT 1   // comment out this line to completely disable all ETHERNET and networking related code

#include <SoftwareSerial.h>

/* USAGE: be sure to revise this script for each peice of hardware:
 * change the Mac address
 * change the IP address that it should be assigned to ( default is 192.168.7.7 ) 
 * make sure the "server" it will authenticate to is correct ( default is 192.168.7.77 ) 
 * make sure the URL that is being used for your "auth server" is correct.  ( default is /auth.php? ) 
 * make sure you have it wired-up correctly! Code currently assumes the RFID reader is on Software Serial on RX/TX pins  D15 and D16 ( also sometimes called A1 and A2 )
*/
void write_codes_to_eeprom(void);


#ifdef CLIENT
// network settings:
byte mac[] = {  0xDE, 0xDA, 0xEB, 0xFE, 0xFE, 0xEE };

IPAddress ip(192,168,7,7); // client arduino

//byte gateway[] = {  
// 192, 168, 1, 254 };
//byte subnet[] = {  
// 255, 255, 0, 0 };

IPAddress server( 192, 168, 7, 77 ); // destinaton end-point is "auth server" internal IP.

// http recieve buffer
char client_recieve_data[16];
int client_recieve_pointer = 0;
// http timeouts etc
long unsigned int client_timeout;
int x_client_finished;

// be a http client, not a server, connect to port 80
EthernetClient client;

#endif

SoftwareSerial RFIDSerial(15, 16);  //RFID reader is on Soft Serial port.   A1-A5 are D15-D20


// where in EEPROM do we write the next EEPROM?
// this field is populated by either write_codes_to_eeprom() or read_eeprom_codes() or find_last_eeprom_code()
// and after each of tehse will be left pointing to the "next slot" in the EEPROM to write to
unsigned int last_address = 0;

// this may be used by find_code_in_eeprom() to point to the last-located code's EEPROM address ( for if we need to update it)
unsigned int last_found_address = 0;


// last detected code, and which door it was that triggered it.
char last_code[12];
int  last_door ;

//when writing an invalid code, write this one:
char invalidCode[] = "          ";

// If you use more than one door in a "set" of readers, you can name there all here. Keep this consistent between all doors, and also the auth.php script needs it too! 
// Always put "INVALID" as the first element, then other elements follow:
// eg multi-door system: 
enum AccessType {  INVALID, INNER , OUTER, BACK, OFFICE1, SHED, LIGHTS };
// eg one door system:
//enum AccessType {  INVALID, INNER };


//  Which of the above list is this particulare one?  ( this should be different for each door/reader that has different permissions ) 
//  This tells us which "bit" in the access data we should consider ours! 
// eg: this means that if the server sends "access:1, then only the "INNER" door is permitted. if the server sends "access:3" then both the INNER and OUTER door are permitted.  If it sends "access:8" then only "SHED" is permittd  ( it's a bit-mask ) 
#define THISSNARC SHED // in this case, this means any number with bit 5 set to 1 will cause us to trigger the mosfet nad "open door", eg, the number 16


void setup()
{

    // set the data rate for the SoftwareSerial port


// logging/programming/USB interface   
Serial.begin(19200);

Serial.println(F("started - network will initialise in approx 30 secs! "));



// "request exit" button input
//pinMode(REX_PIN, INPUT);
//digitalWrite(REX_PIN, HIGH);

// status led
pinMode(LED_PIN1, OUTPUT);
pinMode(LED_PIN2, OUTPUT);

digitalWrite(LED_PIN1, LOW);
digitalWrite(LED_PIN2, LOW);
 ledState = OFF;

// Inner Door reader
RFIDSerial.begin(2400); // RFID reader SOUT pin connected to Serial RX pin at 2400bps
pinMode(THISSNARC_RFID_ENABLE_PIN, OUTPUT); // RFID /ENABLE pin
digitalWrite(THISSNARC_RFID_ENABLE_PIN, LOW);
pinMode(THISSNARC_OPEN_PIN, OUTPUT); // Setup internal door open
digitalWrite(THISSNARC_OPEN_PIN, LOW);


// Ethernet module
#ifdef CLIENT
Ethernet.begin(mac,ip);
#endif

//
programming_mode();


Serial.println(F("Entering Normal Mode! "));


}

void prompt() {
Serial.println(F("PROGRAM MODE [r n x]>"));
}


void programming_mode() {

Serial.println(F("Press a few keys, then ENTER *NOW* to start programming mode ( 3...2...1... ) "));
Serial.flush();

delay(3000);

// three ENTER keys is >2, AND WE ARE IN PROGRAMMING MODE !  
int incomingByte = 0;
if (Serial.available() > 2) {
  
   
    Serial.println(F("Entered Programming Mode! "));
    Serial.println(F("Type 'r' to read eeprom list, 'n' to program new key to EEPROM. 'x' to exit mode. "));
    prompt();

    while( incomingByte != -1 ) {
     
     led_flash(RED);   // RED FLASH FOR PROGRAMMING MODE


    if ( Serial.available() ) {
      incomingByte = Serial.read();
      Serial.println((char)incomingByte);

      switch((char)incomingByte) {
    case 's':
        // test send to server
        Serial.println(F("test send_to_server()"));
        Serial.println(send_to_server("1234567890", 0));
        break;
        // w means "write" initial LIST to EEPROM cache - undocumented command for initial population of eeprom only during transition.
      case 'w':
        write_codes_to_eeprom();
        Serial.print(F("address:"));
        Serial.println(last_address);
        prompt();
        break;
        
      case 'i':
        init_eeprom(); //wipe all codes
        Serial.print(F("address:"));
        Serial.println(last_address);
        prompt();
        break;

        // r mean read current list from EEPROM cache
      case 'r':
        read_eeprom_codes();
        Serial.print(F("address:"));
        Serial.println(last_address);
        prompt();
        break;

        // x mean exit programming mode, and resume normal behaviour
      case 'x':
        incomingByte = -1; // exit this mode
        break;

        // ignore whitespace
      case '\r':
      case '\n':
      case ' ':

        break;

        // n means write new code to EEPROM
        // ( the next key scanned
      case 'n':
        {
          
          //read_eeprom_codes(); // nee to do this so as to get last_address pointing ot the end of the EEPROM list.
          find_last_eeprom_code();  // updates last_address to correct value without any output.

          listen_for_codes();
          // result is in last_door and last_code

          unlockDoor(); // clunk the door as user feedback

          // see if the key already has access
          int access = matchRfid(last_code) & 0xF;


	  // IF THIS DOOR/READER is not already in the permitted access list from the EEPROM/SERVER allow it! 
	  // this converts the bit number, to its binary equivalent, and checks if that bit is set in "access"
	  // by using a binary "and" command. 
          if ( access & (  1 << ( THISSNARC - 1 ) ) ==  0 ) {

            // append this ocde to the EEPROM, with current doors permissions:
            write_next_code_to_eeprom(last_code, THISSNARC);

          } else {
            Serial.println(F("Card already hs access, no change made"));  
          }
          prompt();
        }   
        break;  

      default:   
        // nothing
        prompt();
        break;
      } //switch/case
    } // if
    } //while

}  //END PROGRAMMING MODE

Serial.println(F("Exited Programming Mode! "));

}


// on-demand, connect, GET + data, and disconnect
// return value is the access permission the server said for that user.
int send_to_server(char *tag, int door ) {

// side-effect: LOG TO USB SERIAL:
Serial.print(F("OK: Tag is "));
Serial.print(tag);
Serial.println(F(" .")); // needed by space server auth script to represent end-of-communication.

#ifdef CLIENT
// REPORT TO HTTP GET REQUEST:
if (client.connect(server,80)) {
    Serial.println(F("http client connected"));
    client.print("GET /auth.php?q=");
    client.print(tag);
    client.print("&d=");
    client.println(door);
    client.println();
}
else {
    Serial.println(F("http connection failed"));
//  Serial.print(F("GET /auth.php?q="));
//  Serial.println(tag);
//  Serial.println();
}
#endif

// delay some arbitrary amount for the server to respond to the client. say, 1 sec. ?
delay(3000);

// if there are incoming bytes available
// from the server, read them and print them:
client_timeout=millis();
client_recieve_pointer = 0;
#ifdef CLIENT
    for (;;)
    {
      if (client.available())
      {
        //char c = client.read();
        client_recieve_data[client_recieve_pointer++] = client.read();
        //Serial.print(c);
        }

        // if the server's disconnected, stop the client:
        if (!client.connected())
        {
            client.stop();
            break;
        }
    }
#endif
client_recieve_pointer = 0;
// if the server's disconnected, stop the client:
Serial.print(F("http data:"));
// recieved data is now in the string: client_recieve_data
Serial.println(client_recieve_data);

// we expect the permissions string to look like 'access:3' ( for permit ), or 'access:0' (for deny )
String Permissions =  String(client_recieve_data);
  
int colonPosition = Permissions.indexOf(':');

String scs = Permissions.substring(colonPosition + 1);  // as a "String" "object" starting from after the colon to the end! 
char cs[10]; 
scs.toCharArray(cs,10); // same this as a char array "string" ( lower case) 
Serial.print(F("perms from server:"));
Serial.println(cs); 
int ci = atoi(cs); // as an int!  if this fails, it returns zero, which means no-access, so that's OK. 

// basic bound check: 
if ( ci < 0 || ci > 255 ) { 
  return INVALID;   
} 
return ci;   
}

// scan the EEPROM looking for a matching RFID code, and return it if it's found
//also, as a side-affect, we leave 
int matchRfid(char * code)
{
int address = 0;
int result = 0; // access level from EEPROM
boolean match = false;

while(!match)
{
    if((result = EEPROM.read(address*11)) == INVALID)
    {
    // end of list
    Serial.println(F("Error: No tag match"));
    return INVALID;
    }
    match = true;
    for(int i=0; i<10; i++)
    {
    if(EEPROM.read(address*11+i+1) != code[i])
    {
      match = false;
      break;
    }
    }
    address++;
}
Serial.print(F("OK: Match found, access "));
Serial.println(result);

last_found_address = address-1; //remember it incase we need to "UPDATE" this location later....

return result;
}

// pull the codes from EEPROM, emit to Serial0
void read_eeprom_codes() {

int address = 0; // eeprom address pointer
int result = 0;  
char codeout[12]; // holder for code strings as we output them

// read EEPROM in 11 byte chunks till we get an INVALID tag to label the end of them.
while((result = EEPROM.read(address*11)) != INVALID)
{
    for(int i=0; i<10; i++)
    {
    codeout[i] = EEPROM.read(address*11+i+1);
    }
    codeout[10] = '\0'; //end of string

    // write the current code to the USB port for the user!
    Serial.print(F("code: "));
    Serial.println(codeout);

    address++;
}

last_address = address;
}

// simpler/quieter variant of the above, used in different circumstance.
void find_last_eeprom_code() {

int address = 0; // eeprom address pointer
int result = 0;  
//  char codeout[12]; // holder for code strings as we output them

// read EEPROM in 11 byte chunks till we get an INVALID tag to label the end of them.
while((result = EEPROM.read(address*11)) != INVALID)
{
    address++;
}

last_address = address;
}

void unlockDoor()
{
digitalWrite(THISSNARC_OPEN_PIN, HIGH);
digitalWrite(LED_PIN1, HIGH);
digitalWrite(LED_PIN2, LOW);
Serial.println(F("OK: Opening internal door"));
delay(2000);
digitalWrite(THISSNARC_OPEN_PIN, LOW);
digitalWrite(LED_PIN1, LOW);
digitalWrite(LED_PIN2, LOW);
//Serial.println(F("OK: Normalising internal door"));
}



void loop () {
// char code[12]; // 10 chars of code , 1 byte of null terminator, and one byte of the door number.

// reset global variables each time thru
last_code[10]=0; // null terminator
last_code[0]=0;
last_door =  INVALID;

// int door = INVALID; // assume INVALID to start with.


listen_for_codes();   // blocking, waits for a code/door combo, then writes the result into global last_code and last_door

//    Serial.println(F("TAG detected!"));
//    Serial.println(last_code);  //don't tell user the full tag number

// NOTE, THIS MATCHES AGAINS THE EEPROM FIRST, NOT THE REMOTE SERVER, AS ITS FASTER ETC.  
// IN ORDER TO UPDATE OUR EEPROM AGAINST THE REMOTE SERVER, WE DO THAT *AFTER* allowing someone through ( for speed)
// but before denying them fully.
int access = matchRfid(last_code) & 0xF; // high bits for future 'master' card flag

// last_code = the just-scanned RFID card string   
// last_door = the integer representing the door this code was just scanned at  (not necessarily permitted )

int serveraccess = INVALID; // what the remote server is willing to allow the user, we cache this info to EEPROM

// THISSNARC DOOR CHECK
if ( last_door == THISSNARC ) {
    // 
    if( access & ( 1 << ( THISSNARC - 1 ) ) )  
    {
    unlockDoor();
    serveraccess = send_to_server(last_code,last_door); //log successes/failures/etc, and return the permissions the server has.

    // after the fact, if the server access and the local cached access don't match, update the local one
    // this can be used for "revoking" keys, by setting they access to INVALID/0
      if ( serveraccess != access ) { 
	writeCode( last_found_address, last_code , serveraccess ) ;
      }  
    }
    else {
    Serial.println(F("Error: Insufficient rights in EEPROM for THISSNARC door - checking server"));  // must end in fullstop!
    serveraccess = send_to_server(last_code,last_door); //log successes/failures/etc, and return the permissions the server has.
    if(serveraccess & ( 1 << ( THISSNARC -1 ) ) ) {
      unlockDoor();
      //NOTE:   to get here, the key was NOT in EEPROM, but WAS on SERVER, SO ALLOW IT, AND CACHE TO EEPROM TOO!
      if ( access != THISSNARC ) {
        // append this ocde to the EEPROM, with BOTH doors permissions:
        write_next_code_to_eeprom(last_code,serveraccess);
      }  
    }
    else {
      Serial.println(F("Error: Insufficient rights ( from server) for THISSNARC door "));
    }
    }
}




}



// blocking function that waits for a card to be scanned, and returns the code as a string.
// WE ALSO supply THE DOOR/READER NUMBER AS THE 12th BYTE in the string ( the 11th byte is the null string terminator )
// result data is put into global last_code, not returned.
void listen_for_codes ( void ) {

// boolean

// before starting to listen, clear all the Serial buffers:
while (RFIDSerial.available() > 0) {
    RFIDSerial.read();
}

RFIDSerial.flush();


Serial.println(F("listening to READERS, please swipe a card now"));

int  found = 0 ;
while ( found == 0 ) {


    //     char code[12];
    int val;
    last_code[10]=0;
    last_code[0]=0;
    last_door = INVALID;


    led_flash(GREEN);   // GREEN FLASH FOR READY MODE
  

    if (RFIDSerial.available() > 0) // input waiting from internal rfid reader
    {
    if ((val = RFIDSerial.read()) == 10)
    {
      int bytesread = 0;
      while (bytesread < 10)
      { // read 10 digit code
        if (RFIDSerial.available() > 0)
        {
          val = RFIDSerial.read();
          if ((val == 10) || (val == 13))
          {
            break;
          }
          last_code[bytesread++] = val;
        }
      }
      if(bytesread == 10)
      {
        Serial.println(F("TAG detected!"));
        Serial.println(last_code);  //don't tell user the full tag number

        // just in case.....
        Serial.flush();
        RFIDSerial.flush();

        // which door was this?
        last_door = THISSNARC;

        found = 1 ;

      }
      Serial.flush();
      bytesread = 0;
    }

    }


}

Serial.flush();

}

void led_flash(int whichled) {

  unsigned long currentMillis = millis();

 if(currentMillis - previousMillis > interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;   

    // if the LED is off turn it on and vice-versa:
    if (ledState == whichled)  {
     ledState = OFF;
     led_off();  //both off!
    }
    else {
     ledState = whichled;
     led_on(whichled);
    }

 }

}
void led_on(int which){
 if ( which == RED ) {
    digitalWrite(LED_PIN1, LOW);
    digitalWrite(LED_PIN2, HIGH);
 } else {
        digitalWrite(LED_PIN1, HIGH);
    digitalWrite(LED_PIN2, LOW);
 }
}
void led_off(){
    digitalWrite(LED_PIN1, LOW);
    digitalWrite(LED_PIN2, LOW);
}


/////////////////////FROM PROGRAMMER CODE!
// Upload rfid codes into EEPROM

//#include <EEPROM.h>


// currently MAX of approx 50? 
// SIZE data; each is 11 bytes, and there are 50, so memory used is 550-600 bytes
// ... which would be better in the 4K EEPROM, or the 128K PROGMEM , but in 8K SRAM on a Mega it's OK too.
// IF YOU WANT TO HARDCODE ALL YOUR RFID CARDS IN THE CODE, STORE THEM HERE, and use the "w" command in programming mode!
// ( not recommended for poduction, there are better and easier ways to do it!  ) 
//
char * codeListBoth[]  =
{
"1234567890",
"\0"  //EOF
};



void writeCode(int address, char * code, int accessLevel)
{
digitalWrite(LED_PIN1, HIGH);
digitalWrite(LED_PIN2, LOW);
//delay(10);

Serial.print(F("Writing to address "));
Serial.println(address);

Serial.print(F("Access "));
Serial.println(accessLevel);
EEPROM.write(address*11,accessLevel);

Serial.print(F("Code "));
for(int i = 1; i<11; i++)
{
    EEPROM.write(address*11+i,code[i-1]);
    Serial.print(code[i-1]);
}
Serial.println("");

digitalWrite(LED_PIN1, LOW);
digitalWrite(LED_PIN2, LOW);

}



// write a single code (  and it's permissions bitmask ) 
void write_next_code_to_eeprom(char * code, int dooraccess ){

// first locate the end of the EEPROM list. Updates last_address variable to correct value without any output.
find_last_eeprom_code();  

writeCode( last_address, code, dooraccess ) ;
last_address++;
writeCode( last_address, invalidCode, INVALID ) ; // termination code after loast code is ESSENTIAL to know where end of list is.
}

void write_codes_to_eeprom()
{
int address = 0;

int codeIdx = 0;
while(codeListBoth[codeIdx][0]!=0)
{
    writeCode(address, codeListBoth[codeIdx], THISSNARC);
    address++;
    codeIdx++;
}

// char invalidCode[] = "          ";
writeCode(address, invalidCode, INVALID);
Serial.println(F("Finished"));

last_address = address;

}

// wipres eeprom, and initialised it for codes bbeing programmed later
void init_eeprom()
{
int address = 1;

writeCode(address, invalidCode, INVALID);
Serial.println(F("Init Finished"));

last_address = address;

}




