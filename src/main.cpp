#include <DallasTemperature.h>
#include <VirtualWire.h>
#include "LowPower.h"
//#include <Wire.h>
#include <OneWire.h>
int pocet = 0;
int MaxPocetSpanku = 8;      // pocet cyklu spanku, 75 = 10 minut, 37???
String str_adresa;
String str_teplota;
String str_vlhkost;
String str_napeti;
String str_out;
float temperature = 0; 
//// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS A4
//
//// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
//
//// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
//

void setup() { 
  Serial.begin(115200); 
  Serial.println("Setup pro vysílání");
  delay(50);
  // ============  setup pro vysilani ===============
   // nastaveni typu bezdratove komunikace
  vw_set_ptt_inverted(true);
  // nastaveni rychlosti prenosu v bitech za sekundu
  vw_setup(1000);
  // nastaveni datoveho pinu pro vysilac
  vw_set_tx_pin(14);
}

long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

void loop()
{
 // ====== cyklovani spanku =========
 if (pocet > 0)
  {
      if(pocet < MaxPocetSpanku)
      {
        pocet ++;
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
      }
      else
      {
        pocet = 0;
      }
  }
  else
  {
 // =================================  
 float humidity = 0;
 //Serial.print("Requesting temperatures...");
 sensors.requestTemperatures(); // Send the command to get temperatures
 //Serial.print("Temperature for the device 1 (index 0) is: ");
 //Serial.println(sensors.getTempCByIndex(0));   
 temperature = sensors.getTempCByIndex(0);
 int h = 0;
 int t = (int)(temperature*10);
 //Priprava stringu pro odeslani
 long vcc;
 str_adresa = "1";
 str_teplota = String(t);
 str_vlhkost = String(h);
 vcc = readVcc();
 str_napeti = String(vcc);
 str_out = str_adresa + "," + str_teplota + "," + str_vlhkost + "," + str_napeti;
 Serial.print("Odesílám:"); 
 static char *msg = str_out.c_str(); 
 Serial.println(msg);
 // Odeslani zpravy
 vw_send((uint8_t *)msg, strlen(msg));  
 vw_wait_tx();
 //Serial.println("vypinam napajeni");
 //Uspání modulu
 pocet++;
 delay(2500);
 LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
}
