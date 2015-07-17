/** Weasley Clock Code - Main Arduino on Clock
 *  
 *  Clock changes color from red to blue and changes clock hands, to demonstrate distance from
 *  the clock of a given user. Distance is approximated using xbee RSSI power reading.
 */

 //Using 56 total LEDs


#include <SoftwareSerial.h>
#include <Servo.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 7
#define N_LEDS 56

Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

unsigned int count = 0;
unsigned int sum_count = 0;
unsigned int sum = 0;
float old_power = 36;

unsigned int samples[50] = {36, 36, 36, 36, 36, 36, 36, 36, 36, 36,36, 36, 36, 36, 36, 36, 36, 36, 36, 36,36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,36, 36, 36, 36, 36, 36, 36, 36, 36, 36};
unsigned int index = 0;
unsigned int minuteAngle = 0;

void setup()
{ 
  Serial.begin(9600);
  strip.begin();
  for (int i = 0; i < N_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(150, 150, 150));
  }
  strip.show();
}

void loop()
{
  if (Serial.available())
  { 
    unsigned int val = Serial.read();
    if (val == 0x7E) {
        count=0;
    }

    /* Take power samples in circular array (7th byte is RSSI)*/
    if(count == 6 && (old_power - val <= 10.0) && (old_power - val >= -10.0)){
        samples[index] = val;
        if (index == 49) index = 0;
        else index++;
        sum_count++;
      
    }

    /* React every 50 measurements */
    if(sum_count == 50) {
        sum = 0;
        for (int i = 0; i < 49; i++) {
            sum += samples[i];
        }

        /* Change lights */
        uint16_t r = 0;
        uint16_t b = 255;
        int newMinuteAngle = minuteAngle;
        if (sum <= 1800) {
          r = 250;
          b = 0;
          newMinuteAngle = 0;
        }
        else if (sum > 1800 && sum < 2300/*sum > 1800 && sum < 3050*/) {
          r = 250;
          b = (sum-1800)/2;
          newMinuteAngle = (sum-1800)/10;
        }
        else if (sum >= 2300 && sum < 2800/*sum >= 3050 && sum < 4300*/) {
          r = (/*43*/2800-sum)/2/*5*/;
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
        Serial.print("Set r="); Serial.print(r); Serial.print("\n");
        Serial.print("Set b="); Serial.print(b); Serial.print("\n");
        
        delay(10);
        strip.show();

        sum = 0;
        sum_count = 0;

        int alreadyWaitedPos = moveMinuteServoTo(newMinuteAngle);
        delay(10);
        minuteAngle = newMinuteAngle;
        
        /* OLD discontinued clock hand code. Needs to be updated/improved */
        */
        
        /*if (!justResetHour && hourAngle <= 15) {
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
        

        justResetMinute--;
        justResetHour--;*/
    }
        
    count++;
  }
}

// returns the total servo delay used
int moveMinuteServoTo(unsigned int deg) {
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

