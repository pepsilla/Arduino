// Defines use of UART communications
#define USARTDRIVER_INSKETCH
#define USART_TXENABLE          1
#define USART_TXENPIN           16
#define USARTDRIVER             Serial

#define USARTBAUDRATE_INSKETCH
#define USART_BAUD9k6        0
#define USART_BAUD19k2       0
#define USART_BAUD57k6       0
#define USART_BAUD115k2      1
#define USART_BAUD256k       0

// **** Define the WiFi name and password ****
#define WIFICONF_INSKETCH
#define WiFi_SSID      "MyAccessPoint"
#define WiFi_Password  "MyPasswordEntry"

// Define SLOTS in this node
#define ANTIROBO        0
#define PERROGUARDIAN   1

#define TEMPERATURA     2
#define SLOT_PIR        4
#define SLOT_PUERTA     5
#define SLOT_TAMPER     6
#define SLOT_RESET_SONERIA 7

#define SLOT_SONERIA    8
#define SLOT_PRESENCIA  9
#define SLOT_OUTPUT1    10
#define SLOT_OUTPUT2    11

//Defines address and slots of peer RGB controller.
#define peerAddress 0xD002  //Remote peer address and slots-
#define LEDCONTROL        2
#define LEDRED            3
#define LEDGREEN          4
#define LEDBLUE           5

// Define I2C pins for Wire.begin on ESP8266 core.
#define scl_pin 5
#define sda_pin 4

//Define human references for port bits in PCF8574
// INPUTS
#define IN1_BIT 4
#define IN2_BIT 5
#define IN3_BIT 6
#define PIR_BIT 7
// OUTPUTS
#define RELAY1_BIT 0
#define RELAY2_BIT 1
#define OUT1_BIT 2
#define OUT2_BIT 3

//Backlight of display I2C
#define BUILTIN_LED	2
#define WELLCOME_TIME 9

// All includes
#include "bconf/MCU_ESP8266.h" // Load the code directly on the ESP8266
//#include "conf/SuperNode.h"   // The main node is the Gateway, we have just one node
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
#include <I2CRX2002ABIWTS.h>

//Global declarations


I2CPCF8574 pinExpander(0x20,B11110000);
I2CRX2002ABIWTS displayI2c;

//Definitions of colors for RGB stip on peer
U8 COLOR_RED[4] = {Souliss_T1n_Set,254,0,0};
U8 COLOR_GREEN[4] = {Souliss_T1n_Set,0,254,0};
U8 COLOR_BLUE[4] = {Souliss_T1n_Set,0,0,254};
U8 COLOR_WHITE[4] = {Souliss_T1n_Set,254,254,254};



// First Run, initialize hardware.
void setup()
{
    Initialize();
    //Serial.begin(115200);
    Wire.begin(sda_pin,scl_pin);

    //Init display I2C
    displayI2c.init();
    delay(60);
    displayI2c.printLine1("- WellcomeToEcomred!");
    displayI2c.printLine2(" v.20160813@pepsilla");
    //Backlight del display
    pinMode(BUILTIN_LED,OUTPUT);
    analogWriteFreq(250);
    analogWriteRange(255);
    analogWrite(BUILTIN_LED,21);
    //Init pinExpander PCF8574
    pinExpander.initPort();


    // Get the IP address from DHCP
    GetIPAddress();
    SetAsGateway(myvNet_dhcp); // Set this node as gateway for SoulissApp
    //Pending print on line 1 of display IP address...
    //displayI2c.printLine1(Wifi.localIP())

    // This is the vNet address for this node, used to communicate with other
    // nodes in your Souliss network
    SetAddress(0xAB01, 0xFF00, 0x0000);
    SetAddress(0xD001, 0xFF00, 0x0000);
    SetAsPeerNode(peerAddress, 1);




    // Set Logics
    Set_T41(ANTIROBO);
    Set_T52(TEMPERATURA); //Temperature sensor LM35

    Set_T13(SLOT_PIR); //Pir sensor
    Set_T13(SLOT_PUERTA); //Door open
    Set_T13(SLOT_TAMPER);//Tamper detection
    Set_T13(SLOT_RESET_SONERIA); // Reset siren

    Set_T13(SLOT_SONERIA); // Buzzer Relay - RELAY1_BIT
    Set_AutoLight(SLOT_PRESENCIA); //Pir, presence light - RELAY2_BIT.
    Set_SimpleLight(SLOT_OUTPUT1); // Internal buzzer.
    Set_SimpleLight(SLOT_OUTPUT2); // Internal siren.

    //Set SLOT_PRESENCIA in automode defaults
    mInput(SLOT_PRESENCIA) = Souliss_T1n_AutoCmd;

}


// Main Loop program
void loop()
{
    // Here we start to play

    EXECUTEFAST() {
        UPDATEFAST();
        FAST_50ms() {
        //Process inputs logics
          pinExpander.readPort();//Read pinPort of PCF8574

          if(pinExpander.getPinPort(PIR_BIT))mInput(SLOT_PIR) = Souliss_T1n_OffCmd;
          else mInput(SLOT_PIR) = Souliss_T1n_OnCmd;

          if(pinExpander.getPinPort(IN1_BIT))mInput(SLOT_PUERTA) = Souliss_T1n_OffCmd;
          else mInput(SLOT_PUERTA) = Souliss_T1n_OnCmd;

          if(pinExpander.getPinPort(IN2_BIT))mInput(SLOT_TAMPER) = Souliss_T1n_OffCmd;
          else mInput(SLOT_TAMPER) = Souliss_T1n_OnCmd;

          if(pinExpander.getPinPort(IN3_BIT)) mInput(SLOT_RESET_SONERIA) = Souliss_T1n_OffCmd;
          else mInput(SLOT_RESET_SONERIA) = Souliss_T1n_OnCmd;




          Logic_T13(SLOT_RESET_SONERIA);

          //Process input to output logics
          	//Movement detection
          	if(Logic_T13(SLOT_PIR)){
				if(mOutput(SLOT_PIR) == Souliss_T1n_OffCoil && mOutput(ANTIROBO) == Souliss_T4n_Antitheft)
					mInput(ANTIROBO) = Souliss_T4n_Alarm; // Sensor pir == 1 La alarma está activada
				else {
					if(mOutput(SLOT_PIR) == Souliss_T1n_OffCoil){
						mInput(SLOT_PRESENCIA) = Souliss_T1n_Timed + WELLCOME_TIME; //La alarma no esta activada, Luz de cortesia. 90 SEGUNDOS (9 CICLOS DE 10 SEGUNDOS)
						displayI2c.printLine2("* MOVIMIENTO DETECT.");
					}
					if(mOutput(SLOT_PIR) == Souliss_T1n_OnCoil)
						displayI2c.printLine2("* SIN ACTIVIDAD DET.");
				}

          	}
            //Door change status detection
            if(Logic_T13(SLOT_PUERTA)){
				if(mOutput(SLOT_PUERTA) == Souliss_T1n_OffCoil && mOutput(ANTIROBO) == Souliss_T4n_Antitheft)
				  mInput(ANTIROBO) = Souliss_T4n_Alarm; // La alarma está activada. Puerta abierta == 1
				else{
				  if((mOutput(SLOT_PUERTA) == Souliss_T1n_OffCoil) && (mOutput(SLOT_SONERIA) == Souliss_T1n_OffCoil)){
					//La alarma no esta activada, la puerta ha cambiado a abierto: encender RGB de puerta.
					SendData(peerAddress,LEDCONTROL,COLOR_WHITE,4);//Enciende a blanco la tira RGB
					displayI2c.printLine2("* PUERTA ABIERTA ***");
				 }
				 if((mOutput(SLOT_PUERTA) == Souliss_T1n_OnCoil) && (mOutput(SLOT_SONERIA) == Souliss_T1n_OffCoil)){
				 	//La alarma no esta activada, La puerta ha cambiado a cerrado. Apagar la tira RGB.
				 	Send(peerAddress,LEDCONTROL,Souliss_T1n_OffCmd);//Apaga la tira RGB
				 	displayI2c.printLine2("* PUERTA CERRADA ***");
				 }
				}
            }

            //Tamper detection
            if(Logic_T13(SLOT_TAMPER)){
				if(pinExpander.getPinPort(IN2_BIT)){
				  // Tamper == 1
				  if(mOutput(ANTIROBO) == Souliss_T4n_Antitheft){
					// La alarma está activada
					mInput(ANTIROBO) = Souliss_T4n_Alarm;
				  }
				  if(mOutput(ANTIROBO) == Souliss_T4n_NoAntitheft){
					displayI2c.printLine2("* TAMPER ACTIVADO **");
					pinExpander.setPinPort(RELAY1_BIT, 0);
				  }
				}
				else{
					if(mOutput(ANTIROBO) == Souliss_T4n_NoAntitheft){
						displayI2c.printLine2("* TAMPER DESACTIVADO");
					}
				}

            }

            //Update RELAY1 status on SLOT_SONERIA
            if(pinExpander.getPinPort(RELAY1_BIT))
            	mInput(SLOT_SONERIA) = Souliss_T1n_OffCmd;
            else{
            	mInput(SLOT_SONERIA) = Souliss_T1n_OnCmd;

            }



            if(Logic_T13(SLOT_SONERIA)){
            	if(mOutput(SLOT_SONERIA) == Souliss_T1n_OnCoil)SendData(peerAddress,LEDCONTROL,COLOR_RED,4); //Enciende a rojo la tira led RGB
            	else Send(peerAddress,LEDCONTROL,Souliss_T1n_OffCmd);
            }



            //Update RELAY2 on SLOT_PRESENCIA STATUS
            if(Logic_AutoLight(SLOT_PRESENCIA)){
				if(mOutput(SLOT_PRESENCIA) & Souliss_T1n_Coil){
					pinExpander.setPinPort(RELAY2_BIT, 0);
					analogWrite(BUILTIN_LED,254);
				}
				else{
					pinExpander.setPinPort(RELAY2_BIT, 1);
					analogWrite(BUILTIN_LED,26);
				}
            }

            data_changed = 0;//Reset changes in data.
            Logic_T41(ANTIROBO);
            if(data_changed){
				//Update RELAY1 on Anti-theft detection. Reset to disarm
				if(mOutput(ANTIROBO) == Souliss_T4n_InAlarm){
				  // La alarma está disparada-
					pinExpander.setPinPort(RELAY1_BIT,0);
					displayI2c.printLine1("# ALARMA ACTIVADA ##");
				}
				else if(mOutput(ANTIROBO) == Souliss_T4n_Antitheft){
					displayI2c.printLine1("# ESCUDO ACTIVADO ##");
				}
				else if (mOutput(ANTIROBO) == Souliss_T4n_NoAntitheft){
					displayI2c.printLine1("# ESCUDO DESACTIVADO");
				}
			}

            Logic_SimpleLight(SLOT_OUTPUT1);
            Logic_SimpleLight(SLOT_OUTPUT2);

            //Update two auxiliar outputs.
            if(mOutput(SLOT_OUTPUT1))
            	pinExpander.setPinPort(OUT1_BIT, 0);
            else
            	pinExpander.setPinPort(OUT1_BIT, 1);

            if(mOutput(SLOT_OUTPUT2))
            	pinExpander.setPinPort(OUT2_BIT, 0);
            else
            	pinExpander.setPinPort(OUT2_BIT, 1);

            //Write changes on pinExpander
            pinExpander.writePort();

            ProcessCommunication();

        }


        //Every 250ms: Process analogics inputs.
        if (!(phase_fast % 1110)){ // FAST_250ms()
          float scaleFactor = 3.20/1023.00*100;
          AnalogIn(A0,TEMPERATURA,scaleFactor,0);
          Logic_T52(TEMPERATURA);

          ProcessCommunication();
        }

        FAST_2110ms()   {

			// Build a watchdog chain to monitor the nodes
			mInput(ANTIROBO) = Watchdog(peerAddress, PERROGUARDIAN, Souliss_T4n_Alarm);
		}

        // Process the other Gateway stuffs
        FAST_GatewayComms();
    }
    EXECUTESLOW()
    {
        UPDATESLOW();
        //Refresh timer to off
        SLOW_10s()  {
          Timer_T12(SLOT_PRESENCIA);
          Timer_T41(ANTIROBO);
        }
    }
}
