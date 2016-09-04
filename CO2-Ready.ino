#include <TimerOne.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define PIN_CLOCK 2
#define PIN_DATA  3
#define STRBUFMAX 50
#define TIME_PERIOD  1000

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
float    AirPre,Humi,Temp;

void setup() {
    Serial.begin(9600);
    pinMode(PIN_CLOCK,INPUT);
    pinMode(PIN_DATA,INPUT);
    pinMode(13, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_CLOCK), CallExtINT, FALLING);
    Timer1.initialize(TIME_PERIOD);
    Timer1.attachInterrupt(CallTimerINT);

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
                        str = String("") + "CO2 = " + CO2 + " ppm" + "\n" + "---------------------------";
                        MeasureTimes++;
                        if(MeasureTimes > 10) //Record Max CO2
                          if(CO2>MaxCO2) MaxCO2 = CO2;
                        Serial.println(CO2);
                        break;
                    case '|':                       // I2C Temperature
                        str = String("") + "I2C Temperature = " + ((float)data)/100 + " degC" ;
                        break;
                    case 'A':                       // I2C RH
                        Humi = ((float)data)/100;
                        str = String("") + "RH = " + Humi + " %" ;
                        break;
                    case 'B':                       // Temperature
                        Temp = (((float)(data))/16.0-273.15);
                        str = String("") + "Temperature = " + Temp + " degC" ;
                        break;
                    case '_':                       // I2C Atmospheric pressure
                        AirPre = ((float)data)/10;
                        str = String("") + "Atmospheric pressure = " + AirPre + " mmHg" ;
                        break;
                }

                //Serial.println(str);

                if(MeasureTimes)
                {
                   display.clearDisplay();

                   display.setTextSize(1);
                   display.setCursor(0,0);
                   str = String("") + Temp + String("C") + String(" ") + AirPre + String("mmHg ")  + MeasureTimes%1000;
                   display.println(str);
                   display.println( String("") + Humi + String("%  ") + "Max:" + MaxCO2 + " ppm" );
                   display.setTextSize(2);
                   display.setTextColor(WHITE);
                   str = String(" ") + CO2 + String(" ppm");
  
                   display.println(str);
                   display.display();
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

void CallTimerINT()
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
