/* ------------------BLYNK INFORMATION--------------------
Data Stream
V0 = Order button
V1 = Syrup level
V2 = Show weight gage
V3 = Show machine status with enum

GUI Web dashboard
░░░░░░░░░░░░░░░░░░░░░---------------------------------------------░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒░░░░░░░
░Syrup level░░░░░░░░░|▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒|░░▒▒▒▒░░░░░░░░░░░░░░░▒▒░░░░░▒▒░░░░
░- O====== +░░░░░░░░░|▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒|░░░░░░▒▒▒▒▒▒░░░░░░░▒░░░░░░░░░░░▒░░
░░░░░░░░░░░░░░░░░░░░░|▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒STATUS▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒|░░░░░▒░░░░░░▒░░░░░▒░░░░░LED░░░░░▒░
░░░░░░░░░░░░░░░░░░░░░|▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒|░░░░▒░weight░▒░░░░▒░░░░ORDER░░░░▒░
░░/=====\░░░░░░░░░░░░|▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒|░░░░▒░░░░░░░░▒░░░░░▒░░░░░░░░░░░▒░░
░|░order░|░░░░░░░░░░░|▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒|░░░░░░░░░░░░░░░░░░░░▒▒░░░░░░░▒▒░░░
░░\=====/░░░░░░░░░░░░---------------------------------------------░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒▒░░░░░░
1. Adjust syrup level 1 to 3
  1.1. 1 is sweetless
  1.2. 2 is normal
  1.3. 3 is very sweet (diabetes is waiting)
2. Push order switch to ordered drink
3. Wait until order switch turn off
4. Get the drink!

FEATURE
- can read real time weight and show at weight guage
- show LED ORDER when the machine is working
- show status of the machine

 ------------------BOARD INFORMATION--------------------
 DEBUG LED: Pin 18
 RED = not Ready, Green = Ready, Yellow = Fixing
 LED 1 = BOARD ON
 LED 2 = Blynk connection
 LED 3 = MACHINE
 ---------------------------------------------------------*/
#include <Arduino.h>
// ------------------definable section--------------------
// define pin
#define VALVE_PIN 15   // pin for relay which control syrup pump
#define PUMP_PIN 13    // pin for relay which control water pump
#define MOTOR_PIN_A 2  // pin for relay which control motor direction A
#define MOTOR_PIN_B 5  // pin for relay which control motor direction A
#define BLADE_PIN 12   // pin for relay which control motor with mixing blade
#define LOAD_DOUT 17   // pin for DOUT from HX711
#define LOAD_CLK 16    // pin for CLK from HX711

// load cell calibration
float calibration_factor = 360408;
long zero_factor = 8204305;
float offset = 0;

// weight define    level    1   2    3
float syrup_weight[4] = { 0, 25, 50, 75 };  //gram
float fill_weight = 200;                     //gram

// NETWORK
char ssid[] = "True-WiFi";  // WIFI SSID
char pass[] = "ainaheee";   // WIFI Password


/*===================!!!DEBUG SECTION!!!===================*/ 
#define DEBUG_U 23      // manual override motor to go up
#define DEBUG_D 18      // manual override motor to go down
#define DEBUG_B 19      // manual override motor to spin blade
#define DEBUG_L 0       // status led
#define DEBUG_P 21      // manual override water pump
#define DEBUG_V 22      // manual override syrup valve
bool debug_mode = false; // turn on debug mode
/*===================!!!DEBUG SECTION!!!===================*/ 