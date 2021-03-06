#include <stdint.h>
#include <neopixel.h>
#include "sequences.h"

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_COUNT 16
#define PIXEL_PIN A5
#define PIXEL_TYPE WS2812B
#define MAX_BRIGHTNESS 200

uint8_t grid_brightness = 200;
uint8_t state_transition_flag = 1;
uint8_t mode_transition_flag = 1;

Adafruit_NeoPixel strip(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

int currentPixel = 1;

uint32_t red = strip.Color(255, 0, 0);
uint32_t green = strip.Color(0, 255, 0);
uint32_t blue = strip.Color(0, 0, 255);
uint32_t white = strip.Color(255, 255, 255);
uint32_t magenta = strip.Color(255, 0, 255);
uint32_t black = strip.Color(0,0,0);

uint16_t custom_animation_framerate = 200;

//***************************
//Animation control variables
static uint16_t frame = 0;
static uint32_t begin = 0;

const uint8_t pixel_map[] = {
  0,7,8,15,
  1,6,9,14,
  2,5,10,13,
  3,4,11,12
};

bool missed_call = false;
bool twitter_mention  = false;
bool custom_animation = false;
bool raining = false;

int missed_call_flashes = 0;
int missed_call_flash_limit  = 3;

int mention_flashes = 0;
int mention_flash_limit = 10;

int rain_flashes = 0;
int rain_flash_limit = 10;

void setup() {

    strip.begin();
    delay(50);
    strip.setBrightness(100);
    strip.clear();
    strip.show();

    Serial.begin(115200);
    Serial.println("ONLINE SUNSHINE\n");

    Particle.function("phone_event", phoneEvent);
    Particle.function("fill", fill_function);
    Particle.function("mention", twitterMention);
    Particle.function("acknowledge", acknowledge);
    Particle.function("custom", triggerCustom);
    Particle.function("weather", triggerWeather);

    delay(250);
    fill_worm(strip.Color(30,30,30));
    fill_worm(black);
}

int triggerWeather(String weather) {
    
    if (weather == "rain") {
        fill_worm(white);
        raining = true;
        return 0;
    }
    
    return 1;
    
}

int triggerCustom(String arg) {
    custom_animation = true;
    return 0;
}

int acknowledge(String arg) {
    twitter_mention = false;
    missed_call = false;
    custom_animation = false;
    raining = false;
    
    fill_worm(magenta);
    fill_worm(black);
    return 0;
}

int twitterMention(String command) {
    twitter_mention = true;
    return 0;
}

int phoneEvent(String command) {

    Particle.publish("phoneEvent", command);
    if (command == "missed_call") {
        missed_call = true;
        Particle.publish("device_event", "RX MISSED CALL EVENT");
        return 1;
    }
    return 0;
}

int fill_function(String command) {

    String red_str = search_string(command, ',', 0);
    String green_str = search_string(command, ',', 1);
    String blue_str = search_string(command, ',', 2);
    String bright_str = search_string(command, ',', 3);

    uint8_t red = red_str.toInt();
    uint8_t green = green_str.toInt();
    uint8_t blue = blue_str.toInt();
    uint8_t brightness = bright_str.toInt();

    Particle.publish("colors", String::format("R %d, G %d, B %d BRIGHT %d", red, green, blue, brightness));
    uint32_t fill_color = strip.Color(red, green, blue);
    if (brightness < MAX_BRIGHTNESS) {
        strip.setBrightness(brightness);
        delay(10);
    }
    fill_worm(fill_color);

    fill_worm(black);
    strip.setBrightness(MAX_BRIGHTNESS);
    return 0;
}

void update_display() {

    if (missed_call)  {
        missed_call_sequence();

        if (missed_call_flashes > missed_call_flash_limit) {
            missed_call = false;
            missed_call_flashes = 0;
            strip.clear();
            strip.show();
        }
    }

    if (twitter_mention) {
        mention_flashes++;
        mention_display(magenta);

        if (mention_flashes > mention_flash_limit) {
            twitter_mention = false;
            mention_flashes = 0;
        }
    }

    if (raining) {
        
        rain_sequence();
        
        if (rain_flashes > rain_flash_limit) {
            raining = false;
            rain_flashes = 0;
        }
    }

    if (custom_animation) {
        custom_sequence();
    }
}

void mention_display(uint32_t c) {

        strip.setPixelColor(2, c);
        strip.setPixelColor(3, c);
        strip.show();
        delay(500);

        strip.clear();
        strip.show();
        delay(50);

        strip.setPixelColor(0, c);
        strip.setPixelColor(1, c);
        strip.setPixelColor(2, c);
        strip.setPixelColor(7, c);
        strip.show();
        delay(500);

        strip.clear();
        strip.show();
        delay(50);

        strip.setPixelColor(0, c);
        strip.setPixelColor(1, c);
        strip.setPixelColor(7, c);
        strip.setPixelColor(8, c);
        strip.show();
        delay(500);

        strip.clear();
        strip.show();
        delay(50);

        strip.setPixelColor(0, c);
        strip.setPixelColor(7, c);
        strip.setPixelColor(8, c);
        strip.setPixelColor(15, c);
        strip.show();
        delay(500);

        strip.clear();
        strip.show();
        delay(50);

        strip.setPixelColor(7, c);
        strip.setPixelColor(8, c);
        strip.setPixelColor(15, c);
        strip.setPixelColor(14, c);
        strip.show();
        delay(500);

        strip.clear();
        strip.show();
        delay(50);
}

void rain_sequence(void) {
      //**********************************************
  //Check if enough time has passed between frames
  if(time_since(begin) < custom_animation_framerate)
  {
    return;
  }

  //*****************************************
  //Check if we're just starting the sequence
  if(state_transition_flag)
  {
    strip.setBrightness(100);
    state_transition_flag = 0;
    frame = 0;
  }
  begin = millis();

  //*****************************************
  //Update the LEDs to display the next frame
  for(uint8_t i=0; i<strip.numPixels(); i++)
  {
    strip.setPixelColor(pixel_map[i],raining_data[frame][i]);
  }
  strip.show();

  //*********************************************************************
  //Set the animation to loop by starting from the beginning once it ends
  if(++frame >= RAINING_FRAME_COUNT)
  {
    frame = 0;
    // missed_call = false;
    rain_flashes++;
  }

  return;
}
void missed_call_sequence(void)
{
  //**********************************************
  //Check if enough time has passed between frames
  if(time_since(begin) < custom_animation_framerate)
  {
    return;
  }

  //*****************************************
  //Check if we're just starting the sequence
  if(state_transition_flag)
  {
    strip.setBrightness(100);
    state_transition_flag = 0;
    frame = 0;
  }
  begin = millis();

  //*****************************************
  //Update the LEDs to display the next frame
  for(uint8_t i=0; i<strip.numPixels(); i++)
  {
    strip.setPixelColor(pixel_map[i],snake_animation_lut[frame][i]);
  }
  strip.show();

  //*********************************************************************
  //Set the animation to loop by starting from the beginning once it ends
  if(++frame >= snake_animation_length)
  {
    frame = 0;
    // missed_call = false;
    missed_call_flashes++;
  }

  return;
}

// not used yet, experimental
void car_chase(uint32_t pc, uint32_t sc) {

    for(uint8_t i=0; i<strip.numPixels(); i++)
    {
        strip.setPixelColor(pixel_map[i],pc);

        if (i > 0) {
            strip.setPixelColor(pixel_map[i-1],sc);
        }
        if (i > 1) {
            strip.setPixelColor(pixel_map[i-2],black);
        }
    }
    strip.show();
}

// fills the grid with a new color, 1 pixel at a time
void fill_worm(uint32_t c) {

    for (int i = 0; i < PIXEL_COUNT; i++) {
        strip.setPixelColor(i, c);
        strip.show();
        delay(25);
    }

}

void set_all_pixels(uint32_t c) {

    for (int i = 0; i < PIXEL_COUNT; i++) {
        strip.setPixelColor(i, c);
    }

    strip.show();
}


void loop() {
    update_display();
}

//*******************************************
//The function that runs your custom sequence
void custom_sequence(void)
{
  //**********************************************
  //Check if enough time has passed between frames
  if(time_since(begin) < custom_animation_framerate)
  {
    return;
  }

  //*****************************************
  //Check if we're just starting the sequence
  if(state_transition_flag)
  {
    strip.setBrightness(100);
    state_transition_flag = 0;
    frame = 0;
  }
  begin = millis();

  //*****************************************
  //Update the LEDs to display the next frame
  for(uint8_t i=0; i<strip.numPixels(); i++)
  {
    strip.setPixelColor(pixel_map[i],custom_animation_lut[frame][i]);
  }
  strip.show();

  //*********************************************************************
  //Set the animation to loop by starting from the beginning once it ends
  if(++frame >= custom_animation_length)
  {
    frame = 0;
  }

  return;
}

//**************************************************
//Return the number of milliseconds since start_time
uint32_t time_since(uint32_t start_time)
{
  return(millis()-start_time);
}

//******************************************************************************************
//Search for delimiter separated values within a string - used for parsing incoming commands
String search_string(String data, char delimiter, int index)
{
    uint8_t i;
    uint8_t found = 0;
    int8_t strIndex[] = {0, -1};
    uint8_t maxIndex = data.length()-1;

    //***************************************
    //Search for delimiters within the string
    for(i=0; i<=maxIndex && found<=index; i++)
    {
      if(data.charAt(i)==delimiter || i==maxIndex)
      {
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
      }
    }

    //***********************
    //If we found a delimiter
    if(found>index)
    {
      return(data.substring(strIndex[0], strIndex[1]));
    }
    else
    {
      return("");
    }
}
