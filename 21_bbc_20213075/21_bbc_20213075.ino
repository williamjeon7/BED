#include <Servo.h>

// Arduino pin assignment
#define PIN_IR A0
#define PIN_LED 9
#define PIN_SERVO 10

// configurable parameters
#define _DUTY_MIN 1300 // servo full clockwise position (0 degree)
#define _DUTY_NEU 1700// servo neutral position (90 degree)
#define _DUTY_MAX 2100 // servo full counterclockwise position (180 degree)

#define _POS_START (_DUTY_MIN + 100)
#define _POS_END (_DUTY_MAX - 100)

#define _SERVO_SPEED 100 // servo speed limit (unit: degree/second)
#define INTERVAL 20  // servo update interval

// global variables
unsigned long last_sampling_time; // unit: ms
int duty_chg_per_interval; // maximum duty difference per interval
int toggle_interval, toggle_interval_cnt;
float pause_time; // unit: sec
Servo myservo;
int duty_target, duty_curr;
int a, b; // unit: mm

void setup() {
// initialize GPIO pins
  pinMode(PIN_LED,OUTPUT);
  digitalWrite(PIN_LED, 1);
  myservo.attach(PIN_SERVO); 
  duty_target = duty_curr = _DUTY_NEU;
  myservo.writeMicroseconds(duty_curr);
  
// initialize serial port
  Serial.begin(57600);

// convert angle speed into duty change per interval.
    duty_chg_per_interval = (_DUTY_MAX - _DUTY_MIN) * (_SERVO_SPEED / 180.0) * (INTERVAL / 1000.0);

// initialize variables for servo update.
  pause_time = 1;
  toggle_interval = (180.0 / _SERVO_SPEED + pause_time) * 1000 / INTERVAL;
  toggle_interval_cnt = toggle_interval;
  
// initialize last sampling time
  last_sampling_time = 0;
  
  a = 85;
  b = 300;
}

float ir_distance(void){ // return value unit: mm
  float val;
  float volt = float(analogRead(PIN_IR));
  val = ((6762.0/(volt-9.0))-4.0) * 10.0;
  return val;
}

void loop() {
  float raw_dist = ir_distance();
  float dist_cali = 100 + 300.0 / (b - a) * (raw_dist - a);
  
  // wait until next sampling time. 
// millis() returns the number of milliseconds since the program started. Will overflow after 50 days.
  if(millis() < last_sampling_time + INTERVAL) return;

// adjust duty_curr toward duty_target by duty_chg_per_interval
  if(duty_target > duty_curr) {
    duty_curr += duty_chg_per_interval;
    if(duty_curr > duty_target) duty_curr = duty_target;
  }
  else {
    duty_curr -= duty_chg_per_interval;
    if(duty_curr < duty_target) duty_curr = duty_target;
  }
// update servo position
  myservo.writeMicroseconds(duty_curr);

// output the read value to the serial port
  Serial.print("Min:1000https://www.dropbox.com/s/xwgob8tga1ny42p/12_example_1.ino?dl=0,duty_target:");
  Serial.print(duty_target);
  Serial.print(",duty_curr:");
  Serial.print(duty_curr);
  Serial.println(",Max:2000");

// toggle duty_target between _DUTY_MIN and _DUTY_MAX.
  if(toggle_interval_cnt >= toggle_interval) {
    toggle_interval_cnt = 0;
      // duty_target setting
    if(dist_cali >255) {
      duty_target = _DUTY_MIN;
    }

    else {
      duty_target = _DUTY_MAX;
    }
  }
  else {
    toggle_interval_cnt++;
  }

// update last sampling time
  last_sampling_time += INTERVAL;
}
