/* 
  Chromakey ring light with 24 Neopixel ring and rotally encoder w/ switch
  driven by Arduino compatible Pro Micro 5v board

  https://learn.adafruit.com/retroreflective-green-screen-light-ring-controller/code-the-chromakey-light-ring
  http://bildr.org/2012/08/rotary-encoder-arduino/

*/

#include <Adafruit_NeoPixel.h>
#include <Bounce2.h>

//these pins can not be changed. 2/3 are special pins
int encoderPin1 = 2;
int encoderPin2 = 3;

int encoderSwitchPin = A3; //push button switch
int blueLed = 10;
int redLed = 9;
int greenLed = 5;
int neopixelPin = 6;
int neopixel_n = 24;

volatile int lastEncoded = 0;
volatile byte encoderValue = 0;
volatile byte brightnessValue = 96;
volatile bool changed = true;
int mode_int = 0; // 0: Green, 1: Blue, 2: Rainbow
int n_modes = 3;

int lastMSB = 0;
int lastLSB = 0;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(neopixel_n, neopixelPin, NEO_GRB + NEO_KHZ800);
// INSTANTIATE A Button OBJECT FROM THE Bounce2 NAMESPACE
Bounce2::Button button = Bounce2::Button();

void setup() {

  Serial.begin(115200); //This pipes to the serial monitor
  Serial.println("Initialize Serial Monitor");
  
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(blueLed, OUTPUT);

  pinMode(encoderPin1, INPUT_PULLUP);
  pinMode(encoderPin2, INPUT_PULLUP);

  //pinMode(encoderSwitchPin, INPUT);
  button.attach( encoderSwitchPin ,  INPUT );
  // DEBOUNCE INTERVAL IN MILLISECONDS
  button.interval(5); 
  // INDICATE THAT THE LOW STATE CORRESPONDS TO PHYSICALLY PRESSING THE BUTTON
  button.setPressedState(LOW); 
  
  //call updateEncoder() when any high/low changed seen
  //on interrupt 0 (pin 2), or interrupt 1 (pin 3)
  attachInterrupt(0, updateEncoder, CHANGE);
  attachInterrupt(1, updateEncoder, CHANGE);

  digitalWrite(redLed, HIGH);
  digitalWrite(greenLed, LOW);
  digitalWrite(blueLed, HIGH);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop(){
  button.update();
  
  if(button.pressed()){
    //button is being pushed
    mode_int++;
    int mode = mode_int % n_modes;
    Serial.print("Mode changed to: ");
    if (mode==0){ // Green
      Serial.println("Green");
      digitalWrite(redLed, HIGH);
      digitalWrite(greenLed, LOW);
      digitalWrite(blueLed, HIGH);
    }
    else if (mode==1){ // Blue
      Serial.println("Blue");
      digitalWrite(redLed, HIGH);
      digitalWrite(greenLed, HIGH);
      digitalWrite(blueLed, LOW);
      
    }
    else { // Rainbow
      Serial.println("Rainbow");
      digitalWrite(redLed, LOW);
      digitalWrite(greenLed, HIGH);
      digitalWrite(blueLed, HIGH);
    }
    changed = true;
  }
 
  if (changed){
    Serial.print("brightness: ");
    Serial.print(brightnessValue);
    Serial.print(", encoderValue: ");
    Serial.println(encoderValue);
    strip.setBrightness(brightnessValue);
    int mode = mode_int % n_modes;
    uint32_t color;
    if (mode==0){
      color = strip.Color(0, 255, 0);
    }
    else if (mode==1){
      color = strip.Color(0, 0, 255);
    }
    else {
      color = Wheel(encoderValue);
    }
    for (uint16_t i=0; i<neopixel_n; i++){
      strip.setPixelColor(i, color);
    }
    strip.show();
    changed = false;
  }
  delay(10); //just here to slow down the output, and show it will work even during a delay
}

void updateEncoder(){
  int MSB = digitalRead(encoderPin1); //MSB = most significant bit
  int LSB = digitalRead(encoderPin2); //LSB = least significant bit
  
  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number 
  int sum = (lastEncoded << 2) | encoded; //adding it to the previous encoded value
  int mode = mode_int % n_modes;
 
  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {

    if (mode==2){
      encoderValue ++;      
    }
    else{
      brightnessValue ++;
    }
  }
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) { 
    if (mode==2){
      encoderValue --; 
    }
    else{
      brightnessValue --;
    }
  }
  lastEncoded = encoded; //store this value for next time 
  changed = true;

}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
