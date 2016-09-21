volatile int NbTopsFan; //measuring the rising edges of the signal
double flowrate;                               
int hallsensor = 2;    //The pin location of the sensor
double totalliquid = 0;
int cnt;

void rpm ()     //This is the function that the interupt calls 
{
  NbTopsFan++;  //This function measures the rising and falling edge of the hall effect sensors signal
}

// The setup() method runs once, when the sketch starts
void setup() //
{ 
  pinMode(hallsensor, INPUT); //initializes digital pin 2 as an input
  Serial.begin(9600); //This is the setup function where the serial port is initialised,
  attachInterrupt(0, rpm, RISING); //and the interrupt is attached
  cnt = 0;
}

// the loop() method runs over and over again,
// as long as the Arduino has power

void loop ()    
{
  NbTopsFan = 0;   //Set NbTops to 0 ready for calculations
  interrupts();    //sei();      //Enables interrupts
  delay (1000);   //Wait 1 second
  noInterrupts(); //cli();      //Disable interrupts
  flowrate = ((double)NbTopsFan/4380);  //(Pulse frequency 4380 for 1L   flow rate in L/s 
  totalliquid += flowrate;
  Serial.print (flowrate, DEC); //Prints the number calculated above
  Serial.println (" L/s"); //Prints "L/hour" and returns a  new line
  Serial.print (totalliquid);

  if(flowrate == 0) cnt++;

  if(cnt>=30)
  {
    cnt = 0;
    totalliquid = 0;
  }

}
