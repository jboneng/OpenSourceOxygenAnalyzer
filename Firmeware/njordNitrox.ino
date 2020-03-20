#include <Wire.h>
#include <SPI.h>
#include <Adafruit_ADS1015.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

#define OLED_RESET 4
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

Adafruit_ADS1015 ads;     
Adafruit_SH1106 display(OLED_RESET);

double  calibrationv; //used to store calibrated value
byte current_function=0;

double mod_low_meter;
double mod_med_meter;
double mod_high_meter;

double fractionO2;

const int CalibrationButton=13;

//string constants
const char *slashSeperator=" / ";

#if (SH1106_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void setup() {
   display.begin(SH1106_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
   display.clearDisplay();
  
   pinMode(CalibrationButton,INPUT);

   ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
   ads.begin();
 
   calibrationv=calibrate();
}

int calibrate(){
  int16_t adc0=0;
  double result;
  for(byte i=0; i<=49; i++)
       {
         adc0=adc0+ads.readADC_SingleEnded(0);
       }
  
  result=adc0/50;
  return result;
}

void loop() {
    int16_t adc0=0;
    double result;//After calculations holds the current O2 percentage
    double currentmv; //the current mv put out by the oxygen sensor;
    double calibratev;

    
    char adc0Value[10];
    char currentmvValue[10];
  
  byte calibrateButtonstate=digitalRead(CalibrationButton);
  
  if(calibrateButtonstate==HIGH){
    if(current_function==0){
       current_function=1;//Sensor needs to be calibrated
    }
  }

  switch(current_function){
    case 0://Analyzing O2
     //taking 50 samples. The sensor might spike for a millisecond. After we average the samples into one value
     for(byte i=0; i<=49; i++)
       {
         adc0=adc0+ads.readADC_SingleEnded(0);
       }
       
      currentmv = adc0/50;
      calibratev=calibrationv;
      result=(currentmv/calibratev)*20.9;

      fractionO2=result/100;
      
      mod_low_meter=((1.4/fractionO2)-1)*10;
      mod_med_meter=((1.5/fractionO2)-1)*10;
      mod_high_meter=((1.6/fractionO2)-1)*10;

      //Write to display
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE, BLACK);
      display.setCursor(0,0);
      display.print(F("O2%"));
      display.setTextColor(WHITE, BLACK);
      display.print(" ");
      display.setTextSize(3);
      display.println(result,1);

      display.setTextSize(2);
      display.setCursor(0,30);
      display.println(F("MOD:"));
      display.setTextSize(1);
      display.setCursor(0,50);
      display.print((int)floor(mod_low_meter));
      display.print(slashSeperator);
      display.print((int)floor(mod_med_meter));
      display.print(slashSeperator);
      display.print((int)floor(mod_high_meter));
      
      display.display();
  
           delay(1000);
    break;
   case 1:
     display.clearDisplay();
     display.setCursor(30,25);
     display.setTextSize(2);
     display.setTextColor(WHITE,BLACK);
     display.println(F("Calc.."));
     display.display();
     
     current_function=0;//O2 analyzing
     calibrationv=calibrate();
     delay(2000);

   break;
  }
}
