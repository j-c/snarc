// https://github.com/j-c/snarc

#include "MemoryFree.h"
#include "configuration.h"

#ifdef ENABLE_ESTOP
	bool lockdown = false;
#endif

#include <EEPROM.h>

#ifdef ETHERNET
	#include <SPI.h> //needed by Ethernet.h
	#include <Ethernet.h>
#endif

#include <SoftwareSerial.h>


#include "utils.h"
#include "led.h"
#include "ProgrammingMode.h"

#include "IRFIDReader.h"
#include "IAuthenticator.h"
#include "IActuator.h"

// Single RED FLASH with BLINKING GREEN programming mode. Will FLASH RED with each command.
// Single RED FLASH during operation means card ID does not have access.
// Single GREEN FLASH during operation means card ID has access.
// SOLID RED with BLINKING GREEN means E-Stop condition.


char device_name[DEVICE_NAME_MAX_LENGTH + 1]; // +1 for terminating char



/* USAGE: be sure to revise this script for each peice of hardware:
 * change the IP address that it should be assigned to ( default is 192.168.7.7 ) 
 * make sure the "server" it will authenticate to is correct ( default is 192.168.7.77 ) 
 * make sure the URL that is being used for your "auth server" is correct.  ( default is /auth.php? ) 
 * make sure you have it wired-up correctly! Code currently assumes the RFID reader is on Software Serial on RX/TX pins  D15 and D16 ( also sometimes called A1 and A2 )
*/

#ifdef ETHERNET
// network settings:
byte mac[] = {0, 0 ,0 ,0 ,0 ,0};
byte clientIpBytes[] = {0, 0, 0, 0};

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
#else
	char client_recieve_data[16];
	int client_recieve_pointer = 0;
	long unsigned int client_timeout;
#endif

RFID_READER_CLASS rfidReaderInstance;
IRFIDReader * rfidReader = &rfidReaderInstance;

AUTHENTICATOR_CLASS authenticatorInstance;
MockAuthenticator * authenticator = &authenticatorInstance;

ACTUATOR_CLASS actuatorInstance;
IActuator * actuator = &actuatorInstance;


enum AccessType {  INVALID, INNER , OUTER, BACK, OFFICE1, SHED, LIGHTS };
// eg one door system:
// enum AccessType {  INVALID, INNER };

//  Which of the above list is this particulare one?  ( this should be different for each door/reader that has different permissions ) 
//  This tells us which "bit" in the access data we should consider ours! 
// eg: this means that if the server sends "access:1, then only the "INNER" door is permitted. if the server sends "access:3" then both the INNER and OUTER door are permitted.  If it sends "access:8" then only "SHED" is permittd  ( it's a bit-mask ) 
#define THISSNARC SHED // in this case, this means any number with bit 5 set to 1 will cause us to trigger the mosfet nad "open door", eg, the number 16


void setup()
{
	// Needed for random MAC address
	randomSeed(analogRead(A1));

	Serial.begin(SNARC_BAUD_RATE);

	Serial.println();
	load_device_name();

	// Display basic config details
	Serial.print(F("Name: "));
	print_device_name();
	Serial.println();

	#ifdef ETHERNET
	load_ip_address();
	ip = IPAddress(clientIpBytes[0], clientIpBytes[1], clientIpBytes[2], clientIpBytes[3]);
	Serial.print(F("IP:   "));
	print_ip_address();
	Serial.println();

	Serial.print(F("MAC:  "));
	get_mac_address();
	print_mac_address();
	Serial.println();
	#endif

	pinMode(ACTUATOR_PIN, OUTPUT);
	digitalWrite(ACTUATOR_PIN, LOW);

	#ifdef ENABLE_ESTOP
	eStopInit();
	#endif

	Serial.println(F("Started"));

	// Status LEDs
	pinMode(RED_LED_PIN, OUTPUT);
	digitalWrite(RED_LED_PIN, LOW);
	pinMode(GREEN_LED_PIN, OUTPUT);
	digitalWrite(GREEN_LED_PIN, LOW);

	// Ethernet module
	#ifdef ETHERNET
	bool useDhcp = true;
	for (byte i = 0; i < 4; i++)
	{
		if (clientIpBytes[i] != 0)
		{
			useDhcp = false;
			break;
		}
	}
	if (useDhcp)
	{
		Serial.println("Attempting to aquire IP via DHCP");
		if (Ethernet.begin(mac) == 0)
		{
			Serial.println(F("Failed to configure Ethernet using DHCP"));
			// TODO: Do somehting aout this error
		}
	}
	else
	{
		Ethernet.begin(mac, ip);
	}
	#endif

	programmingMode();


	Serial.println(F("Entering Normal Mode! "));
}


void loop ()
{
	manageLeds();

	#ifdef ENABLE_ESTOP
	if (lockdown)
	{
		eStopLockdown();
	}
	#endif

	
	// Check to see if card is available
	if (rfidReader->available())
	{
		Serial.print("Memory free: ");
		Serial.println(freeMemory());
		Serial.print("E-Stop pin: ");
		Serial.println(digitalRead(ESTOP_PIN) ? "HIGH" : "LOW");
		Serial.print("Lockdown: ");
		Serial.println(lockdown ? "true" : "false");
		unsigned long id = rfidReader->read();
		if (authenticator->authenticate(id, device_name))
		{
			// Success
			flashLed(GREEN, 2000);
			manageLeds();
			Serial.println(F("ID has access"));
			actuator->on();
		}
		else
		{
			// Fail
			flashLed(RED, 1000);
			Serial.println(F("ID does not have access"));
			// TODO: Implement timeout and turn off if no valid card found
		}
	}

}





// EVERYTHING BELOW HERE WILL MOST LIKELY DIE IN A FIRE
int last_code = 0;
int last_door = 0;
int last_address = 0;
int last_found_address = 0;

void clear_serial_buffer ()
{
	// nothing
}
void prompt(){}
void programming_mode() {
/*
	clearSerial();
	Serial.println(F("Press ENTER within 3 seconds to enter programming mode"));
	serialListenForInput(1, 3000, false, true, true);
	int incomingByte = 0;

	#ifdef DEBUG
	if (true) // DEBUG: Always enter programming mode
	#else
	if (serial_recieve_index > 0);
	#endif
	{
	  	clear_serial_buffer();
	    Serial.println(F("Entered Programming Mode! "));
	    prompt();

	    while( incomingByte != -1 ) {
     
		//led_flash(RED);   // RED FLASH FOR PROGRAMMING MODE

		if ( Serial.available() ) {
      incomingByte = Serial.read();
      Serial.println((char)incomingByte);

      switch((char)incomingByte) {
		case 's':
			// test send to server
			Serial.println(F("test send_to_server()"));
			Serial.println(send_to_server((unsigned long)1234567890, 0));
	        prompt();
			break;
		#ifdef ETHERNET
		case 'm':
			// set MAC address
			listen_for_new_mac_address();
	        prompt();
			break;
		case 'a':
			// TODO: Implement IP address change
			listen_for_ip_address();
			prompt();
			break;
		#endif
		case 'd':
			listen_for_device_name();
			prompt();
			break;
        // w means "write" initial LIST to EEPROM cache - undocumented command for initial population of eeprom only during transition.
      case 'w':
        write_codes_to_eeprom();
        Serial.print(F("address:"));
        //Serial.println(last_address);
        prompt();
        break;
        
      case 'i':
        init_eeprom(); //wipe all codes
        Serial.print(F("address:"));
        //Serial.println(last_address);
        prompt();
        break;

        // r mean read current list from EEPROM cache
      case 'r':
        read_eeprom_codes();
        Serial.print(F("address:"));
        //Serial.println(last_address);
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
          int access =  0;//matchRfid(last_code) & 0xF;


	  // IF THIS DOOR/READER is not already in the permitted access list from the EEPROM/SERVER allow it! 
	  // this converts the bit number, to its binary equivalent, and checks if that bit is set in "access"
	  // by using a binary "and" command. 
          if ( access & (  1 << ( THISSNARC - 1 ) ) ==  0 ) {

            // append this ocde to the EEPROM, with current doors permissions:
           // TEST // write_next_code_to_eeprom(last_code, THISSNARC);

          } else {
            Serial.println(F("Card already hs access, no change made"));  
          }
		  Serial.println(last_code);
          prompt();
        }   
        break;  

      default:   
        // nothing
        prompt();
        break;
      } //switch/case
	  clear_serial_buffer();
    } // if
    } //while
}  //END PROGRAMMING MODE

Serial.println(F("Exited Programming Mode! "));
#ifdef ENABLE_ESTOP
	if (lockdown)
	{
		Serial.println(F("E-STOP triggered while in programming mode."));
	}
#endif
	*/
}

int send_to_server(unsigned long tag, int door )
{

	// side-effect: LOG TO USB SERIAL:
	Serial.print(F("OK: Tag is "));
	Serial.print(tag);
	Serial.println(F(" .")); // needed by space server auth script to represent end-of-communication.

	#ifdef ETHERNET
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
	#ifdef ETHERNET
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

// on-demand, connect, GET + data, and disconnect
// return value is the access permission the server said for that user.
int send_to_server(char *tag, int door ) {

	// side-effect: LOG TO USB SERIAL:
	Serial.print(F("OK: Tag is "));
	Serial.print(tag);
	Serial.println(F(" .")); // needed by space server auth script to represent end-of-communication.

	#ifdef ETHERNET
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
	#ifdef ETHERNET
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

int matchRfid(unsigned long code)
{
	int cardAddress = 0;
	int readAddress = EEPROM_CARDS_START_ADDRESS;
	int result = 0;
	bool match = false;
	while (!match)
	{
		if((result = EEPROM.read(readAddress)) == INVALID)
		{
			// end of list
			Serial.println(F("Error: No tag match"));
			return INVALID;
		}
		else
		{
			unsigned long eepromCard = 0;
			byte* p = (byte*)(void*)&eepromCard;
			for (byte i = 0; i < sizeof(eepromCard); i++)
			{
				*p++ = EEPROM.read(readAddress + 1 + i);
			}
			if (code == eepromCard)
			{
				Serial.print(F("OK: Match found, access "));
				match = true;
			}
			cardAddress++;
			readAddress = EEPROM_CARDS_START_ADDRESS + cardAddress * 5;
		}
	}
	if (match)
	{
		Serial.println(result);
	}
	else
	{
		Serial.println(F("No match found"));
	}
	
	last_found_address = cardAddress - 1; //remember it incase we need to "UPDATE" this location later....

	return result;

}

// scan the EEPROM looking for a matching RFID code, and return it if it's found
//also, as a side-affect, we leave 

// pull the codes from EEPROM, emit to Serial0
void read_eeprom_codes()
{
	int readAddress = EEPROM_CARDS_START_ADDRESS; // eeprom address pointer
	int cardAddress = 0; // This is the actual card "slot"
	int result = 0;  
	//char codeout[12]; // holder for code strings as we output them
	unsigned long codeout = 0;
	int i = 0;
	// read EEPROM in 5 byte chunks till we get an INVALID tag to label the end of them.
	while((result = EEPROM.read(readAddress)) != INVALID)
	{
		Serial.print("Card: ");
		codeout = 0;
		byte* p = (byte*)(void*)&codeout;
		for (byte i = 0; i < sizeof(codeout); i++)
		{
          *p++ = EEPROM.read(readAddress + 1 + i);
		}
		Serial.print(codeout);
		Serial.print("; Access: ");
		Serial.println(result);
		cardAddress++;
		readAddress = EEPROM_CARDS_START_ADDRESS + cardAddress * 5;
	}
	//last_address = cardAddress;
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

	//last_address = address;
}

void unlockDoor()
{
	Serial.println(F("OK: Opening internal door"));
	actuator_on();
}


void actuator_on()
{
	#ifdef ENABLE_ESTOP
	if (lockdown)
	{
		digitalWrite(ACTUATOR_PIN, LOW); // Force low
		eStopLockdown();
	}
	#endif
	Serial.println(F("OK: Activating actuator."));
	actuator->on();
}

void actuator_off()
{
	Serial.println(F("OK: Disabling actuator."));
	actuator->off();
}

void loop2 () {
	#ifdef ENABLE_ESTOP
	if (lockdown)
	{
		eStopLockdown();
	}
	#endif
	
	// reset global variables each time thru
	//last_code = 0;
	last_door =  INVALID;

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
	if ( last_door == THISSNARC )
	{
		// 
		if( access & ( 1 << ( THISSNARC - 1 ) ) )  
		{
		unlockDoor();
		serveraccess = send_to_server(last_code,last_door); //log successes/failures/etc, and return the permissions the server has.

		// after the fact, if the server access and the local cached access don't match, update the local one
		// this can be used for "revoking" keys, by setting they access to INVALID/0
		  if ( serveraccess != access )
		  { 
			writeCode(last_found_address, last_code , serveraccess) ;
		  }  
		}
		else
		{
			Serial.println(F("Error: Insufficient rights in EEPROM for THISSNARC door - checking server"));  // must end in fullstop!
			serveraccess = send_to_server(last_code, last_door); //log successes/failures/etc, and return the permissions the server has.
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
void listen_for_codes ( void )
{
	// before starting to listen, clear all the Serial buffers:
	/*
	while (RFIDSerial.available() > 0) {
		RFIDSerial.read();
	}

	RFIDSerial.flush();
	Serial.println(F("listening to READERS, please swipe a card now"));

	int  found = 0 ;
	while ( found == 0 )
	{
		#ifdef ENABLE_ESTOP
		if (lockdown)
		{
			estop_lockdown();
		}
		#endif

		int val;
		last_code = 0;
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
				  val = Serial.read();
				  bytesread++;
				  Serial.println(val);
				  if ((val == 10) || (val == 13))
				  {
					break;
				  }
				  last_code = last_code * 10 + (val - 48);
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
	*/
	Serial.flush();
}

void led_flash(int whichled) {
}

void led_on(int which){
}

void led_off(){
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
/*
char * codeListBoth[]  =
{
"1234567890",
"\0"  //EOF
};
*/
unsigned long codeListBoth[] = {
	1,
	1234567890,
	4294967295
};


void writeCode (int address, char *code, byte accessLevel)
{
	Serial.println("nothing happened. writeCode (int address, char *code, byte accessLevel) is depricated");
}

/**
 * Saves the 32bit id as a number. Using 5 bytes (1 byte + 4 ulong)
 * address is actually "slot". Actual eeprom address = EEPROM_CARDS_START_ADDRESS + address * 5;
 */
void writeCode (int address, unsigned long code, byte accessLevel)
{
	int writeAddress = EEPROM_CARDS_START_ADDRESS + address * 5;
	//digitalWrite(LED_PIN1, HIGH);
	//digitalWrite(LED_PIN2, LOW);
	//delay(10);

	Serial.print(F("Writing to address "));
	Serial.println(address);

	Serial.print(F("Access "));
	Serial.println(accessLevel);
	EEPROM.write(writeAddress++, accessLevel);

	Serial.print(F("Code "));

	const byte* p = (const byte*)(const void*)&code;
    unsigned int i;
    for (i = 0; i < sizeof(code); i++)
	{
		EEPROM.write(writeAddress++, *p++);
	}
    
	Serial.println(code);
	
	//digitalWrite(LED_PIN1, LOW);
	//digitalWrite(LED_PIN2, LOW);
}

void write_next_code_to_eeprom(char *code, byte dooraccess)
{
	Serial.println("nothing happened. write_next_code_to_eeprom(unsigned long code, byte dooraccess) is depricated");
}

// write a single code (  and it's permissions bitmask ) 
void write_next_code_to_eeprom(unsigned long code, byte dooraccess)
{
	// first locate the end of the EEPROM list. Updates last_address variable to correct value without any output.
	find_last_eeprom_code();  
	//writeCode( last_address, code, dooraccess ) ;
	last_address++;
	//writeCode( last_address, (unsigned long)0, INVALID ) ; // termination code after last code is ESSENTIAL to know where end of list is.
}

void write_codes_to_eeprom()
{
	int address = 0;
	for (int i = (sizeof(codeListBoth) / sizeof(codeListBoth[0]))- 1; i >= 0; i--)
	{
		writeCode(address++, codeListBoth[i], THISSNARC);
	}
	writeCode(address, (unsigned long)0, INVALID);
	Serial.println(F("Finished"));

	//last_address = address;
}

// wipes eeprom, and initialised it for codes being programmed later
void init_eeprom()
{
	int address = 0;
	writeCode(address, (unsigned long)0, INVALID);
	Serial.println(F("Init Finished"));
	//last_address = address;
}


// MAC address stuff
#ifdef ETHERNET
void print_mac_address ()
{
	if (mac[0] < 16)
	{
		Serial.print('0');
	}
	Serial.print(mac[0], HEX);
	Serial.print(':');
	if (mac[1] < 16)
	{
		Serial.print('0');
	}
	Serial.print(mac[1], HEX);
	Serial.print(':');
	if (mac[2] < 16)
	{
		Serial.print('0');
	}
	Serial.print(mac[2], HEX);
	Serial.print(':');
	if (mac[3] < 16)
	{
		Serial.print('0');
	}
	Serial.print(mac[3], HEX);
	Serial.print(':');
	if (mac[4] < 16)
	{
		Serial.print('0');
	}
	Serial.print(mac[4], HEX);
	Serial.print(':');
	if (mac[5] < 16)
	{
		Serial.print('0');
	}
	Serial.print(mac[5], HEX);
}

void save_mac_address ()
{
	for (byte i = 0; i < 6; i++)
	{
		EEPROM.write(MAC_START_ADDRESS + i, mac[i]);
	}
}

void generate_random_mac_address ()
{
	set_mac_address(random(0, 255), random(0, 255), random(0, 255), random(0, 255), random(0, 255), random(0, 255));
}

void get_mac_address ()
{
	bool hasNeverBeenSaved = true;
	for (byte i = 0; i < 6; i++)
	{
		mac[i] = EEPROM.read(MAC_START_ADDRESS + i);
		if (mac[i] != 255)  // If a EEPROM cell has never been written to before, it returns 255 (accordoing to Arduino doco)
		{
			hasNeverBeenSaved = false;
		}
	}
	if (hasNeverBeenSaved)
	{
		Serial.print(F("Generating random MAC address: "));
		generate_random_mac_address();
		print_mac_address();
		Serial.println();
	}
}

void set_mac_address (byte octet0, byte octet1, byte octet2, byte octet3, byte octet4, byte octet5)
{
	mac[0] = octet0;
	mac[1] = octet1;
	mac[2] = octet2;
	mac[3] = octet3;
	mac[4] = octet4;
	mac[5] = octet5;
	save_mac_address();
}

/*
Listens for new MAC address over serial or creates random. Warning... here be dragons...
*/
void listen_for_new_mac_address () // blocking operation
{
	
	Serial.print(F("Current MAC address is "));
	print_mac_address();
	Serial.println();

	Serial.println(F("Enter new MAC address 01:23:45:67:89:AB (enter a newline for random MAC address):"));
	byte b;
	byte bi = 0; // Index for octetB
	byte index = 0;
	byte acc = 0;
	byte octetB[] = {0, 0};
	char macOctets[] = {0, 0, 0, 0, 0, 0};
	clear_serial_buffer();
	while (true)
	{
		if (Serial.available())
		{
			b = Serial.read();
			if (b == 10 || b == 13)
			{
				if (index == 0)
				{
					// Nothing entered, use random MAC
					generate_random_mac_address();
					Serial.print(F("New MAC: "));
					print_mac_address();
					Serial.println();
				}
				else if (index == 6)
				{
					// nothing. Should be valid.
				}
				else
				{
					Serial.println(F("Invalid mac address!"));
				}
				break;
			}
			else if ((b >= 48 && b <= 58) || (b >= 97 && b <= 102) || (b >= 65 && b <= 70)) // 0-9: || a-z || A-Z
			{
				if (b >= 48 && b <= 57) // 0-9
				{
					acc *= 16;
					acc += (b - 48);
					bi++;
				}
				else if (b != 58) // (a-f || A-F)
				{
					if (b >= 97) // convert to uppercase
					{
						b -= 32;
					}
					acc *= 16;
					acc += (b - 55);
					bi++;
				}
				if (bi == 2 && index == 5)
				{
					b = 58; // Force b = 58 so that the final octet is added
				}
				else if (bi > 2)
				{
					// Invalid
					Serial.println(F("Invalid mac address!"));
					break;
				}
				
				if (b == 58) // :
				{
					if (bi == 2)
					{
						// Set octet & reset counters
						macOctets[index++] = acc;
						acc = 0;
						bi = 0;
						if (index == 6)
						{
							// Done. Save
							set_mac_address(macOctets[0], macOctets[1], macOctets[2], macOctets[3], macOctets[4], macOctets[5]);
							print_mac_address();
							Serial.println();
							break;
						}
					}
					else if (bi > 2)
					{
						Serial.println(F("Invalid mac address!"));
					}
				}
			}
			else
			{
				// Invalid
				Serial.println(F("Invalid mac address!"));
				break;
			}
		}
	}
	clear_serial_buffer();
}
// IP address
void print_ip_address()
{
	// TODO: Test for saved / actual IP
	Serial.print(clientIpBytes[0], DEC);
	Serial.print('.');
	Serial.print(clientIpBytes[1], DEC);
	Serial.print('.');
	Serial.print(clientIpBytes[2], DEC);
	Serial.print('.');
	Serial.print(clientIpBytes[3], DEC);
}

void save_ip_address()
{
	Serial.println("Saving IP Address...");
	for (byte i = 0; i < 4; i++)
	{
		Serial.println(EEPROM.read(IP_ADDRESS + i));
		Serial.println(clientIpBytes[i]);
		EEPROM.write(IP_ADDRESS + i, clientIpBytes[i]);
		Serial.println(EEPROM.read(IP_ADDRESS + i));
		Serial.println();
	}
}

void load_ip_address ()
{
	Serial.println("Loading IP Address from EEPROM...");
	for (byte i = 0; i < 4; i++)
	{
		Serial.println(EEPROM.read(IP_ADDRESS + i));
		clientIpBytes[i] = EEPROM.read(IP_ADDRESS + i);
	}
}

void listen_for_ip_address()
{
	Serial.print(F("Current ip address: "));
	print_ip_address();
	Serial.println();
	Serial.println(ip);

	Serial.println(F("Enter new ip address (press enter to use DHCP):"));
	
	serial_recieve_index = 0;
	clear_serial_buffer();

	bool keepReading = true;
	while (keepReading)
	{
		while (Serial.available())
		{
			if (Serial.peek() == 13 || Serial.peek() == 10)
			{
				// new line. End entry
				keepReading = false;
				break;
			}
			serial_recieve_data[serial_recieve_index++] = Serial.read();
			if (serial_recieve_index >= SERIAL_RECIEVE_BUFFER_LENGTH)
			{
				// max length. End entry
				keepReading = false;
				break;
			}
		}
	}
	clear_serial_buffer();

	if (serial_recieve_index == 0)
	{
		// Empty, use DHCP
		clientIpBytes[0] = 0;
		clientIpBytes[1] = 0;
		clientIpBytes[2] = 0;
		clientIpBytes[3] = 0;
		save_ip_address();
		Serial.print(F("Using DHCP."));
	}
	else
	{
		boolean validInput = true;
		int octet = 0;
		byte newIp[] = {0, 0, 0, 0};
		byte newIpIndex = 0;
		for (byte i = 0; i < serial_recieve_index; i++)
		{
			if (serial_recieve_data[i] >= 48 && serial_recieve_data[i] <= 57) // 0-9
			{
				octet = octet * 10 + (serial_recieve_data[i] - 48);
				if (octet > 255)
				{
					validInput = false;
					break;
				}
			}
			else if ((char)serial_recieve_data[i] == '.') // TODO: replace with ASCII code for '.'
			{
				newIp[newIpIndex++] = octet;
				octet = 0;
			}
			else if (serial_recieve_data[i] == 10 || serial_recieve_data[i] == 13) // newline
			{
				if (octet != 0 || newIpIndex != 0) // no input parsed yet
				{
					validInput = false;
				}
				else
				{
					newIp[newIpIndex++] = octet;
				}
				break;
			}
			else // invalid char
			{
				validInput = false;
				break;
			}
		}

		if (validInput)
		{
			clientIpBytes[0] = newIp[0];
			clientIpBytes[1] = newIp[1];
			clientIpBytes[2] = newIp[2];
			clientIpBytes[3] = newIp[3];
			save_ip_address();
			Serial.println(F(" Please reset SNARC for changes to take effect."));
		}
		else
		{
			Serial.println(F("Invalid input. IP address not changed."));
		}
	}
}
#endif

// Device name
void print_device_name()
{
	Serial.print(device_name);
}

void save_device_name()
{
	for (byte i = 0; i <= DEVICE_NAME_MAX_LENGTH; i++) // the "<=" is to accomodate the extra null character that's needed to terminate the string.
	{
		EEPROM.write(DEVICE_NAME_ADDRESS + i, (byte)device_name[i]);
	}
}

void load_device_name()
{
	for (byte i = 0; i <= DEVICE_NAME_MAX_LENGTH; i++) // the "<=" is to accomodate the extra null character that's needed to terminate the string.
	{
		device_name[i] = (char)EEPROM.read(DEVICE_NAME_ADDRESS + i);
	}
}

void listen_for_device_name()
{
	Serial.print(F("Current device name is: "));
	print_device_name();
	Serial.println();

	Serial.print(F("Enter new device name (max "));
	Serial.print(DEVICE_NAME_MAX_LENGTH);
	Serial.println(F(" characters):"));
	
	serial_recieve_index = 0;
	clear_serial_buffer();

	bool keepReading = true;
	while (keepReading)
	{
		while (Serial.available())
		{
			if (Serial.peek() == 13 || Serial.peek() == 10)
			{
				// new line. End entry
				keepReading = false;
				break;
			}
			serial_recieve_data[serial_recieve_index++] = Serial.read();
			if (serial_recieve_index >= SERIAL_RECIEVE_BUFFER_LENGTH)
			{
				// max length. End entry
				keepReading = false;
				break;
			}
		}
	}
	clear_serial_buffer();

	if (serial_recieve_index == 0)
	{
		// Empty, do not save.
		Serial.println(F("No input detected. No changes made."));
	}
	else
	{
		// Echo & save new name
		Serial.print(F("Device name set to: "));
		byte i;
		for (i = 0; i < serial_recieve_index; i++)
		{
			device_name[i] = (char)serial_recieve_data[i];
			Serial.print(device_name[i]);
		}
		// Fill rest of device name array with null chars
		for (; i <= DEVICE_NAME_MAX_LENGTH; i++)
		{
			device_name[i] = '\0';
		}
		Serial.println();
		save_device_name();
	}
}

// Stuff under here is still good
/**
 * Start: E-Stop
 */
void eStopInit ()
{
	// Configure E-Stop pin
	pinMode(ESTOP_PIN, INPUT);
	digitalWrite(ESTOP_PIN, HIGH);
	attachInterrupt(ESTOP_INTERRUPT_PIN, eStopInterruptHandler, LOW);
}

void eStopBroadcast ()
{
	// TODO: Implement
}

void eStopInterruptHandler ()
{
	actuator->off();
	lockdown = true;
}

void eStopLockdown ()
{
	detachInterrupt(ESTOP_INTERRUPT_PIN); // disable estop interrupt so repeated estop presses won't cause weird/mistimed LED blinks.
	Serial.println(F("LOCKED DOWN!!!"));
	eStopBroadcast();
	ledOn(RED);
	blinkLed(GREEN);
	while (true)
	{
		// Lockdown loop
		manageLeds();
	}
	attachInterrupt(ESTOP_INTERRUPT_PIN, eStopInterruptHandler, LOW);
}
/**
 * End: E-Stop
 */
