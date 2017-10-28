
#include <NeoPixelBus.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define PIN_WIRE 31
#define PIN_LED 18
#define PIN_SWITCH 30
#define PIN_OUT1 0
#define PIN_OUT2 1
#define PIN_OUT3 2
#define PIN_VOUT 3
#define PIN_IN1 27
#define PIN_IN2 28
#define PIN_IN3 29

NeoPixelBus<NeoGrbwFeature, Neo400KbpsMethod> strip(3, PIN_LED);
OneWire oneWire(PIN_WIRE);
DallasTemperature sensors(&oneWire);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);

  pinMode(PIN_WIRE, OUTPUT);
  pinMode(PIN_LED, OUTPUT); 
  pinMode(PIN_OUT1, OUTPUT);
  pinMode(PIN_OUT2, OUTPUT);
  pinMode(PIN_OUT3, OUTPUT);
  pinMode(PIN_VOUT, OUTPUT);
  pinMode(PIN_IN1, INPUT);
  pinMode(PIN_IN2, INPUT);
  pinMode(PIN_IN3, INPUT);
  strip.Begin();
  sensors.begin();
  strip.Show();
  Serial.println( "'t' = LED-Test  'r' = Relay-Test" );
}

int i = 0;

void loop() {

  if(Serial.available())  
  {    
    switch( Serial.read())
    {
      case 'l':
        ledTest();
        break;
      case 'r':
        relaisTest();
        break;
    }
  }

  Serial.print( "Switch/DI1/DI2/DI3/TEMP  ");  
  Serial.print( digitalRead(PIN_SWITCH)); Serial.print( "/"); 
  Serial.print( digitalRead(PIN_IN1));   Serial.print( "/"); 
  Serial.print( digitalRead(PIN_IN2));   Serial.print( "/"); 
  Serial.print( digitalRead(PIN_IN3));   Serial.print( "/"); 
  sensors.requestTemperatures();
  Serial.print(sensors.getTempCByIndex(0));   Serial.println(); 
  delay(500);
  i++;
  if( i==10 )
  {
    i=0;
    Serial.println( "'l' = LED-Test  'r' = Relay-Test" );
  }
}

void relaisTest()
{
  digitalWrite( PIN_OUT1, HIGH );
  Serial.println( "REL 1:  1"); 
  delay(1000);
  digitalWrite( PIN_OUT1, LOW );
  Serial.println( "REL 1:  0"); 
  delay(1000);
  digitalWrite( PIN_OUT2, HIGH );
  Serial.println( "REL 2:  1"); 
  delay(1000);
  digitalWrite( PIN_OUT2, LOW );
  Serial.println( "REL 2:  0"); 
  delay(1000);
  digitalWrite( PIN_OUT3, HIGH );
  Serial.println( "REL 3:  1"); 
  delay(1000);
  digitalWrite( PIN_OUT3, LOW );
  Serial.println( "REL 3:  0"); 
  delay(1000);
  digitalWrite( PIN_VOUT, HIGH );
  Serial.println( "VOUT:   1"); 
  delay(1000);
  digitalWrite( PIN_VOUT, LOW );
  Serial.println( "VOUT:   0"); 
  digitalWrite( PIN_OUT1, HIGH );
  digitalWrite( PIN_OUT2, HIGH );
  digitalWrite( PIN_OUT3, HIGH );
  digitalWrite( PIN_VOUT, HIGH );
  Serial.println( "ALL:  1"); 
  delay(1000);
  digitalWrite( PIN_OUT1, LOW);
  digitalWrite( PIN_OUT2, LOW );
  digitalWrite( PIN_OUT3, LOW );
  digitalWrite( PIN_VOUT, LOW );
  Serial.println( "ALL:  0"); 
}

void ledTest()
{
  for( int p = 0; p<3; p++ )
  {
      ledShow( p, 100,0,0,0 );
      ledShow( p, 0,100,0,0 );
      ledShow( p, 0,0,100,0 );
      ledShow( p, 0,0,0,100 );
  }
}
void ledShow( byte p, byte r, byte g, byte b, byte w )
{
RgbwColor c(r,g,b,w);
      strip.SetPixelColor(p, c); 
      strip.Show();
      delay(300);  
      strip.SetPixelColor(p, RgbwColor(0));
      strip.Show(); // This sends the updated pixel color to the hardw`are.
      delay(300);  

}

