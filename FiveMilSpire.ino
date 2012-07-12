// Tiny85 versioned - ATmega requires these defines to be redone as well as the
// DDRB/PORTB calls later on.

//#define LINE_A 0 //Pin 5 on ATtiny85
//#define LINE_B 1 //Pin 6 on ATtiny85
//#define LINE_C 3 //Pin 2 on ATtiny85
//#define LINE_D 4 //Pin 3 on ATtiny85

// ATmega is like so, although using 8,9,10 & 11 would keep them all on port B
#define LINE_A 2
#define LINE_B 3 
#define LINE_C 4 
#define LINE_D 5

#include <avr/io.h>

//#define F_CPU 16000000UL

uint8_t led_grid[12] = {
  000 , 000 , 000 , 000 , // R
  000 , 000 , 000 , 000 , // G
  000 , 000 , 000 , 000  // B
};

uint8_t led_grid_next[12] = {
  000 , 000 , 000 , 000 ,  // R
  000 , 000 , 000 , 000 ,  // G
  000 , 000 , 000 , 000 // B
};

void setup() {
  randomSeed(analogRead(2));
//  for(int x = 0; x<12; x++) {
//    light_led(x);
//    delay(100);
//    leds_off;
//  }
  Serial.begin(115200);
}

void loop() {
  switch(random(2)) {
    // led test
  case 0:
    while(1) {
      uint8_t led = random(4);
      int hue = random(360);

      Serial.print(led); Serial.print(" "); Serial.println(hue);
      setLedColorHSV(led,hue, 1, 1);
      draw_for_time(500);
    }
    break;
    // downward flowing rainbow inspired from the shiftPWM library example
  case 1:
    {
      uint8_t width = random(5,20);
      Serial.print("Width: "); Serial.println(width);
      while(1) {
	for(uint16_t colorshift=0; colorshift<360; colorshift++) {
	  for(uint8_t led = 0; led<4; led++) {
	    uint16_t hue = ((led) * 360/(width)+colorshift)%360;
	    //	    Serial.print(led); Serial.print(" "); Serial.print(colorshift); Serial.print(" "); Serial.println(hue);
	    setLedColorHSV(led,hue,1,1);
	  }
	  draw_for_time(20);
	}
      }
      break;
    }
  }
}


void setLedColorHSV(uint8_t p, uint16_t h, float s, float v) {
  // dropping the s term towards 0 makes the stack a barely changing white spire.  Not especially useful.
  // dropping the v term towards 0 makes the maximum brightness drop.
  
  // Lightly adapted from http://eduardofv.com/read_post/179-Arduino-RGB-LED-HSV-Color-Wheel-
  //this is the algorithm to convert from HSV to RGB
  float r=0; 
  float g=0; 
  float b=0;

  //  float hf=h/60.0;

  if(p == 0) {
    Serial.print(h); Serial.print(" ");
  }

  uint8_t i=(int)floor(h/60.0);
  float f = h/60.0 - i;
  float pv = v * (1 - s);
  float qv = v * (1 - s*f);
  float tv = v * (1 - s * (1 - f));

  switch (i)
    {
    case 0: //red
      r = v;
      g = tv;
      b = pv;
      break;
    case 1: // green
      r = qv;
      g = v;
      b = pv;
      break;
    case 2: 
      r = pv;
      g = v;
      b = tv;
      break;
    case 3: // blue
      r = pv;
      g = qv;
      b = v;
      break;
    case 4:
      r = tv;
      g = pv;
      b = v;
      break;
    case 5: // mostly red (again)
      r = v;
      g = pv;
      b = qv;
      break;
    }

  // commented out since they don't exist longer than necessary to pass off to set_led_rgb, so why bother?
  //set each component to a integer value between 0 and 100
  int red=constrain((int)100*r,0,100);
  int green=constrain((int)100*g,0,100);
  int blue=constrain((int)100*b,0,100);
  if(p==0) {
    Serial.print(red); Serial.print(" "); Serial.print(green); Serial.print(" "); Serial.println(blue);
  }

  set_led_rgb(p,red,green,blue);
}

/* Args:
   position - 0-3, bottom to top
   red value - 0-100
   green value - 0-100
   blue value - 0-100
*/
void set_led_rgb (uint8_t p, uint8_t r, uint8_t g, uint8_t b) {
  // red usually seems to need to be attenuated a bit.
  led_grid[p] = r;
  led_grid[p+4] = g;
  led_grid[p+8] = b;
}

/* Args:
   Same as set_led_rgb, except the change affects the next frame, and not in immediate mode
*/
void set_led_rgb_next (uint8_t p, uint8_t r, uint8_t g, uint8_t b) {
  led_grid_next[p] = r;
  led_grid_next[p+4] = g;
  led_grid_next[p+8] = b;
}

// Clears led_grid_next
void clear_grid () {
  for (uint8_t led = 0; led<12; led++) { led_grid_next[led]=0; }
}

// runs draw_frame a supplied number of times.
void draw_for_time(uint16_t time) {
  for(uint16_t f = 0; f<time; f++) { draw_frame(); }
}


const uint8_t led_dir[12] = {
  ( 1<<LINE_A | 1<<LINE_B ), // 1 r
  ( 1<<LINE_B | 1<<LINE_C ), // 2 r
  ( 1<<LINE_C | 1<<LINE_D ), // 3 r
  ( 1<<LINE_D | 1<<LINE_A ), // 4 r

  ( 1<<LINE_D | 1<<LINE_B ), // 1 g
  ( 1<<LINE_A | 1<<LINE_C ), // 2 g
  ( 1<<LINE_B | 1<<LINE_D ), // 3 g
  ( 1<<LINE_C | 1<<LINE_A ), // 4 g

  ( 1<<LINE_C | 1<<LINE_B ), // 1 b
  ( 1<<LINE_D | 1<<LINE_C ), // 2 b
  ( 1<<LINE_A | 1<<LINE_D ), // 3 b
  ( 1<<LINE_B | 1<<LINE_A ), // 4 b
};

//PORTB output config for each LED (1 = High, 0 = Low)
const uint8_t led_out[12] = {
  ( 1<<LINE_A ), // 1 r
  ( 1<<LINE_B ), // 2 r
  ( 1<<LINE_C ), // 3 r
  ( 1<<LINE_D ), // 4 r

  ( 1<<LINE_D ), // 1 g
  ( 1<<LINE_A ), // 2 g
  ( 1<<LINE_B ), // 3 g
  ( 1<<LINE_C ), // 4 g

  ( 1<<LINE_C ), // 1 b
  ( 1<<LINE_D ), // 2 b
  ( 1<<LINE_A ), // 3 b
  ( 1<<LINE_B ), // 4 b

};

void light_led(uint8_t led_num) { //led_num must be from 0 to 19
  //DDRD is the ports in use on an ATmega328, DDRB on an ATtiny85
  DDRD = led_dir[led_num];
  PORTD = led_out[led_num];
}

void leds_off() {
  DDRD = 0;
  PORTD = 0;
}

void draw_frame(void){
  uint8_t led, bright_val, b;
  // giving the loop a bit of breathing room seems to prevent the last LED from flickering.  Probably optimizes into oblivion anyway.
  for ( led=0; led<=12; led++ ) { 
    //software PWM
    bright_val = led_grid[led];
    for( b=0 ; b < bright_val ; b+=1)  { light_led(led); } //delay while on
    for( b=bright_val ; b<100 ; b+=1)  { leds_off(); } //delay while off
  }
}

void fade_to_next_frame(void){
  uint8_t led, changes;
  while(1){
    changes = 0;
    for ( led=0; led<12; led++ ) {
      if( led_grid[led] < led_grid_next[led] ){ led_grid[led] = (led_grid[led]+1); changes++; }
      if( led_grid[led] > led_grid_next[led] ){ led_grid[led] = (led_grid[led]-1); changes++; }
    }

    // I suspect this should be replaced with draw_for_time so that the fading
    // has a chance to be seen.
    draw_frame();
    if( changes == 0 ){break;}
  }
}
