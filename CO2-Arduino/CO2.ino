#include <TimerOne.h>

#define PIN_CLOCK 2
#define PIN_DATA  7

#define STRBUFMAX 50

uint8_t read_index;
bool    read_outputflag;
uint8_t read_buf;
uint8_t read_timeout_cnt;
uint8_t strbuf[STRBUFMAX];
uint8_t strbuf_len;
uint8_t strbuf_index;

uint16_t CO2,MaxCO2;
float    AirPre,Humi,Temp,I2CTemp;

void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
    pinMode(PIN_CLOCK,INPUT);
    pinMode(PIN_DATA,INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_CLOCK), CallExtINT, FALLING);

    Timer1.initialize(10000);  //0.1s
    Timer1.attachInterrupt(CallTimerINT); // blinkLED to run every 0.15 seconds

    read_buf        = 0;
    strbuf_index    = 0;
    read_outputflag = false; 
    read_index      = 0;
    read_timeout_cnt = 5;

    AirPre = 0;
    Humi   = 0;
    Temp   = 0;
    CO2    = 0;
    MaxCO2 = 0;
    I2CTemp = 0;

    Serial.println("Start = ");
    
}

void loop() {
  // put your main code here, to run repeatedly:
    uint8_t CRC;
    uint16_t data;
    String str;

    if(read_outputflag)
    {
        //Serial.println("OK");
        read_outputflag=false;
        //Serial.println(strbuf_len);
        if(strbuf_len==5) // Length ckeck
        {
            CRC=0;
            for(int i=0;i<3;i++){
                CRC+=strbuf[i];
                //Serial.println(strbuf[i]);
            }
            if(strbuf[3]==CRC & strbuf[4]==0x0D){   // Ckeck sum and ckeck EOL
                data=((((uint16_t)strbuf[1])&0x00ff)<<8) + (((uint16_t)strbuf[2])&0x00ff);
                //Serial.println("Get Data");
                switch(strbuf[0]){                  // Item code
                    case 'P':                       // CO2 ppm
                        CO2 = data;
                        Serial.print("CO2 = ");
                        Serial.print(data);
                        Serial.print(" ppm");
                        Serial.println();
                        Serial.print("---------------------------");
                        Serial.println();
                        //MeasureTimes++;
                        //display.clearDisplay();
                        //display.setTextSize(1);
                        //isplay.setCursor(0,0);
                                                
                        //if(MeasureTimes<5)
                        //{
                        //  display.println("Booting");
                        //  display.setTextSize(2);
                        //  display.setTextColor(WHITE);                          
                        //  display.println(5-MeasureTimes);
                        //}
                        //else
                        {
                          //str = String("") + Temp + String("C  ") + AirPre + String("mmHg");
                          //display.println(str);
                         //str = String("") + Humi + String("%  ") + MeasureTimes%10000;
                          //display.println(str);
                          //display.setTextSize(2);
                          //display.setTextColor(WHITE);
                          //str = String(" ") + CO2 + String(" ppm");
                          //display.println(str);  
                        }
                        //display.display();
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
              //Serial.print("Timer Ture");
        }else{                      // Nothing receive
            read_outputflag = false;
        }
        //Serial.println(strbuf_index);
        read_index   = 0;
        strbuf_index = 0;
    }
    //digitalWrite(13, digitalRead(13) ^ 1); //Debug
}

