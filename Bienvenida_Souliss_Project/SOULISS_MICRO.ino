/**************************************************************************
    Souliss - Garage Door

    Control a garage door using two Ethernet boards: one device act on the
    relays that drive the motor and get the limit switches, the other has one
    push-button for opening and closing the door. The door can be controlled
    also via Android (download SoulissApp from Play Store).

  Connect the boards via USART crossing TX or RX, or through an RS485 transceiver.

    Ensure to use limit switches to protect the motor once the door is completely
    closed or open, if limit switches are not used the motor should be self-protected.

    Run this code on one of the following boards:
      - Arduino with RS485 transceiver

***************************************************************************/

// Configure the framework
#include "bconf/StandardArduino.h"          // Use a standard Arduino
#include "conf/usart.h"                     // USART RS485

/*************/
// Use the following if you are using an RS485 transceiver with
// transmission enable pin, otherwise delete this section.
//
#define USARTDRIVER_INSKETCH
#define USART_TXENABLE          1
#define USART_TXENPIN           12
#define USARTDRIVER             Serial1

#define USARTBAUDRATE_INSKETCH
#define USART_BAUD9k6        0
#define USART_BAUD19k2       0
#define USART_BAUD57k6       0
#define USART_BAUD115k2      1
#define USART_BAUD256k       0
/*************/

// Include framework code and libraries
#include <SPI.h>

/*** All configuration includes should be above this line ***/
#include "Souliss.h"

// Define the RS485 network configuration
#define myvNet_subnet   0xFF00
#define Gateway_RS485   0xD001
#define Peer_RS485      0xD002

#define ANTIROBO          0
#define PERROGUARDIAN     1
#define LEDCONTROL        2
#define LEDRED            3
#define LEDGREEN          4
#define LEDBLUE           5
#define TEMPERATURA0      6
#define TEMPERATURA1      8
#define TEMPERATURA2      10
#define TEMPERATURA3      12
#define TEMPERATURA4      14
#define TEMPERATURA5      16

void setup()
{
    Initialize();

    // Set network parameters
    SetAddress(Peer_RS485, myvNet_subnet, Gateway_RS485);

    // Set the typical logic to handle the garage door and light
   Set_LED_Strip(LEDCONTROL);
   Set_T52(TEMPERATURA0);
   Set_T52(TEMPERATURA1);
   Set_T52(TEMPERATURA2);
   Set_T52(TEMPERATURA3);
   Set_T52(TEMPERATURA4);
   Set_T52(TEMPERATURA5);
    // Define inputs, outputs pins

    pinMode(11, OUTPUT);                 // Power the RLED
    pinMode(10, OUTPUT);                 // Power the GLED
    pinMode(9, OUTPUT);                 // Power the BLED
    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
    pinMode(A3, INPUT);
    pinMode(A4, INPUT);
    pinMode(A5, INPUT);

}

void loop()
{
    // Here we start to play
    EXECUTEFAST() {
        UPDATEFAST();

        // Execute every 510ms the logic, the command to open and close the garage door
        // cames directly from SoulissApp or the push-button located on the other node
        FAST_10ms() {

            Logic_LED_Strip(LEDCONTROL);
            // Use the output values to control the PWM
            analogWrite(11, mOutput(LEDRED));
            analogWrite(10, mOutput(LEDGREEN));
            analogWrite(9, mOutput(LEDBLUE));
            ProcessCommunication();
        }
        if (!(phase_fast % 411)){ // FAST_4110ms()
                  float scaleFactor = 3.20/1023.00*100;
                  AnalogIn(A0,TEMPERATURA0,scaleFactor,0);
                  Logic_T52(TEMPERATURA0);

                  AnalogIn(A1,TEMPERATURA1,scaleFactor,0);
                  Logic_T52(TEMPERATURA1);

                  AnalogIn(A2,TEMPERATURA2,scaleFactor,0);
                  Logic_T52(TEMPERATURA2);

                  AnalogIn(A3,TEMPERATURA3,scaleFactor,0);
                  Logic_T52(TEMPERATURA3);

                  AnalogIn(A4,TEMPERATURA4,scaleFactor,0);
                  Logic_T52(TEMPERATURA4);

                  AnalogIn(A5,TEMPERATURA5,scaleFactor,0);
                  Logic_T52(TEMPERATURA5);

                  ProcessCommunication();
                  //Test_serialPrintLatches();
                }
        FAST_2110ms()   {

      // Build a watchdog chain to monitor the nodes
      mInput(ANTIROBO) = Watchdog(Gateway_RS485, PERROGUARDIAN, Souliss_T4n_Alarm);
    }

        // Process the communication, this include the command that are coming from SoulissApp
        // or from the push-button located on the other node
        FAST_PeerComms();
        START_PeerJoin();
    }

    EXECUTESLOW() {
        UPDATESLOW();
        SLOW_10s()  {

          // The timer handle timed-on states
            Timer_LED_Strip(LEDCONTROL);
        }
    }

}

