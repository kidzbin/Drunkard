#include <OneWire.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x3F, 20, 4);

// OneWire DS18S20, DS18B20, DS1822 Temperature Example
//
// http://www.pjrc.com/teensy/td_libs_OneWire.html
//
// The DallasTemperature library can do all this work for you!
// http://milesburton.com/Dallas_Temperature_Control_Library

OneWire  ds(12);  // on pin 10 (a 4.7K resistor is necessary)

float fertemp[2];

float fertempMax[2];
float fertempMin[2];


void setup(void) {
  Serial.begin(9600);
  
  // initialize the LCD
  lcd.begin();

  // Turn on the blacklight and print a message.
  lcd.backlight();
  lcd.print("Booting");
  
  fertemp[0] = 0; //0xFF
  fertemp[1] = 0; //0x2B

  fertempMax[0] = 0;
  fertempMax[1] = 0;
  fertempMin[0] = 0xFF;
  fertempMin[1] = 0xFF;

  lcd.clear();
  //lcd.home();
  lcd.print(" Fermentating Temp");
  
}

void loop(void) {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  byte Num;
  float celsius, fahrenheit,diff;
  String str;
  
  if ( !ds.search(addr)) {
    Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  
  Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  Serial.print("  Data = ");
  Serial.print(present, HEX);
  Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(data, 8), HEX);
  Serial.println();

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;

  if(celsius>=70) return;

  switch (addr[1])
  {
      case 0xFF: //Internal
        Num = 0;
        break;
      case 0x2B: //Outside
        Num = 1;
        break;
      default:
        break;
  };

   fertemp[Num] = celsius; //0xFF
   if(fertemp[Num]>fertempMax[Num])
    fertempMax[Num] = fertemp[Num];
   if(fertemp[Num]<fertempMin[Num])
    fertempMin[Num] = fertemp[Num];

  diff = fertemp[0] - fertemp[1];

  lcd.setCursor(0, 0);
  lcd.print(String("")  + "Diff : " + diff );
  lcd.setCursor(0, 1);
  lcd.print(String("")  + " Internal :" + fertemp[0] + " C");
  lcd.setCursor(0, 2);
  lcd.print(String("")  + " Outside  :" + fertemp[1] + " C");
  lcd.setCursor(0, 3);
  lcd.print(String("(")  + Num + ")" + "" + fertempMax[Num] +"/"+ fertempMin[Num] );
  
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");
  Serial.println(fertemp[0] , fertemp[1]);
}
