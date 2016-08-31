#include <SPI.h>
#include <Wire.h>
#include <math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET LED_BUILTIN  //4
Adafruit_SSD1306 display(OLED_RESET);

#define SSD1306_LCDHEIGHT 64

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

unsigned int Cnt = 0;
float        R0sum = 0;
double       alcMAX = 0;

double      RL = 2500;

void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  display.clearDisplay();

  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Drunk Detection");
  display.println("  ");
  display.setTextSize(2);
  display.setTextColor(WHITE);
  //display.print("  0.134 mg/L");
  display.display();
  //delay(2000);
  display.clearDisplay();

}

void loop() 
{
  //GetR0();
  GetAcl();
}

void GetAcl(void)
{
  double VolRs;
  double R0=1.30,R;
  double Ratio;
  double alc;
  String str;
 
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("   Drunkard Detect");
  display.println(String("             MAX:") + alcMAX);
  display.setTextSize(2);
  display.setTextColor(WHITE);

  VolRs = GetVoltage();
  //R     = (5-VolRs)/VolRs;
  R      = RL*VolRs/(5-VolRs);

  Ratio = R / R0;
 
  alc = CalAlcohol(Ratio);
  if(alc > alcMAX)
    alcMAX = alc;
 
  Serial.println(Ratio, 10);
  Serial.println(alc , 10);

  str = String(" ") + alc + String(" mg/L");

  display.println(str);
  display.display();
  delay(200);
 
}


void GetR0(void)
{
  double VolRs;
  double R0,Rair;
  String str;
 
  VolRs = GetVoltage();
  //Rair  = (5-VolRs)/VolRs;
   Rair   = RL*VolRs/(5-VolRs);
  R0 = Rair/60.0;
  Cnt+=1;
  R0sum += R0;  
  str = String("") + R0 + String(", ") + R0sum/Cnt;
 
  Serial.println(str);
  //Serial.println(R0,4);
  delay(200);
}

double GetVoltage(void)
{
  return (double)analogRead(0) * 5 /1024;
}


double CalAlcohol(double RRatio)
{
  // f(x) = a * exp ( -b * x**c )
  double a = 4444.52 , b = 9.18605 , c = 0.00643814;
  double alc;

  alc = pow( (log(a)-log(RRatio))/b , 1/c );

  //if( alc > 2)
  //  alc = 0.00;

  return alc;
}

