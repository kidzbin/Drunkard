//#include <TimerOne.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

extern "C" {
#include "user_interface.h"
}

os_timer_t ESP8266Timer;

#define PIN_CLOCK 0
#define PIN_DATA  2

#define STRBUFMAX 50
#define TIME_PERIOD  

#define OLED_RESET LED_BUILTIN  //4
Adafruit_SSD1306 display(OLED_RESET);

#define SSD1306_LCDHEIGHT 64

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];

uint8_t read_index;
bool    read_outputflag;
uint8_t read_buf;
uint8_t read_timeout_cnt;
uint8_t strbuf[STRBUFMAX];
uint8_t strbuf_len;
uint8_t strbuf_index;
uint32_t MeasureTimes;

uint16_t CO2,MaxCO2;
float    AirPre,Humi,Temp,I2CTemp;

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

void setup() {
    Serial.begin(9600);
    Serial.setTimeout(1000);    //set the Timeout to 1500ms, longer than the data transmission periodic time of the sensor
    
    pinMode(PIN_CLOCK,INPUT);
    pinMode(PIN_DATA,INPUT);
    pinMode(13, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_CLOCK), CallExtINT, FALLING);
    //Timer1.initialize(TIME_PERIOD);
    //Timer1.attachInterrupt(CallTimerINT);
    os_timer_setfn(&ESP8266Timer, CallTimerINT, NULL);
    os_timer_arm(&ESP8266Timer, 5, true);

    read_buf        = 0;
    strbuf_index    = 0;
    read_outputflag = false; 
    read_index      = 0;
    read_timeout_cnt = 5;

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    // text display tests
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Co2 Detection");
    //display.println("  ");
    //display.setTextSize(2);
    //display.setTextColor(WHITE);
 
    display.display();
    //delay(2000);
    display.clearDisplay();
    MeasureTimes = 0;

    AirPre = 0;
    Humi   = 0;
    Temp   = 0;
    CO2    = 0;
    MaxCO2 = 0;
    I2CTemp = 0;
}

void loop() {
    uint8_t CRC;
    uint16_t data;
    String str;

#if 0
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
  }
#endif

#if 1
    if(read_outputflag){
        read_outputflag=false;
        if(strbuf_len==5){                          // Length ckeck
            CRC=0;
            for(int i=0;i<3;i++){
                CRC+=strbuf[i];
            }
            if(strbuf[3]==CRC & strbuf[4]==0x0D){   // Ckeck sum and ckeck EOL
                data=((((uint16_t)strbuf[1])&0x00ff)<<8) + (((uint16_t)strbuf[2])&0x00ff);
                
                switch(strbuf[0]){                  // Item code
                    case 'P':                       // CO2 ppm
                        CO2 = data;

                        MeasureTimes++;
                        display.clearDisplay();
                        display.setTextSize(1);
                        display.setCursor(0,0);

                        if(MeasureTimes<5)
                        {
                          display.println("Booting");
                          display.setTextSize(2);
                          display.setTextColor(WHITE);                          
                          display.println(5-MeasureTimes);
                        }
                        else
                        {
                          str = String("") + Temp + String("C  ") + AirPre + String("mmHg");
                          display.println(str);
                          str = String("") + Humi + String("%  ") + MeasureTimes%10000;
                          display.println(str);
                          display.setTextSize(2);
                          display.setTextColor(WHITE);
                          str = String(" ") + CO2 + String(" ppm");
                          display.println(str);  
                        }
                        display.display();
                        GetSerial();
                        break;
                    case '|':                       // I2C Temperature
                        I2CTemp = (float)data/100;
                        break;
                    case 'A':                       // I2C RH
                        Humi = ((float)data)/100;
                        break;
                    case 'B':                       // Temperature
                        Temp = ((float)(data))/16.0-273.15;
                        break;
                    case '_':                       // I2C Atmospheric pressure
                        AirPre = ((float)data)/10 ;
                        break;
                }

                Serial.println(String("Air Pressure = ") + AirPre + " mmHg");
                Serial.println(String("Temperuature = ") + Temp + " C");
                Serial.println(String("Temp I2C     = ") + I2CTemp + " C");
                Serial.println(String("Humidity     = ") + Humi + " %");
                Serial.println(String("CO2          = ") + CO2 + " ppm");

            }
        }
    }
 #endif   
}

void CallExtINT()
{
    read_timeout_cnt = 5;

    read_buf<<=1;
    if(digitalRead(PIN_DATA)==0){
        read_buf &= ~1;
    }else{
        read_buf |=  1;
    }
    read_index++;
    if(read_index>7){
        read_index=0;
        strbuf[strbuf_index]=read_buf;
        if(strbuf_index<STRBUFMAX){
            strbuf_index++;
        }
    }

}

void CallTimerINT(void *pArg)
{
    if(read_timeout_cnt>0){
        read_timeout_cnt--;
    }else
    {                          // Time out
        if(strbuf_index!=0){        // Receive something
            strbuf_len      = strbuf_index;
            read_outputflag = true;
        }else{                      // Nothing receive
            read_outputflag = false;
        }
        //Serial.println(strbuf_index);
        read_index   = 0;
        strbuf_index = 0;
    }
    //digitalWrite(13, digitalRead(13) ^ 1); //Debug
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

//  for(int i=0;i<leng;i++)
//  {
//    Serial.print(thebuf[i],HEX);
//    Serial.print(" ");
//  }
  
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

void GetSerial(void)
{
    if(Serial.find(0x42))//start to read when detect 0x42
    {
        Serial.readBytes(buf,LENG);
 
        if(buf[0] == 0x4d)
        {
          if(checkValue(buf,LENG))
          {
            GetData(buf);   
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

          }
        }
    }
}

