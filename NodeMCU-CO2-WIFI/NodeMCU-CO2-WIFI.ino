//#include <TimerOne.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

extern "C" {
#include "user_interface.h"
}

const char* host = "api.thingspeak.com"; // Your domain  
String ApiKey = "CQ85TFFBN74R6WRD";
String path = "/update?key=" + ApiKey + "&field1=";  

const char* ssid = "Elida";
const char* pass = "999999999";

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

void setup() {
    Serial.begin(9600);
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

    WiFi.begin(ssid, pass);
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.print(".");
    }
    
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

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
    Serial.println("Init Done");
}

void loop() {
    uint8_t CRC;
    uint16_t data;
    String str;
    
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
                        Serial.print("CO2 = ");
                        Serial.print(data);
                        Serial.print(" ppm");
                        Serial.println();
                        Serial.print("---------------------------");
                        Serial.println();
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


  static unsigned long Oldtime = millis();

  if( (millis() - Oldtime)  > 60*1000)
  {
     Oldtime = millis();

     str =  String("") + CO2;

    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
    }

    client.print(String("GET ") + path + str + "&field2=" + Temp + " HTTP/1.1\r\n" +
             "Host: " + host + "\r\n" + 
             "Connection: keep-alive\r\n\r\n");
     
    }

                          
                        }
                        display.display();
                        break;
                    case '|':                       // I2C Temperature
                        I2CTemp = (float)data/100;
                        Serial.print("I2C Temperature = ");
                        Serial.print(I2CTemp);
                        Serial.print(" degC");
                        Serial.println();
                        break;
                    case 'A':                       // I2C RH
                        Humi = ((float)data)/100;
                        Serial.print("RH = ");
                        Serial.print(Humi);
                        Serial.print(" %");
                        Serial.println();
                        break;
                    case 'B':                       // Temperature
                        Temp = ((float)(data))/16.0-273.15;
                        Serial.print("Temperature = ");
                        Serial.print(Temp); //
                        Serial.print(" degC");
                        Serial.println();
                        break;
                    case '_':                       // I2C Atmospheric pressure
                        AirPre = ((float)data)/10 ;
                        Serial.print("Atmospheric pressure = ");
                        Serial.print(AirPre);
                        Serial.print(" mmHg");
                        Serial.println();
                        break;
                }
            }
        }
    }



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
