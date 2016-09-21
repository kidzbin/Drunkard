#include <SPI.h>
#include <Wire.h>
#include <math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET LED_BUILTIN  //4
Adafruit_SSD1306 display(OLED_RESET);

#define SSD1306_LCDHEIGHT 64
#define FLOW_PIN 2

typedef struct {
  double freq;
  double flowrate;
  double totalml;
}stFlow;

#if (SSD1306_LCDHEIGHT != 64)
  #error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

volatile int NbTopsFan = 0; //measuring the rising edges of the signal
int cnt;

stFlow beerflow;

void rpm ()     //This is the function that the interupt calls 
{
  NbTopsFan++;  //This function measures the rising and falling edge of the hall effect sensors signal
}

// The setup() method runs once, when the sketch starts
void setup() //
{ 
  /* -- Initial LCD -- */
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  beerflow.freq     = 0;
  beerflow.flowrate = 0;
  beerflow.totalml  = 0;

  ShowOnOLED();

  pinMode(FLOW_PIN, INPUT); //initializes digital pin 2 as an input
  Serial.begin(9600); //This is the setup function where the serial port is initialised,
  attachInterrupt(0, rpm, RISING); //and the interrupt is attached
  cnt = 0;

}

// the loop() method runs over and over again,
// as long as the Arduino has power

void loop ()
{
  unsigned long times,interval;
  int           delaycnt;

  NbTopsFan = 0;   //Set NbTops to 0 ready for calculations

  interrupts();    //Enables interrupts

  times = millis();
  cnt++;
  ShowOnOLED();
  interval = millis()-times;

  delaycnt = (interval<1000)?(1000-interval):1000;
  delay (delaycnt);    //Wait 1 second
  noInterrupts();  //Disable interrupts

  beerflow.freq     = (double)NbTopsFan;

//  if(cnt>=30)
//  {
//     cnt = 0;
//     beerflow.totalml = 0;
//  }

}

void ShowOnOLED(void)
{
  String str;
  
  display.clearDisplay();
  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Beer FlowRate");
  display.println("  ");
  display.setTextSize(2);
  display.setTextColor(WHITE);
  str = String(" ") + (int)beerflow.totalml + String("ml"); 
  display.print(str);
  display.display();
  /* ----------------------------------------*/
}

void GetBeerFlow(void)
{
  String str;

  beerflow.flowrate = ((double)beerflow.freq/4380)*1000;
  beerflow.totalml += beerflow.flowrate;
  str = String("Freq:") + beerflow.freq + String(" Rate:") + beerflow.flowrate +  String(" ml/s ") + String("Total:") +  beerflow.totalml;
  Serial.println(str);
}

