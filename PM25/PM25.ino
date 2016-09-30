
#include <SPI.h>
#include <Wire.h>
#include <math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET LED_BUILTIN  //4
Adafruit_SSD1306 display(OLED_RESET);

#define SSD1306_LCDHEIGHT 64

#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];
 
int PM01Value=0;          //define PM1.0 value of the air detector module
int PM2_5Value=0;         //define PM2.5 value of the air detector module
int PM10Value=0;         //define PM10 value of the air detector module

typedef struct
{
  int PM01Value;
  int PM2_5Value;
  int PM10Value;
  int PM01Value_;
  int PM2_5Value_;
  int PM10Value_;
  int Air03;
  int Air05;
  int Air10;
  int Air25;
  int Air50;
  int Air100;
  int HCHO;
}_stPMS5003;

_stPMS5003 AirData;

unsigned int cnt = 0;
 
void setup()
{
  Serial.begin(9600);   //use serial0
  Serial.setTimeout(1000);    //set the Timeout to 1500ms, longer than the data transmission periodic time of the sensor

  /* -- Initial LCD -- */
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  display.clearDisplay();
  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Booting");
  display.println("  ");
  display.display();
  /* ----------------------------------------*/
  

}
 
void loop()
{
  String str;

  if(Serial.find(0x42)) //start to read when detect 0x42
  {
    Serial.readBytes(buf,LENG);
 
    if(buf[0] == 0x4d)
    {
      if(checkValue(buf,LENG))
      {
        GetData(buf);
        cnt++;
      }        
    }
  }

  static unsigned long OledTimer=millis();  
  if (millis() - OledTimer >=1000) 
  {
      OledTimer=millis(); 
       
      Serial.print("PM1.0: ");  
      Serial.print(AirData.PM01Value);
      Serial.println("  ug/m3");            
     
      Serial.print("PM2.5: ");  
      Serial.print(AirData.PM2_5Value);
      Serial.println("  ug/m3");     
       
      Serial.print("PM1 0: ");  
      Serial.print(AirData.PM10Value);
      Serial.println("  ug/m3");

      Serial.print("HCHO: ");  
      Serial.print((double)AirData.HCHO/1000);
      Serial.println("  mg/m3");  

      Serial.println();

      display.clearDisplay();
      // text display tests
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println(String("PM2.5 Detection ") + cnt%10000);

      str = String("PM1.0: ") + AirData.PM01Value + String(" PM2.5: ") + AirData.PM2_5Value;
      display.println(str);
      str = String("PM 10: ") + AirData.PM10Value;
      display.println(str);
      str = String("HCHO : ") + (double)AirData.HCHO/1000;
      display.println(str);      
      display.display();

    }

}

void GetData(unsigned char *thebuf)
{
  thebuf+=3;

  AirData.PM01Value   = ((thebuf[ 0]<<8) + thebuf[ 1]);
  AirData.PM2_5Value  = ((thebuf[ 2]<<8) + thebuf[ 3]);
  AirData.PM10Value   = ((thebuf[ 4]<<8) + thebuf[ 5]);
  AirData.PM01Value_  = ((thebuf[ 6]<<8) + thebuf[ 7]);
  AirData.PM2_5Value_ = ((thebuf[ 8]<<8) + thebuf[ 9]);
  AirData.PM10Value_  = ((thebuf[10]<<8) + thebuf[11]);
  AirData.Air03       = ((thebuf[12]<<8) + thebuf[13]);
  AirData.Air05       = ((thebuf[14]<<8) + thebuf[15]);
  AirData.Air10       = ((thebuf[16]<<8) + thebuf[17]);
  AirData.Air25       = ((thebuf[18]<<8) + thebuf[19]);
  AirData.Air50       = ((thebuf[20]<<8) + thebuf[21]);
  AirData.Air100      = ((thebuf[22]<<8) + thebuf[23]);
  AirData.HCHO        = ((thebuf[24]<<8) + thebuf[25]);
}

char checkValue(unsigned char *thebuf, char leng)
{  
  char receiveflag=0;
  int receiveSum=0;
  
  for(int i=0; i<(leng-2); i++){
  receiveSum=receiveSum+thebuf[i];
  }
  receiveSum=receiveSum + 0x42;
  
  if(receiveSum == ((thebuf[leng-2]<<8)+thebuf[leng-1]))  //check the serial data 
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}

#if 0
int transmitPM01(unsigned char *thebuf)
{
  int PM01Val;
  PM01Val=((thebuf[3]<<8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM01Val;
}
 
//transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf)
{
  int PM2_5Val;
  PM2_5Val=((thebuf[5]<<8) + thebuf[6]);//count PM2.5 value of the air detector module
  return PM2_5Val;
  }
 
//transmit PM Value to PC
int transmitPM10(unsigned char *thebuf)
{
  int PM10Val;
  PM10Val=((thebuf[7]<<8) + thebuf[8]); //count PM10 value of the air detector module  
  return PM10Val;
}
#endif
