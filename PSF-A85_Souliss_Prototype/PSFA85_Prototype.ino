/*
* Preliminary test for develop SOULISS prototype
* 20160921@pepsilla
*/
#include "Arduino.h"
// All includes
#include "bconf/MCU_ESP8266.h" // Load the code directly on the ESP8266
#include "conf/SuperNode.h"   // The main node is the Gateway, we have just one node
#include "conf/usart.h"       // USART
#include "conf/IPBroadcast.h"
#include "conf/Gateway.h"
// Include framework code and libraries
#include "Souliss.h"
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Wire.h>
#include <I2CPCF8574.h>
#include <I2CPCF8591.h>

// Defines use of UART communications
# define USARTDRIVER_INSKETCH
#	define USART_TXENABLE          1
#	define USART_TXENPIN           15
#	define USARTDRIVER             Serial

# define USARTBAUDRATE_INSKETCH
#	define USART_BAUD9k6        0
#	define USART_BAUD19k2       0
#	define USART_BAUD57k6       0
#	define USART_BAUD115k2      1
#	define USART_BAUD256k       0

#define SERIALPORT_INSKETCH
#define LOG	Serial

// **** Define the WiFi name and password ****
# define WIFICONF_INSKETCH

#	define WiFi_Infrastracture		1
#	define WiFi_AdHoc				0

// Network protection type
#	define	WiFi_OpenNetwork		0
#	define 	WiFi_WEP				0					// Please avoid the use of WEP
#	define 	WiFi_WPA				0
#	define	WiFi_WPA2				1

#	define WiFi_SSID      "Your SSID acess point"
#	define WiFi_Password  "Your password for access it"


// Define I2C pins for Wire.begin on ESP8266 core.
#define scl_pin 5
#define sda_pin 4

//Define I2C chip specific parameters
#define PCF8574_APPMASK B11110000
#define PCF8591_APPMODE B11110000

//Defines address and slots of peer RGB controller.
#define peerAddress 0xD002  //Remote peer address and slots-
#define LEDCONTROL        2
#define LEDRED            3
#define LEDGREEN          4
#define LEDBLUE           5

//Define local slots
#define ANTIROBO        0
#define PERROGUARDIAN   1
#define LDR		2

//Global declarations

I2CPCF8574 digitalExpander(0x20, PCF8574_APPMASK);
I2CPCF8591 analogExpander(0x48, PCF8591_APPMODE);

void i2cScann();

//The setup function is called once at startup of the sketch
void setup()
{
// Add your initialization code here

	 Initialize();

	 Wire.begin(sda_pin,scl_pin);
	 //For log and debug operations
	 //Serial1.begin(115200);
	 i2cScann();

	 delay(4000);
	 digitalExpander.initPort();
	 analogExpander.initPort();

	    // Get the IP address from DHCP
	    GetIPAddress();
	    SetAsGateway(myvNet_dhcp); // Set this node as gateway for SoulissApp
	    //Pending print on line 1 of display IP address...
	    //displayI2c.printLine1(Wifi.localIP())

	    // This is the vNet address for this node, used to communicate with other
	    // nodes in your Souliss network
	    SetAddress(0xAB11, 0xFF00, 0x0000);
	    SetAddress(0xD011, 0xFF00, 0x0000);
	    SetAsPeerNode(peerAddress, 1);

	    // Set Logics
		Set_T41(ANTIROBO);
		Set_T54(LDR); //Temperature sensor LM35
}

// The loop function is called in an endless loop
void loop()
{
//Add your repeated code here
	EXECUTEFAST() {
		UPDATEFAST();
		FAST_50ms() {
			//Uptate Expander Values
			digitalExpander.readPort();
			analogExpander.readPort();
			//Process inputs logics
			Logic_T41(ANTIROBO);
			analogExpander.setAnalogValue(127);

		}


		//Every 250ms: Process analogics inputs.
		if (!(phase_fast % 1110)){ // FAST_250ms()
		  float scaleFactor = 3.30-(3.30/1023.00);
		  AnalogIn(A0,LDR,scaleFactor,0);
		  Logic_T54(LDR);
		}

		FAST_2110ms()   {
			// Build a watchdog chain to monitor the nodes
			mInput(ANTIROBO) = Watchdog(peerAddress, PERROGUARDIAN, Souliss_T4n_Alarm);
		}

		// Process the other Gateway stuffs
		FAST_GatewayComms();
		FAST_PeerComms();
	}
	EXECUTESLOW()
	{
		UPDATESLOW();
		//Refresh timer to off
		SLOW_10s()  {
			Timer_T41(ANTIROBO);
		}
	}
}

void i2cScann(){
	byte error, address;
	  int nDevices;

	  LOG.println("Scanning...");

	  nDevices = 0;
	  for(address = 1; address < 127; address++ )
	  {
	    // The i2c_scanner uses the return value of
	    // the Write.endTransmisstion to see if
	    // a device did acknowledge to the address.
	    Wire.beginTransmission(address);
	    error = Wire.endTransmission();

	    if (error == 0)
	    {
	      LOG.print("I2C device found at address 0x");
	      if (address<16)
	    	  LOG.print("0");
	      LOG.print(address,HEX);
	      LOG.println("  !");

	      nDevices++;
	    }
	    else if (error==4)
	    {
	      LOG.print("Unknow error at address 0x");
	      if (address<16)
	    	  LOG.print("0");
	      LOG.println(address,HEX);
	    }
	  }
	  if (nDevices == 0)
		  LOG.println("No I2C devices found\n");
	  else
		  LOG.println("done\n");
}
