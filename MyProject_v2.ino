#include "Adafruit_VL53L0X.h"
#include <M5Stack.h>
#include <Wire.h>

#define TCAADDR 0x70

Adafruit_VL53L0X lox = Adafruit_VL53L0X();
int tof_front=0,tof_top=0;
bool status_tof=false,status_pir = false;

const int motor_pin = 23;
const int motor_pin2 = 18;
int freq=3500,ledChannel=0,ledChannel2=1,resolution=8; // ledChannel for tof_front, ledChannel2 for tof_top


void tcaselect(uint8_t i) {
  if (i > 7) return;
 
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}

void setup() {
  // put your setup code here, to run once:
  M5.begin();
  Wire.begin();
  
  Serial.begin(115200);
  pinMode(35, INPUT);

  ledcSetup(ledChannel, freq, resolution); // set up motor
  ledcSetup(ledChannel2, freq, resolution); // set up motor
  ledcAttachPin(motor_pin, ledChannel);
  ledcAttachPin(motor_pin2, ledChannel2);
  dacWrite (25,0);
  
  tcaselect(5); // //tof top
  Serial.println("Start Program");
  if (!lox.begin()) 
  {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }
  tcaselect(4); // tof front
  if (!lox.begin()) 
  {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }
}

void loop() {
  ledcWrite(ledChannel, 0);
  VL53L0X_RangingMeasurementData_t measure;
  tcaselect(4); // tof front
  lox.rangingTest(&measure, false); 
  tof_front = measure.RangeMilliMeter;
  Serial.print("L1 (Front): "); 
  Serial.print(tof_front);
  Serial.println(" ");
  if(measure.RangeStatus != 4 && tof_front>=400 && tof_front<=1500) // detect from front but dont know what is it
  {
    status_tof = true;
  }
  else
  {
    status_tof = false;
  }

  if(digitalRead(35)==1) // pir detect found something has alive
  {
    status_pir = true;
  }
  else
  {
    status_pir = false;
  }
  if(status_pir==true && status_tof==true) // found alive
  {
    Serial.println("Found Alive");
    for(int dutyCycle = 0; dutyCycle <= 125; dutyCycle+=5) // 35,50 to use
    {   
      ledcWrite(ledChannel, dutyCycle);
      delay(20);  // 50 ,100 , 250 
    }
    delay(75);
  }
  else if(status_pir==false && status_tof==true) // found somthing not alive
  {
    Serial.println("Found Something");
    for(int dutyCycle = 0; dutyCycle <= 150; dutyCycle+=5) // 35,50 to use
    {   
      ledcWrite(ledChannel, dutyCycle);
      delay(20);  // 50 ,100 , 250 
    }
    delay(75);
  }
  else // tof_front = false and pir = false then motor will stop
  {
    ledcWrite(ledChannel, 0);
  }
  tcaselect(5);  //tof top
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
  
  
  tof_top = measure.RangeMilliMeter;
  Serial.print("L2 (Top): "); 
  Serial.print(tof_top);
  Serial.println(" ");
  
  if (measure.RangeStatus != 4 && tof_top>=400 && tof_top<=1000) // tof_top dectect found something
  {  
    for(int dutyCycle = 0; dutyCycle <= 150; dutyCycle+=5) // 35,50 to use
    {   
      ledcWrite(ledChannel2, dutyCycle); // motor sharking
      delay(20);  // 50 ,100 , 250 
    }
  } 
  else
  {
    ledcWrite(ledChannel2, 0); // motor stop
  }
  delay(75);
}
