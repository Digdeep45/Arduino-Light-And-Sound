//A random walking snake that leaves a trail
// Here two arrays are used to remeber the past locations of the snake
// and to fade the brightness out.

//led biz begin. don't worry about anything in this section besides max_brightness
#include <WS2812Serial.h>
//we'll be using the Teensy audio library and it doesn't play nicely with neopixels.h or fastled
// so Paul of PJRC made this much more efficient version
#define num_of_leds 64
#define led_data_pin 5 // only these pins can be used on the Teensy 3.2:  1, 5, 8, 10, 31
byte drawingMemory[num_of_leds * 3];       //  3 bytes per LED
DMAMEM byte displayMemory[num_of_leds * 12]; // 12 bytes per LED
WS2812Serial leds(num_of_leds, displayMemory, drawingMemory, led_data_pin, WS2812_GRB);

//1.0 is VERY bright if you're powering it off of 5V
// this needs to be declared and set to something >0 for the LEDs to work
float max_brightness = 0.2;
//led biz end

//defines are not variables
// but unlike varibles they can be used in the declaration section and don't take up any memory
#define left_button_pin 0
#define right_button_pin 1
#define top_left_pot_pin A0
#define top_right_pot_pin A1
#define bottom_left_pot_pin A2
#define bottom_right_pot_pin A3

unsigned long current_time;
unsigned long prev_time[8]; //array of 8 variables named "prev_time[0] to prev_time[7]"

int xy_sel;
int xy_count;
int x_pot;
int y_pot;
int rate1 = 30;
int x_sel;
int y_sel;

#define snake_length 10
int prev_pos[snake_length]; //you can only do this with a #define, varibles won't work
float fade_level[snake_length];
int trail;
float fade;
float set_hue;
int pos_count, last_xy_sel; //both of these raribles will be integers set to 0
int first_position = 1;
int trail_type;
int pot[4];

void setup() {
  leds.begin(); //must be done in setup for the LEDs to work.

  pinMode(left_button_pin, INPUT_PULLUP); //must be done when reading buttons
  pinMode(right_button_pin, INPUT_PULLUP);
  analogReadResolution(12); //0-4095 pot values
  analogReadAveraging(64);  //smooth the readings some

}


void loop() {
  current_time = millis();

  if (current_time - prev_time[1] > 100) {
    prev_time[1] = current_time;
    if (random(10) > 5) { //if a random roll of the dice that retunrs 0-9 is over 5 do this
      x_sel++;
    }
    //% is remainder aka modulo. remainder = dividend % divisor. https://www.arduino.cc/reference/en/language/structure/arithmetic-operators/remainder/
    //when x_sel reaches 7 it gets wrapped around to 0
    // so 6%7= 6,7%7=0, 9%7=2, 15%7=1.
    // you can use this calculator to try it out //https://www.wolframalpha.com/input/?i=9%257%3D
    // in this situation saying if (x_sel>=7){x_sel=0;} would work beacuse we're only incremnting by 1 so we won't have 9%7=2 come up

    x_sel %= 7;

    if (random(10) > 5) {
      y_sel++;
    }
    y_sel %= 7;

  }

  if (current_time - prev_time[0] > rate1) {
    prev_time[0] = current_time;

    set_hue = (analogRead(bottom_left_pot_pin) / 4095.0); //0.0 to 1
    last_xy_sel = xy_sel;
    xy_sel = x_sel + (y_sel * 8);

    if (xy_sel != last_xy_sel) {
      //shift each space in the array over. 2 goes to 3, 1 goes to 2, etc.
      for (int i = snake_length - 1; i > 0; i--) {
        prev_pos[i] = prev_pos[i - 1];
        fade_level[i] = fade_level[i - 1];
      }

      fade_level[1] = 1.0; // the newest trail gets full brightness
      prev_pos[0] = xy_sel; //The cursor or head idk waht to call it
    }

    //x_count goes from 0-7 and so does y_count but since we have it arranged
    // with one for loop after another we get x_count=0 for y_count from 0-7,
    // then x_count=1 for y_count from 0-7 and so on
    // this way we can more easily deal with the two dimensional LED array

    //Here we just clear the screen
    for ( int x_count = 0; x_count < 8; x_count++) {
      for ( int y_count = 0; y_count < 8; y_count++) {
        xy_count = x_count + (y_count * 8); //goes from 0-64
        set_pixel_HSV(xy_count, 0, 0, 0); // turn everything off. otherwise the last "frame" swill still show
      }
    }
    //now the tail is lit up at the points in the array and at their coresponding brightnesses
    for ( int trail = 0; trail < snake_length; trail++) {
      set_pixel_HSV(prev_pos[trail], set_hue, 1, fade_level[trail]);
    }
    set_pixel_HSV(prev_pos[0], set_hue, 1, 1); //The cursor or head idk waht to call it

    //fade out each spot in the array seperatly
    for ( int i = 0; i < snake_length; i++) {
      fade_level[i] *= .85;
      if (fade_level[i] <= .01) { //an arbitray cuttoff since it will never quite get to 0 through multiplication
        fade_level[i] = 0;
      }
    }

    leds.show(); // after we've set what we want all the LEDs to be we send the data out through this function

  } //timing "if" is over


}
// loop is over



//This function is a little different than you might see in other libraries but it works pretty similar
// instead of 0-255 you see in other libraries this is all 0-1.0
// you can copy this to the bottom of any code as long as the declarations at the top in "led biz" are done

//set_pixel_HSV(led to change, hue,saturation,value aka brightness)
// led to change is 0-63
// all other are 0.0 to 1.0
// hue - 0 is red, then through the ROYGBIV to 1.0 as red again
// saturation - 0 is fully white, 1 is fully colored.
// value - 0 is off, 1 is the value set by max_brightness
// (it's not called brightness since, unlike in photoshop, we're going from black to fully lit up

//based on https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both

void set_pixel_HSV(int pixel, float fh, float fs, float fv) {
  byte RedLight;
  byte GreenLight;
  byte BlueLight;

  while (fh > 1.0) {
    fh -= 1.0;
  }

  byte h = fh * 255;
  byte s = fs * 255;
  byte v = fv * max_brightness * 255;

  h = (h * 192) / 256;  // 0..191
  unsigned int i = h / 32;   // We want a value of 0 thru 5
  unsigned int f = (h % 32) * 8;   // 'fractional' part of 'i' 0..248 in jumps

  unsigned int sInv = 255 - s;  // 0 -> 0xff, 0xff -> 0
  unsigned int fInv = 255 - f;  // 0 -> 0xff, 0xff -> 0
  byte pv = v * sInv / 256;  // pv will be in range 0 - 255
  byte qv = v * (256 - s * f / 256) / 256;
  byte tv = v * (256 - s * fInv / 256) / 256;

  switch (i) {
    case 0:
      RedLight = v;
      GreenLight = tv;
      BlueLight = pv;
      break;
    case 1:
      RedLight = qv;
      GreenLight = v;
      BlueLight = pv;
      break;
    case 2:
      RedLight = pv;
      GreenLight = v;
      BlueLight = tv;
      break;
    case 3:
      RedLight = pv;
      GreenLight = qv;
      BlueLight = v;
      break;
    case 4:
      RedLight = tv;
      GreenLight = pv;
      BlueLight = v;
      break;
    case 5:
      RedLight = v;
      GreenLight = pv;
      BlueLight = qv;
      break;
  }
  leds.setPixelColor(pixel, RedLight, GreenLight, BlueLight);
}
