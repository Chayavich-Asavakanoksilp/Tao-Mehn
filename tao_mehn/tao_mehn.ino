#define BLYNK_TEMPLATE_ID "TMPL6tJ0Bb5Rh"
#define BLYNK_TEMPLATE_NAME "Tao mehn"
#define BLYNK_AUTH_TOKEN "1So1uzHMivCxXuIwhWnZIsfHyK_y009z"
#define BLYNK_PRINT Serial

// include libaries
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <cmath>
#include <FastLED.h>
#include <HX711.h> //https://github.com/bogde/HX711

#include "configuration.h"


//skip======================================================

// initialize variable
BlynkTimer timer;                 // blynk simple timer class
HX711 scale(LOAD_DOUT, LOAD_CLK); // hx711 library class
CRGB leds[3];
int level = 2;
enum motor_mode { cw = 1,
                  x = 0,
                  ccw = -1 };
enum motor_mode mode;
bool ready;
int linear_time = 4000;           // time for sudo linear actuator
bool up = true;                  // check if mixing motor is already in position

// function for calculate weight in gram
float scale_g() {
  return (scale.get_units(3) * 0.453592 * 1000) + offset;
}

// function for delay which not delay the device
bool ddelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms);
  return true;
}

// mixing the drink
void blade() {
  digitalWrite(BLADE_PIN, HIGH);
  ddelay(10000);
  digitalWrite(BLADE_PIN, LOW);
}

// function to add syrup in grams
void syrup(float order_syrup) {
  // open pump until the syrup reach configue weight at order_syrup grams
  Serial.println("syrup");

  int i = 0;
  float weight;
  while (i <= 2) {
    weight = scale_g();
    Blynk.virtualWrite(V2, weight);
    if (weight >= order_syrup) i++;
    else {
      i = 0;
      digitalWrite(VALVE_PIN, HIGH);
      ddelay(500);
      digitalWrite(VALVE_PIN, LOW);
    }
    ddelay(500);

    if (debug_mode) {
      Serial.print(i);
      Serial.print("\t");
      Serial.println(weight);}
  };
}

// function to fill the water
void fill() {
  Serial.println("fill");
  // open pump until the syrup reach configue weight at fill_weight grams
  digitalWrite(PUMP_PIN, HIGH);
  int i = 0;
  float weight;
  while (i <= 5) {
    weight = scale_g();
    if (weight >= fill_weight) {
      i++;
    } else {
      i = 0;
    }
    Blynk.virtualWrite(V2, weight);

    if (debug_mode) {
      Serial.print(i);
      Serial.print("\t");
      Serial.println(weight);}
  };
  digitalWrite(PUMP_PIN, LOW);
}

// funtion for control direction of motor with two relay
void motor_ctrl(enum motor_mode mode) {
  switch (mode) {
    case 0:
      digitalWrite(MOTOR_PIN_A, LOW);
      digitalWrite(MOTOR_PIN_B, LOW);
      break;

    case 1:
      digitalWrite(MOTOR_PIN_A, HIGH);
      digitalWrite(MOTOR_PIN_B, LOW);
      break;

    case -1:
      digitalWrite(MOTOR_PIN_A, LOW);
      digitalWrite(MOTOR_PIN_B, HIGH);
      break;
  }
}

// function for mixing
void mix() {

  motor_ctrl(ccw);
  ddelay(linear_time);
  motor_ctrl(x);
  up = false;

  blade();

  motor_ctrl(cw);
  ddelay(linear_time);
  motor_ctrl(x);
  up = true;
}

// show weight
void read_weight() {
  Blynk.virtualWrite(V2, scale_g());
}

// alwalys check connection from blynk and try to reconnect
void check_connection() {
  if (!Blynk.connected()) {
    leds[1] = CRGB(255, 255, 0);
    FastLED.show();
    bool res = Blynk.connect();
    if (Blynk.connect()) {
      leds[1] = CRGB(0, 255, 0);
      FastLED.show();
      Blynk.virtualWrite(V3, 0);
    }
  }
}

//skip======================================================

// recive order
BLYNK_WRITE(V1) {
  level = param.asInt();
  switch (level) {
    case 1:
      Blynk.virtualWrite(V3, 21);
      Serial.println("sweet: 1");
      ddelay(500);
      Blynk.virtualWrite(V3, 0);
      break;

    case 2:
      Blynk.virtualWrite(V3, 22);
      Serial.println("sweet: 2");
      ddelay(500);
      Blynk.virtualWrite(V3, 0);
      break;

    case 3:
      Blynk.virtualWrite(V3, 23);
      Serial.println("sweet: 3");
      ddelay(500);
      Blynk.virtualWrite(V3, 0);
      break;
  }
}

// show the status at blynk
BLYNK_WRITE(V0) {
  if (param.asInt() && ready) {
    ready = false;
    int order_level = level;
    float order_syrup = syrup_weight[order_level];
    scale.tare(5);
    float db1 = scale_g();

    Blynk.virtualWrite(V3, 11);
    syrup(order_syrup);
    Serial.println("added syrup");
    float db2 = scale_g();
    if (debug_mode)ddelay(5000);

    Blynk.virtualWrite(V3, 12);
    fill();
    Serial.println("filled water");
    float db3 = scale_g();

    Blynk.virtualWrite(V3, 13);
    mix();
    Serial.println("mixed");

    Blynk.virtualWrite(V3, 3);
    Serial.println("serve");
    float db4 = scale_g();
    while (scale_g() >= 20) { Blynk.virtualWrite(V2, scale_g()); }

    Blynk.virtualWrite(V3, 0);
    Blynk.virtualWrite(V0, 0);
    ready = true;
    Serial.println("ready");

    if (debug_mode){
      Serial.println("--summary--");
      Serial.print("level\t");
      Serial.println(order_syrup);
      Serial.print("start\t");
      Serial.println(db1);
      Serial.print("syrup\t");
      Serial.println(db2);
      Serial.print("filled\t");
      Serial.println(db3);
      Serial.print("end\t");
      Serial.println(db4);
      Serial.println("=================");
    }

  } else {
    Blynk.virtualWrite(V3, 4);
    Serial.println("waittt");
    Blynk.virtualWrite(V0, 1);
  }
}

// initialize blynk
BLYNK_CONNECTED() {
  Blynk.virtualWrite(V3, -1);
  leds[1] = CRGB(0, 255, 0);
  FastLED.show();
  timer.setInterval(60000, check_connection);
  if (!up) {
    motor_ctrl(x);
    motor_ctrl(cw);
    ddelay(linear_time);
    motor_ctrl(x);
    up = true;
  }

  Blynk.virtualWrite(V0, 0);
  Blynk.virtualWrite(V1, 2);

  scale.set_scale(calibration_factor);
  scale.set_offset(zero_factor);
  scale.tare(5);
  ddelay(1000);
  ready = true;
  Blynk.virtualWrite(V3, 0);

  leds[2] = CRGB(0, 255, 0);
  FastLED.show();
}

void setup() {
  // Set pin mode for all relay
  pinMode(VALVE_PIN,   OUTPUT);
  pinMode(PUMP_PIN,    OUTPUT);
  pinMode(MOTOR_PIN_A, OUTPUT);
  pinMode(MOTOR_PIN_B, OUTPUT);
  pinMode(BLADE_PIN,   OUTPUT);

  // Set input for manual overide debugger pin
  pinMode(DEBUG_U, INPUT_PULLUP);
  pinMode(DEBUG_D, INPUT_PULLUP);
  pinMode(DEBUG_B, INPUT_PULLUP);
  pinMode(DEBUG_P, INPUT_PULLUP);
  pinMode(DEBUG_V, INPUT_PULLUP);

  FastLED.addLeds<WS2811, DEBUG_L, GRB>(leds, 3).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(10);
  leds[0] = CRGB(0, 255, 0);
  leds[1] = CRGB(255, 0, 0);
  leds[2] = CRGB(255, 0, 0);
  FastLED.show();

  Serial.begin(9600);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  timer.setInterval(500, read_weight);
  Blynk.virtualWrite(V3, 0);

  mode = x;
}

void loop() {
  timer.run();
  Blynk.run();
  if (digitalRead(DEBUG_U) == LOW) {
    motor_ctrl(cw);
    ddelay(500);
    motor_ctrl(x);
  }
  if (digitalRead(DEBUG_D) == LOW) {
    motor_ctrl(ccw);
    ddelay(500);
    motor_ctrl(x);
  }
  if (digitalRead(DEBUG_B) == LOW) {
    blade();
  }
  if (digitalRead(DEBUG_P) == LOW) {
    digitalWrite(PUMP_PIN, HIGH);
    ddelay(1000);
    digitalWrite(PUMP_PIN, LOW);
  }
  if (digitalRead(DEBUG_V) == LOW) {
    digitalWrite(VALVE_PIN, HIGH);
    ddelay(1000);
    digitalWrite(VALVE_PIN, LOW);
  }
}
