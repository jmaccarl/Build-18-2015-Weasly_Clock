/*****************************************************************
XBee_Serial_Passthrough.ino

Set up a software serial port to pass data between an XBee Shield
and the serial monitor.

Hardware Hookup:
  The XBee Shield makes all of the connections you'll need
  between Arduino and XBee. If you have the shield make
  sure the SWITCH IS IN THE "DLINE" POSITION. That will connect
  the XBee's DOUT and DIN pins to Arduino pins 2 and 3.
*****************************************************************/
// We'll use SoftwareSerial to communicate with the XBee:
#include <SoftwareSerial.h>
#include <Servo.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 7
#define N_LEDS 56

#define PIN_PWM_MINUTE 5
#define PIN_PWM_HOUR 6

#define PIN_5V 13

// XBee's DOUT (TX) is connected to pin 2 (Arduino's Software RX)
// XBee's DIN (RX) is connected to pin 3 (Arduino's Software TX)
//SoftwareSerial XBee(2, 3); // RX, TX
Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

unsigned int count = 0;
unsigned int sum_count = 0;
unsigned int vel_count = 0;
unsigned int sum = 0;
float old_power = 36;

unsigned int hourAngle = 0;
unsigned int minuteAngle = 0;

float hourZero = 91;
float minuteZero = 88.7;

unsigned int cw = 120;
unsigned int ccw = 60;

unsigned int samples[50] = {36, 36, 36, 36, 36, 36, 36, 36, 36, 36,36, 36, 36, 36, 36, 36, 36, 36, 36, 36,36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,36, 36, 36, 36, 36, 36, 36, 36, 36, 36};
unsigned int index = 0;

unsigned int oldVelD = 36;

Servo hourHand;
Servo minuteHand;

int justResetMinute = 0;
int justResetHour = 0;

void setup()
{
  // Set up both ports at 9600 baud. This value is most important
  // for the XBee. Make sure the baud rate matches the config
  // setting of your XBee.
  //XBee.begin(9600);
  Serial.begin(9600);
  hourHand.attach(PIN_PWM_HOUR);
  minuteHand.attach(PIN_PWM_MINUTE);
  strip.begin();
  for (int i = 0; i < N_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(150, 150, 150));
  }
  strip.show();
  hourHand.write(hourZero); //Zero-point for "hours" = 91 degrees
  minuteHand.write(minuteZero); //Zero-point for "minutes" = 89.3 degrees
  
  pinMode(PIN_5V, OUTPUT);
  digitalWrite(PIN_5V, HIGH);
  
  moveMinuteServoTo(180);
  minuteAngle = 180;
  moveMinuteServoTo(0);
  minuteAngle = 0;
  moveHourServoTo(180);
  hourAngle = 180;
  moveHourServoTo(0);
  hourAngle = 0;
  //pinMode(4, OUTPUT);
  //digitalWrite(4, HIGH);
  calibrate();
}

void calibrate() {
  int calibVal = analogRead(A0);
  hourHand.write(cw);
  while (analogRead(A0) > calibVal-6) {}
  delay(80);
  hourHand.write(hourZero);
  justResetHour = 10;
  calibVal = analogRead(A0);
  minuteHand.write(cw);
  while (analogRead(A0) > calibVal-6) {}
  delay(80);
  minuteHand.write(minuteZero);
  justResetMinute = 10;
}

void loop()
{
  
  /*if (Serial.available())
  { // If data comes in from serial monitor, send it out to XBee
    XBee.write(Serial.read());
  }*/
  if (/*XBee*/Serial.available())
  { // If data comes in from XBee, send it out to serial monitor
    unsigned int val = /*XBee*/Serial.read();
    if (val == 0x7E) {
        count=0;
    }


    if(count == 6 && (old_power - val <= 10.0) && (old_power - val >= -10.0)){
        //Serial.print("\n Old Ave power reading: ");
        //Serial.print(old_power);
        //Serial.print("\n New val: ");
        //Serial.print(val);
        samples[index] = val;
        if (index == 49) index = 0;
        else index++;
        //sum += val;
        sum_count++;
      
    }
    /*sum = 0;
    for (int i = 0; i < 9; i++) {
      sum += samples[i];
    }*/
    
    if(sum_count == 50) {
        sum = 0;
        for (int i = 0; i < 49; i++) {
            sum += samples[i];
        }
        //Serial.print(sum/50.0);
        //Serial.print("\n");
        
        //Serial.print("velocity ");
        //Serial.print(sum/50.0 - old_power);
        //Serial.print("\n");
        
        //hand1.write(round5(126-(sum/50)));
        uint16_t r = 0;
        uint16_t b = 255;
        int newMinuteAngle = minuteAngle;
        if (sum <= 1800) {
          r = 250;
          b = 0;
          newMinuteAngle = 0;
        }
        else if (sum > 1800 && sum < 3050) {
          r = 250;
          b = (sum-1800)/5;
          newMinuteAngle = (sum-1800)/10;
        }
        else if (sum >= 3050 && sum < 4300) {
          r = (4300-sum)/5;
          b = 250;
          newMinuteAngle = (sum-1800)/10;
        }
        else {
          r = 0;
          b = 250;
          newMinuteAngle = 250;
        }
        for (int i = 0; i < N_LEDS; i++) {
          strip.setPixelColor(i, strip.Color((uint8_t)r, 0, (uint8_t)b));
        }
        //Serial.print("Set r="); Serial.print(r); Serial.print("\n");
        //Serial.print("Set b="); Serial.print(b); Serial.print("\n");
        
        //Calculate servo angle
        if (vel_count >= 3) {
          int newHourAngle = (sum/50.0 - old_power)*100;
          if (newHourAngle < 0) newHourAngle *= -1;
          int alreadyWaitedVel = moveHourServoTo(newHourAngle);
          hourAngle = newHourAngle;
          vel_count = 0;
          old_power = sum/50.0;
        }
        int alreadyWaitedPos = moveMinuteServoTo(newMinuteAngle);
        delay(10);
        //if (alreadyWaitedPos < 10) delay(10-alreadyWaited);
        minuteAngle = newMinuteAngle;
        strip.show();
        //hand1.write(90);
        
        if (!justResetHour && hourAngle <= 15) {
          int calibVal = analogRead(A0);
          hourHand.write(cw);
          while (analogRead(A0) > calibVal-6) {}
          delay(80);
          hourHand.write(hourZero);
          justResetHour = 10;
          hourAngle = 0;
        }
        else if (justResetHour && hourAngle > 15) {
          justResetHour = 0;
        }
        
        if (!justResetMinute && minuteAngle <= 15) {
          int calibVal = analogRead(A0);
          minuteHand.write(cw);
          while (analogRead(A0) > calibVal-6) {}
          delay(80);
          minuteHand.write(minuteZero);
          justResetMinute = 10;
          minuteAngle = 0;
        }
        else if (justResetMinute && minuteAngle > 15) {
          justResetMinute = 0;
        }
        
        /*if (!justResetMinute) {
          int prevHourAngle = hourAngle;
          hourAngle = 0;
          int prevMinuteAngle = minuteAngle;
          minuteAngle = 0;
          calibrate();
          int trash = moveHourServoTo(prevHourAngle);
          hourAngle = prevHourAngle;
          trash = moveMinuteServoTo(prevMinuteAngle);
          minuteAngle = prevMinuteAngle;
        }*/
        
        sum = 0;
        sum_count = 0;
        vel_count++;
        /*justResetMinute--;
        justResetHour--;*/
    }
        
    //Serial.write(XBee.read());
    count++;
  }
}

int round5(int num) {
  int rem = num%5;
  int res;
  if (rem >= 3) res = (num/5)*5 + 5;
  else res = (num/5)*5;
  //Serial.print("rounding "); Serial.print(num); Serial.print(" to "); Serial.print(res); Serial.print("\n");
  return res;
}

// returns the total servo delay used
int moveHourServoTo(unsigned int deg) {
  //hand1.write(90);
  //return 0;
  
  int relDeg = deg-hourAngle;
  if (relDeg == 0) return 0;
  if (relDeg > 0) {
    hourHand.write(ccw);
    delay(7.5*relDeg);
    hourHand.write(hourZero);
    return relDeg;
  }
  //relDeg < 0
  hourHand.write(cw);
  delay(-7.5*relDeg);
  hourHand.write(hourZero);
  return(-relDeg);
}

// returns the total servo delay used
int moveMinuteServoTo(unsigned int deg) {
  //hand1.write(90);
  //return 0;
  
  int relDeg = deg-minuteAngle;
  if (relDeg == 0) return 0;
  if (relDeg > 0) {
    minuteHand.write(ccw);
    delay(7.5*relDeg);
    minuteHand.write(minuteZero);
    return relDeg;
  }
  //relDeg < 0
  minuteHand.write(cw);
  delay(-7.5*relDeg);
  minuteHand.write(minuteZero);
  return(-relDeg);
}

//Using 56 total LEDs
