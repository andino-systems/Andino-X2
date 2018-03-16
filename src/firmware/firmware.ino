//--------------------------------------------------------------------------------------------
//
//        ___      .__   __.  _______   __  .__   __.   ______      ___   ___  ___   
//       /   \     |  \ |  | |       \ |  | |  \ |  |  /  __  \     \  \ /  / |__ \  
//      /  ^  \    |   \|  | |  .--.  ||  | |   \|  | |  |  |  |     \  V  /     ) | 
//     /  /_\  \   |  . `  | |  |  |  ||  | |  . `  | |  |  |  |      >   <     / /  
//    /  _____  \  |  |\   | |  '--'  ||  | |  |\   | |  `--'  |     /  .  \   / /_  
//   /__/     \__\ |__| \__| |_______/ |__| |__| \__|  \______/     /__/ \__\ |____| 
//
// Please install https://github.com/MCUdude/MightyCore
// Docu:          https://github.com/andino-systems/Andino-X2/tree/master/src/firmware
//
// Use:	 
//       Boards:          MightyCore/ATmega164
//       Pinout:          Standard
//       Clock:           12 MHz external
//       Compiler LTO:    Disabled
//       Variant:         164P / 164 PA
//       BOD:             2.7 Volt
//
//       Baudrate:        38400
//
// Version 1 from 16.03.2018
//
//  V1: Initial
//
//--------------------------------------------------------------------------------------------

#include <EEPROM.h>
#include "TimerOne.h"
#include <avr/wdt.h>
#include <NeoPixelBus.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define PIN_WIRE  31  // Dallas
#define LED_PIN   18  // RGB

#define IN_1_PIN   27
#define IN_2_PIN   28
#define IN_3_PIN   29
#define SWITCH_PIN 30

#define POWER_OUT_PIN   3 // PB3: & Power Out
#define RELAY_1_PIN     0 // PB0: Rel 1
#define RELAY_2_PIN     1 // PB1: Rel 2
#define RELAY_3_PIN     2 // PB2: Rel 3

#define LF 10
#define CR 13

#define BAUD_RATE 38400

NeoPixelBus<NeoRgbFeature, Neo400KbpsMethod> strip(3, LED_PIN);
OneWire oneWire(PIN_WIRE);
DallasTemperature sensors(&oneWire);


// ---------------------- [ Receive Data ]------------------
#define RX_SIZE  19
byte RxIndex = 0;
char RxBuffer[RX_SIZE+1];

// ---------------------- [ Setup Data ]------------------
struct Setup
{
  unsigned long CRC;          // CRC of the data
  byte          StructLen;    // Length of the structure
  unsigned int  PollCycle;    // Poll cycle in ms
  byte          PollCount;    // Poll-Count till level is stable
  bool          CountOnLH;    // Count on .. edge
  byte          SkipCount;    // Skip nn Scans after recognize a pulse
  unsigned int  SendCycle;    // Send cycle in ms
  byte          Shield;       // 0 = none
} TheSetup;

// ---------------------- [ Input & Debounce ] ----------------
typedef struct {
  byte current_val = 0;
  byte poll_counter = 0;
  byte skip_counter = 0;
  unsigned long Counter = 0;
} CounterControl;

CounterControl Counter1;
CounterControl Counter2;
CounterControl Counter3;

int ledState = LOW;

// ---------------------- [ Releais ] ----------------

typedef struct {
  byte puls_timer  = 0;
} RelayControl;

RelayControl PowerRelay;
RelayControl Relay1;
RelayControl Relay2;
RelayControl Relay3;

void setup() 
{
  Serial.begin(BAUD_RATE);

  memset( &PowerRelay, 0, sizeof( RelayControl ) );
  memset( &Relay1, 0, sizeof( RelayControl ) );
  memset( &Relay2, 0, sizeof( RelayControl ) );
  memset( &Relay3, 0, sizeof( RelayControl ) );

  SetupRead();
  pinMode(LED_PIN, OUTPUT);
  setup_in_out();

  strip.Begin();
  sensors.begin();
  strip.Show();

  setup_interrupt();

  Serial.println( "STRT" );
}

void setup_in_out()
{
  pinMode(IN_1_PIN,INPUT);   
  pinMode(IN_2_PIN,INPUT); 
  pinMode(IN_3_PIN,INPUT); 

  pinMode(POWER_OUT_PIN,OUTPUT);
  pinMode(RELAY_1_PIN,OUTPUT); 
  pinMode(RELAY_2_PIN,OUTPUT);
  pinMode(RELAY_3_PIN,OUTPUT);
  digitalWrite(POWER_OUT_PIN, LOW); 
  digitalWrite(RELAY_1_PIN, LOW); 
  digitalWrite(RELAY_2_PIN, LOW); 
  digitalWrite(RELAY_3_PIN, LOW); 
}

void setup_interrupt()
{
  unsigned long timeInMicro = TheSetup.PollCycle;
  timeInMicro *= 1000;
  Timer1.initialize(timeInMicro); 
  Timer1.detachInterrupt();
  Timer1.attachInterrupt(timerInterrupt);
}

unsigned long lastSendMillis = 0;
unsigned long lastSecondMillis = 0;

word loopCounter = 0;

void loop() 
{
  DoCheckRxData();
  unsigned long currentMillis = millis();
  if (currentMillis - lastSendMillis  >= TheSetup.SendCycle) 
  {
    lastSendMillis = currentMillis;
    Serial.write( ':' );
    PrintHex16(++loopCounter); 
    Serial.write( '{0000,' );
    PrintHex16(Counter1.Counter); 
    Serial.write( ',' );
    PrintHex16(Counter2.Counter);
    Serial.write( ',' );
    PrintHex16(Counter3.Counter);

    Serial.write( "}{" );
    Serial.print(digitalRead(SWITCH_PIN));
    Serial.write( ',' );
    Serial.print(Counter1.current_val, DEC ); 
    Serial.write( ',' );
    Serial.print(Counter2.current_val, DEC ); 
    Serial.write( ',' );
    Serial.print(Counter3.current_val, DEC );
    Serial.println("}");
  }

  if (currentMillis - lastSecondMillis  >= 1000) 
  {
    lastSecondMillis = currentMillis;
    if( PowerRelay.puls_timer != 0 )
    {
      if( --PowerRelay.puls_timer == 0 )
      {
        digitalWrite(POWER_OUT_PIN, 0);
        Serial.println( "POWR 0" );
      }
    }
    if( Relay1.puls_timer != 0 )
    {
      if( --Relay1.puls_timer == 0 )
      {
        digitalWrite(RELAY_1_PIN, 0);
        Serial.println( "REL1 0" );
      }
    }
    if( Relay2.puls_timer != 0 )
    {
      if( --Relay2.puls_timer == 0 )
      {
        digitalWrite(RELAY_2_PIN, 0);
        Serial.println( "REL2 0" );
      }
    }
    if( Relay3.puls_timer != 0 )
    {
      if( --Relay3.puls_timer == 0 )
      {
        digitalWrite(RELAY_3_PIN, 0);
        Serial.println( "REL3 0" );
      }
    }
  }
}

void timerInterrupt()
{
  doCounter( &Counter1, digitalRead(IN_1_PIN) );
  doCounter( &Counter2, digitalRead(IN_2_PIN) );
  doCounter( &Counter3, digitalRead(IN_3_PIN) );  
}

void doCounter( CounterControl * pCounter, byte input )
{
  if( pCounter->skip_counter == 0 )
  {
    input = (input==1 )?0:1;
    if( pCounter->current_val  == input && input == TheSetup.CountOnLH)
    {
      if( pCounter->poll_counter  != 0xff ) // schon gemeldet
      {
        pCounter->poll_counter ++;
        if( pCounter->poll_counter == TheSetup.PollCount )
        {
           pCounter->Counter++;
           pCounter->poll_counter  = 0xff;
           pCounter->skip_counter = TheSetup.SkipCount;
        }
      }
    }
    else
    {
      pCounter->poll_counter = 0;
      pCounter->current_val  = input;
    }
  }
  else
  {
    pCounter->skip_counter--;
  }
}

// ----------------------------------------------------------------------------------
// Setup
// ----------------------------------------------------------------------------------
unsigned long SetupCalcCrc()
{
  const unsigned long crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };

  unsigned long crc = ~0L;

  byte * p = (byte*)&TheSetup.StructLen;
  int len = sizeof( TheSetup ) - sizeof( TheSetup.CRC );
  for (int i = 0 ; i<len; i++)
  {
    crc = crc_table[(crc ^ *p) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (*p >> 4)) & 0x0f] ^ (crc >> 4);
    p++;
    crc = ~crc;
  }
  return crc;
}

void SetupDefault()
{
  TheSetup.PollCycle = 10;    // Poll cykle 
  TheSetup.PollCount = 3;     // Poll count till accepted
  TheSetup.SkipCount = 0;     // Skip nn Scans after recognize a pulse
  TheSetup.CountOnLH = 1;     // LH Edge
  TheSetup.SendCycle = 5000;  // Cycle in ms
  TheSetup.Shield = 0;        // No Shield 
}

#define EEPROM_ADDRESS  0
void SetupRead()
{
  EEPROM.get(EEPROM_ADDRESS, TheSetup);
  if( TheSetup.CRC != SetupCalcCrc() )
  {
    Serial.println( "ERROR CONFIG SET TO DEFAULT" );
    DoCmdInfo();
    SetupDefault();
    SetupWrite();
  }
}

void SetupWrite()
{
  TheSetup.StructLen = sizeof( TheSetup );
  TheSetup.CRC = SetupCalcCrc();
  EEPROM.put(EEPROM_ADDRESS, TheSetup);
}

void ledShowValue( byte p, int v )
{
  byte b = v % 10;
  byte g = v / 10 % 10;
  byte r = v / 100 % 10;
  ledShow( p, r<<4,g<<4,b<<4);
}
void ledShow( byte p, byte r, byte g, byte b )
{
RgbColor c(r,g,b);
      strip.SetPixelColor(p, c); 
      strip.Show();
}

// ----------------------------------------------------------------------------------
// R X D
// ----------------------------------------------------------------------------------
void DoCheckRxData()
{
  if( Serial.available() > 0 )
  {
    if( RxIndex >= RX_SIZE )
    {
      RxIndex = 0;
    }
    byte rx = Serial.read();

    if( RxIndex == 0 && (rx == LF || rx == CR) )
      return;
    if( rx == LF || rx == CR )
    {
      RxBuffer[RxIndex++] = 0;
      OnDataReceived(); 
      RxIndex = 0;
    }
    else
    {
      RxBuffer[RxIndex++] = rx;
    }
  }
}


// ----------------------------------------------------------------------------------
// C O M M A N D S
// ----------------------------------------------------------------------------------
// RESET          ( Restart controller)
// INFO           ( print settings)
// HARD           ( Hardware, 0=noShield)
// POLL 10        ( Poll cycle in ms )
// DEBO 3         ( Debounce n Scans stable to accept )
// SKIP 3         ( Skip n Scans after pulse reconized )
// EDGE 1|0       ( count on Edge HL or LH )
// SEND 5000      ( send all xxx ms )
// REL1 0|1       ( set relay 1 to on or off )
// REL2 0|1       ( set relay 2 to on or off )
// REL3 0|1       ( set relay 3 to on or off )
// POWR 0|1       ( set Power relay to on or off )
// RPU1 1000      ( pulse relay 1 for nnn ms )
// RPU2 1000      ( pulse relay 2 for nnn ms )
// RPU3 1000      ( pulse relay 3 for nnn ms )
// PPWR 1000      ( pulse power relay for nnn ms )
// LED1 123       ( set LED1 to R=1,G=2,B=3 )
// TEMP			  ( request the Temperature)
void OnDataReceived()
{
  bool result = false;
  bool writeSetup = false;
  bool callSetupInterrupt = false;
  String cmd = RxBuffer;
  cmd.toUpperCase();
  
  if( cmd.startsWith("RESET"))
  {
    Serial.println( "OK" );
    DoCmdReset();
  }
  else
  if( cmd.startsWith("INFO"))
  {
    DoCmdInfo();
    result = true;
  }
  else
  if( cmd.startsWith("TEMP"))
  { 
      sensors.requestTemperatures();
      Serial.println(sensors.getTempCByIndex(0)); 
      result = true;
  }
  // commands with args from here..
  int value = toInt( cmd, 5 );
  if( value != -1 )
  {
    if( cmd.startsWith("POLL "))
    {
      if( !checkRange( value, 10, 1000 ) )
        return;
      TheSetup.PollCycle = value;
      Serial.print( "POLL ");
      Serial.println( value, DEC );
      writeSetup = true;
      result = true;
      callSetupInterrupt = true;
    }
    else
    if( cmd.startsWith("SKIP "))
    {
      if( !checkRange( value, 0, 250 ) )
        return;
      TheSetup.SkipCount = value;
      Serial.print( "SKIP ");
      Serial.println( value, DEC );
      writeSetup = true;
      result = true;
      callSetupInterrupt = true;
    }
    else
    if( cmd.startsWith("DEBO "))
    {
      if( !checkRange( value, 1, 20 ) )
        return;
      TheSetup.PollCount = value;
      Serial.print( "DEBO ");
      Serial.println( value, DEC );
      result = true;
      writeSetup = true;
    }
    else
    if( cmd.startsWith("LED1 "))
    {
      if( !checkRange( value, 0, 999 ) )
        return;
      ledShowValue(0,value);
      result = true;
    }
    else
    if( cmd.startsWith("LED2 "))
    {
      if( !checkRange( value, 0, 999 ) )
        return;
      ledShowValue(1,value);
      result = true;
    }
    else
    if( cmd.startsWith("LED3 "))
    {
      if( !checkRange( value, 0, 999 ) )
        return;
      ledShowValue(2,value);
      result = true;
    }
    else
    if( cmd.startsWith("EDGE "))
    {
      if( !checkRange( value, 0, 1 ) )
        return;
      TheSetup.CountOnLH = value;
      Serial.print( "EDGE ");
      Serial.println( value, DEC );
      result = true;
      writeSetup = true;
    }
    else
    if( cmd.startsWith("POWR "))
    {
      if( !checkRange( value, 0, 1 ) )
        return;
      digitalWrite(POWER_OUT_PIN, value);
      Serial.print( "REL1 ");
      Serial.println( value, DEC );
      result = true;
    }
    else
    if( cmd.startsWith("REL1 "))
    {
      if( !checkRange( value, 0, 1 ) )
        return;
      digitalWrite(RELAY_1_PIN, value);
      Serial.print( "REL1 ");
      Serial.println( value, DEC );
      result = true;
    }
    else
    if( cmd.startsWith("REL2 "))
    {
      if( !checkRange( value, 0, 1 ) )
        return;
      digitalWrite(RELAY_2_PIN, value);
      Serial.print( "REL2 ");
      Serial.println( value, DEC );
      result = true;
    }
    else
    if( cmd.startsWith("REL3 "))
    {
      if( !checkRange( value, 0, 1 ) )
        return;
      digitalWrite(RELAY_3_PIN, value);
      Serial.print( "REL3 ");
      Serial.println( value, DEC );
      result = true;
    }
    else
    if( cmd.startsWith("PPWR "))
    {
      if( !checkRange( value, 1, 255 ) )
        return;
      digitalWrite(POWER_OUT_PIN, 1);
      PowerRelay.puls_timer = value;
      Serial.print( "PPWR ");
      Serial.println( value, DEC );
      Serial.println( "POWR 1" );
      result = true;
    }
    else
    if( cmd.startsWith("RPU1 "))
    {
      if( !checkRange( value, 0, 255 ) )
        return;
      digitalWrite(RELAY_1_PIN, 1);
      Relay1.puls_timer = value;
      Serial.print( "RPU1 ");
      Serial.println( value, DEC );
      Serial.println( "REL1 1" );
      result = true;
    }
    else
    if( cmd.startsWith("RPU2 "))
    {
      if( !checkRange( value, 1, 255 ) )
        return;
      digitalWrite(RELAY_2_PIN, 1);
      Relay2.puls_timer = value;
      Serial.print( "RPU2 ");
      Serial.println( value, DEC );
      Serial.println( "REL2 1" );
      result = true;
    }
    else
    if( cmd.startsWith("RPU3 "))
    {
      if( !checkRange( value, 0, 255 ) )
        return;
      digitalWrite(RELAY_3_PIN, 1);
      Relay3.puls_timer = value;
      Serial.print( "RPU3 ");
      Serial.println( value, DEC );
      Serial.println( "REL3 1" );
      result = true;
    }
    else
/*    if( cmd.startsWith("HARD"))
    {
      if( !checkRange( value, 0, 2 ) )
        return;
      TheSetup.Shield = value;
      Serial.print( "HARD ");
      Serial.println( value, DEC );
      setup_in_out();
      writeSetup = true;
      result = true;
    }
    else*/
    if( cmd.startsWith("SEND "))
    {
      if( !checkRange( value, 1000, 50000 ) )
        return;
       TheSetup.SendCycle = value;
      Serial.print( "SEND ");
      Serial.println( value, DEC );
      writeSetup = true;
      result = true;
    }
  }
  if( result )
  {
    if( writeSetup )
      SetupWrite();
    if( callSetupInterrupt )
      setup_interrupt();
  }
  else
    Serial.println( "SYNTAX" );
}

void DoCmdInfo()
{
    Serial.print( "HARD "); Serial.println( TheSetup.Shield, DEC );
    Serial.print( "POLL "); Serial.println( TheSetup.PollCycle, DEC );
    Serial.print( "DEBO "); Serial.println( TheSetup.PollCount, DEC );
    Serial.print( "SKIP "); Serial.println( TheSetup.SkipCount, DEC );
    Serial.print( "EDGE "); Serial.println(TheSetup.CountOnLH, DEC );
    Serial.print( "SEND "); Serial.println( TheSetup.SendCycle, DEC);
    Serial.print( "POWR "); Serial.println( digitalRead(POWER_OUT_PIN), DEC);
    Serial.print( "REL1 "); Serial.println( digitalRead(RELAY_1_PIN), DEC);
    Serial.print( "REL2 "); Serial.println( digitalRead(RELAY_1_PIN), DEC);
    Serial.print( "REL3 "); Serial.println( digitalRead(RELAY_1_PIN), DEC);
//    Serial.println( "HARD 0 (no extension)" );
//    Serial.println( "HARD 1 (1DI2DO)" );
//    Serial.println( "HARD 2 (2DO)" );

}

void DoCmdReset()
{
  wdt_enable( WDTO_60MS );
   while(1) {}
}


bool checkRange( unsigned int val, unsigned int mini, unsigned int maxi )
{
  if( val < mini || val > maxi )
  {
    Serial.print( "INVALID. MIN=" );
    Serial.print( mini, DEC );
    Serial.print( " MAX=" );
    Serial.println( maxi, DEC );
    return false;
  }
  return true;
}

int toInt( String s, int pos  )
{
  String valString = s.substring(pos);
  valString.trim();
  if( valString.length() == 0 )
    return -1;
  if( !isNumeric(valString) )
    return -1;
  return valString.toInt();
}

boolean isNumeric(String str) 
{
    for(char i = 0; i < str.length(); i++) 
    {
        if ( !(isDigit(str.charAt(i)) || str.charAt(i) == '.' )) 
        {
            return false;
        }
    }
    return true;
}


void PrintHex16(word data)
{
 char tmp[5];
 byte first;
 int j=0;

   first = (data >> 12) | 48;
   if (first > 57) tmp[0] = first + (byte)7;
   else tmp[0] = first ;
  
   first = ((data>>8) & 0x0F) | 48;
   if (first > 57) tmp[1] = first + (byte)7; 
   else tmp[1] = first;

   first = ((data>>4) & 0x0F) | 48;
   if (first > 57) tmp[2] = first + (byte)7; 
   else tmp[2] = first;

   first = (data & 0x0F) | 48;
   if (first > 57) tmp[3] = first + (byte)7; 
   else tmp[3] = first;
   
   tmp[4] = 0;
   Serial.print(tmp);
}
