/*
 * Copyright ⓒ 2019 Dawid Maliszewski (thedavesky) <dawid@thedavesky.com>
 *
 * Arduino Smart Home Gateway v0.1.1
 * An advanced MySensors gateway code that supports RGB strip (with modes and animations), LED bulbs, relays, buttons, 1-Wire, DHT and radio at the same time.
 *
 * This file is part of arduino-smart-home-gateway (https://github.com/thedavesky/arduino-smart-home-gateway)
 *
 * Require libraries:
 * - MySensors
 * - EEPROM
 * - Dallas Temperature
 * - Adafruit DHT
 * - Bounce 2
 *
 * Compatible with:
 * - openHAB2
 * - openHAB2 Ambilight Screen
 * - Homebridge
 * - Homebridge Plugin For openHAB2 - Complete Edition
 *
 * GNU General Public License v3 (GPL-3)
 */

// Physical pins settings
#define MY_RF24_IRQ_PIN   2   // NRF24L01+ IRQ pin
#define OneWire_Pin       3   // DS18B20 data pin
#define DHT_Pin           4   // DHT data pin
#define RGB_Red_Pin       5   // RGB strip's red transistor pin
#define RGB_Green_Pin     6   // RGB strip's green transistor pin
#define MY_RF24_CS_PIN    7   // NRF24L01+ CS pin
#define MY_RF24_CE_PIN    8   // NRF24L01+ CE pin
#define RGB_Blue_Pin      9   // RGB strip's blue transistor pin
#define LightBulbs_Pin    10  // Light Bulbs' transistor pin
#define Button1_Pin       A0  // First button pin
#define Button2_Pin       A1  // Second button pin
#define MonitorRelay_Pin  A2  // Monitor relay pin
#define LightRelay_Pin    A3  // Light power supply relay pin

// NRF24L01+ settings
#define MY_RF24_CHANNEL 0             // Channel setting (0 = 2400MHz)
#define MY_RF24_PA_LEVEL RF24_PA_MIN  // Amplifier setting (MIN for bigger modules with built in amplifier, HIGH for mini modules)

// Delays' settings
unsigned long SensorsUpdate_Interval = 60000;                 // Sensors update interval (ms)
unsigned long LightOnDelay_Interval = 5400000;                // After turning off light relay after this time additional delay will be added to powering on time (ms)
unsigned long LightOn_Delay = 600000;                         // Delay of turning on to charge the capacitor in power supply (μs)
unsigned long RGBProg0_LightBulbs_NormalFade_Interval = 1500; // RGB strip program 0 and light bulbs normal fade interval (μs)
unsigned long RGBProg0_ShortFade_Interval = 200;              // RGB strip program 0 short fade interval (μs)
unsigned long RGBProg0_ShortFade_Delay = 500000;              // RGB strip program 0 short fade delay (μs)
unsigned long RGBProg1_Fade_Interval = 16000;                 // RGB strip program 1 fade interval (μs)
unsigned long RGB_Saving_Delay = 2000000;                     // Delay before saving RGB strip values to EEPROM (μs)
unsigned long RGB_RelayDisabling_Delay = 5000000;             // Delay before light relay disabling (μs)
unsigned long Button1_Delay = 500;                            // Multifunction button 1 delay (ms)

/*
 *  End of settings, if you aren't a programmer, don't change anything below!
 */

// USB communication
#define MY_GATEWAY_SERIAL

// NRF24L01+ radio communication
#define MY_RADIO_RF24
#define MY_RX_MESSAGE_BUFFER_FEATURE

// Include libraries
#include <MySensors.h>
#include <EEPROM.h>
#include <DHT.h>
#include <DallasTemperature.h>
#include <Bounce2.h>

// Define DHT
DHT dht;

// Define OneWire
OneWire oneWire(OneWire_Pin);
DallasTemperature sensors(&oneWire);

// Define buttons
Bounce BounceButton1 = Bounce();
Bounce BounceButton2 = Bounce();

// Configure sending messages to the openHAB server
MyMessage SendRGBStatus           (1, V_STATUS);
MyMessage SendRGBColor            (1, V_RGB);
MyMessage SendRGBProgram          (1, V_VAR1);
MyMessage SendLightBulbsLevel     (2, V_DIMMER);
MyMessage SendMonitorRelayStatus  (3, V_STATUS);
MyMessage SendDSTemp              (4, V_TEMP);
MyMessage SendDHTTemp             (5, V_TEMP);
MyMessage SendDHTHum              (6, V_HUM);

// Load data from EEPROM
byte  Red_TempVal = EEPROM.read(10);
byte  Green_TempVal = EEPROM.read(11);
byte  Blue_TempVal = EEPROM.read(12);
short RGB_Status = EEPROM.read(13);
short RGB_Program = EEPROM.read(14);
char  RGB_Color[] = {(char)EEPROM.read(15), (char)EEPROM.read(16), (char)EEPROM.read(17), (char)EEPROM.read(18), (char)EEPROM.read(19), (char)EEPROM.read(20)};
short LightBulbs_Level = EEPROM.read(21);
short MonitorRelay_Status = EEPROM.read(22);

// Default program variables
unsigned long Sensors_LastFirstChange = 0;
unsigned long Sensors_LastSecondChange = 0;
unsigned long DS_Interval = 0;
unsigned long RGB_LastChange = 0;
unsigned long LightBulbs_LastChange = 0;
unsigned long LightRelay_LastChange = 0;
unsigned long RGB_Interval = 0;
unsigned long RGB_OnDelay = 0;
unsigned long LightBulbs_OnDelay = 0;
unsigned long Button_LastFirstChange = 0;
unsigned long Button_LastSecondChange = 0;
bool  RGB_Changed = false;
bool  RGB_ShortFade = false;
bool  RGB_ToSave = false;
bool  LightBulbs_Changed = false;
bool  LightRelay_ChangedToOff = false;
short LightRelay_Status = 0;
short RGB_Cycles = 0;
short LightBulbs_Cycles = 0;
short RGBProg1_Mode = 0;
byte  Red_NowVal = 0;
byte  Green_NowVal = 0;
byte  Blue_NowVal = 0;
byte  LightBulbs_NowVal = 0;
byte  Red_OldVal = 0;
byte  Green_OldVal = 0;
byte  Blue_OldVal = 0;
byte  LightBulbs_OldVal = 0;
float Red_Diff = 0;
float Blue_Diff = 0;
float Green_Diff = 0;
float LightBulbs_Diff = 0;

// Do before MySensors library start
void before()
{
  // Pins modes
  pinMode(RGB_Red_Pin, OUTPUT);
  pinMode(RGB_Green_Pin, OUTPUT);
  pinMode(RGB_Blue_Pin, OUTPUT);
  pinMode(LightBulbs_Pin, OUTPUT);
  pinMode(MonitorRelay_Pin, OUTPUT);
  pinMode(LightRelay_Pin, OUTPUT);
  pinMode(Button1_Pin, INPUT_PULLUP);
  pinMode(Button2_Pin, INPUT_PULLUP);

  // Buttons modes
  BounceButton1.attach(Button1_Pin);
  BounceButton1.interval(5);
  BounceButton2.attach(Button2_Pin);
  BounceButton2.interval(5);

  // DHT mode
  dht.setup(DHT_Pin);

  // Sensors modes
  sensors.begin();
  sensors.setWaitForConversion(false);
  DS_Interval = sensors.millisToWaitForConversion(sensors.getResolution());
}

// Present all hardware informations
void presentation()
{
  // Present sketch information
  sendSketchInfo("Arduino Gateway", "0.1.1");

  // Present sensors
  present(1, S_RGB_LIGHT, "RGB strip (MyRoom)");
  present(2, S_DIMMER, "LED bulbs (MyRoom)");
  present(3, S_BINARY, "Screen (MyRoom)");
  present(4, S_TEMP, "Temperature (Balcony)");
  present(5, S_TEMP, "Temperature (MyRoom)");
  present(6, S_HUM, "Humidity (MyRoom)");
}

// Do after MySensors library start
void setup()
{
  // Startup delay
  wait(1000);

  // Set last values
  if (RGB_Program == 0)
  {
    RGBProg0_Set();
  }
  else if (RGB_Program == 1)
  {
    RGB_Changed = true;
  }
  LightBulbs_Set();
  digitalWrite(MonitorRelay_Pin, !MonitorRelay_Status);

  // Update statuses on the OpenHab server
  send(SendRGBStatus.set(RGB_Status));
  send(SendRGBColor.set((RGB_Status == 0)?"000000":RGB_Color));
  send(SendRGBProgram.set(RGB_Program));
  send(SendLightBulbsLevel.set(LightBulbs_Level));
  send(SendMonitorRelayStatus.set(MonitorRelay_Status));
}

// Process the information received from the OpenHab server
void receive(const MyMessage &message)
{
  // Verify server
  if (message.destination == 0 && message.sender == 0)
  {
    // Set RGB strip's status
    if (message.type == V_STATUS && message.sensor == 1)
    {
      RGB_Status = atoi(message.data);
      // Send color information if RGB strip is on
      if (RGB_Status == 1)
      {
        send(SendRGBColor.set(RGB_Color));
      }

      // Set RGB strip
      if (RGB_Program == 0)
      {
        RGBProg0_Set();
      }
      else if (RGB_Program == 1)
      {
        RGBProg1_Mode = RGB_Status?0:1;
        RGB_Changed = true;
      }

      // Save state to EEPROM
      EEPROM.update(13, RGB_Status);
    }

    // Set RGB strip's color
    else if (message.type == V_RGB && message.sensor == 1)
    {
      long HexTempVal = (long)strtol(message.data, NULL, 16);
      strncpy(RGB_Color, message.data, 6);

      // Convert HEX to RGB
      Red_TempVal = (HexTempVal>>16) & 0xFF;
      Green_TempVal = (HexTempVal>>8) & 0xFF;
      Blue_TempVal = HexTempVal & 0xFF;

      // Short the interval if the color has been changed recently, RGB strip is on and RGB program doesn't change
      if ((unsigned long)(micros()-RGB_LastChange) <= RGBProg0_ShortFade_Delay && RGB_Status == 1 && RGB_Program == 0)
      {
        RGB_ShortFade = true;
      }

      // Turn on RGB strip if it's off
      if (RGB_Status == 0 && (Red_TempVal != 0 || Green_TempVal != 0 || Blue_TempVal != 0))
      {
        RGB_Status = 1;
        send(SendRGBStatus.set(RGB_Status));
      }

      // Set program to 0 if it's set different
      if (RGB_Program != 0)
      {
        RGB_Program = 0;
        send(SendRGBProgram.set(RGB_Program));
      }

      // Set RGB strip
      RGBProg0_Set();

      // Save state to EEPROM
      RGB_ToSave = true;
    }

    // Set RGB strip's program
    else if (message.type == V_VAR1 && message.sensor == 1)
    {
      RGB_Program = atoi(message.data);

      // Turn on RGB strip if it's off
      if (RGB_Status == 0)
      {
        RGB_Status = 1;
        send(SendRGBStatus.set(RGB_Status));
        send(SendRGBColor.set(RGB_Color));
        EEPROM.update(13, RGB_Status);
      }

      // Set RGB strip
      if (RGB_Program == 0)
      {
        RGBProg0_Set();
      }
      else if (RGB_Program == 1)
      {
        RGBProg1_Mode = 0;
        RGB_Changed = true;
      }

      // Save state to EEPROM
      EEPROM.update(14, RGB_Program);
    }

    // Set light bulbs brightness
    else if (message.type == V_DIMMER && message.sensor == 2)
    {
      LightBulbs_Level = atoi(message.data);

      // Set light bulbs
      LightBulbs_Set();

      // Save state to EEPROM
      EEPROM.update(21, LightBulbs_Level);
    }

    // Set monitor relay status
    else if (message.type == V_STATUS && message.sensor == 3)
    {
      MonitorRelay_Status = atoi(message.data);

      // Set monitor relay
      digitalWrite(MonitorRelay_Pin, !MonitorRelay_Status);

      // Save state to EEPROM
      EEPROM.update(22, MonitorRelay_Status);
    }
  }
}

// Set RGB strip's data in program 0
void RGBProg0_Set()
{
  // Set new color values
  Red_NowVal = RGB_Status*Red_TempVal;
  Green_NowVal = RGB_Status*Green_TempVal;
  Blue_NowVal = RGB_Status*Blue_TempVal;

  // Calculate color difference for animation
  Red_Diff = (Red_NowVal-Red_OldVal)/255.;
  Blue_Diff = (Blue_NowVal-Blue_OldVal)/255.;
  Green_Diff = (Green_NowVal-Green_OldVal)/255.;

  // Start changing color if new color is different
  if (Red_Diff != 0 || Blue_Diff != 0 || Green_Diff != 0)
  {
    RGB_Changed = true;
  }

  // If the color has not changed then set last change
  else
  {
    RGB_LastChange = micros();
  }
}

// Set RGB strip's data in program 1
void RGBProg1_Set(short Select, short Time, byte Red, byte Green, byte Blue)
{
  // Set new color values
  Red_NowVal = Red;
  Green_NowVal = Green;
  Blue_NowVal = Blue;

  // Calculate color difference for animation
  Red_Diff = (Red_NowVal-Red_OldVal)/255.;
  Blue_Diff = (Blue_NowVal-Blue_OldVal)/255.;
  Green_Diff = (Green_NowVal-Green_OldVal)/255.;

  // Set program data
  RGBProg1_Mode = Select;
  RGB_Interval = Time;
  RGB_Cycles = 255;
}

// Set RGB strip's PWM pins
void RGB_Set()
{
  analogWrite(RGB_Red_Pin, Red_OldVal);
  analogWrite(RGB_Green_Pin, Green_OldVal);
  analogWrite(RGB_Blue_Pin, Blue_OldVal);
}

// Set light bulbs data
void LightBulbs_Set()
{
  // Set new brightness value
  LightBulbs_NowVal = 2.55*LightBulbs_Level;

  // Calculate brightness difference for animation
  LightBulbs_Diff = (LightBulbs_NowVal-LightBulbs_OldVal)/255.;

  // Start changing brightness if new brightness is different
  if (LightBulbs_Diff != 0)
  {
    LightBulbs_Changed = true;
  }
}

// Turn on light relay
void LightRelay_On()
{
  // Stop turning off if it is planned
  if (LightRelay_ChangedToOff == true)
  {
    LightRelay_ChangedToOff = false;
  }

  // Turn on relay if all lights is on and relay is off
  if (LightRelay_Status == 0 && (Red_NowVal != 0 || Green_NowVal != 0 || Blue_NowVal != 0 || LightBulbs_NowVal != 0))
  {
    LightRelay_Status = 1;
    digitalWrite(LightRelay_Pin, !LightRelay_Status);
    LightRelay_LastChange = millis();
  }
}

// Turn off light relay
void LightRelay_Off()
{
  LightRelay_ChangedToOff = true;
}

// Turn on RGB strip
void RGB_On()
{
  // Set delay turning on
  if ((unsigned long)(millis()-LightRelay_LastChange) >= LightOnDelay_Interval && LightRelay_Status == 0 && RGB_OnDelay == 0)
  {
    RGB_OnDelay = LightOn_Delay-RGBProg0_LightBulbs_NormalFade_Interval;
  }
  else if (RGB_OnDelay != 0)
  {
    RGB_OnDelay = LightBulbs_OnDelay;
  }

  // Set RGB strip's status to on if RGB strip is off
  if (RGB_Status == 0 && (Red_NowVal != 0 || Green_NowVal != 0 || Blue_NowVal != 0))
  {
    RGB_Status = 1;
    send(SendRGBStatus.set(RGB_Status));
    EEPROM.update(13, RGB_Status);
  }

  // Turn on light relay
  LightRelay_On();
}

// Turn off RGB strip
void RGB_Off()
{
  // Set RGB strip's status to off if RGB strip is on
  if (RGB_Status == 1 && Red_NowVal == 0 && Green_NowVal == 0 && Blue_NowVal == 0)
  {
    RGB_Status = 0;
    send(SendRGBStatus.set(RGB_Status));
    EEPROM.update(13, RGB_Status);
  }

  // Turn off light relay
  LightRelay_Off();
}

// Continuous code
void loop()
{
  // Update sensors
  if ((unsigned long)(millis()-Sensors_LastFirstChange) >= SensorsUpdate_Interval-DS_Interval)
  {
    // Request for read OneWire temperature before sending it
    if (Sensors_LastSecondChange == 0)
    {
      sensors.requestTemperatures();
      DS_Interval = sensors.millisToWaitForConversion(sensors.getResolution());
      Sensors_LastSecondChange = millis();
    }

    // Send all sensors data after reading temperature from OneWire
    else if ((unsigned long)(millis()-Sensors_LastSecondChange) >= DS_Interval)
    {
      Sensors_LastFirstChange = millis();
      Sensors_LastSecondChange = 0;

      // Send OneWire temperature
      float DSTemp = static_cast<float>(static_cast<int>(sensors.getTempCByIndex(0)*10.))/10.;
      if (DSTemp >= -55. && DSTemp <= 125.)
      {
        send(SendDSTemp.set(DSTemp, 1));
      }

      // Request and send DHT temperature
      dht.readSensor(true);
      float DHTTemp = dht.getTemperature();
      if (!isnan(DHTTemp))
      {
        send(SendDHTTemp.set(DHTTemp, 1));
      }

      // Request and send DHT humidity
      float DHTHum = dht.getHumidity();
      if (!isnan(DHTHum))
      {
        send(SendDHTHum.set(DHTHum, 1));
      }
    }
  }

  // RGB strip program 0's fade
  if (RGB_Program == 0)
  {
    // Reset fading if color is changed
    if (RGB_Changed == true)
    {
      RGB_Changed = false;
      RGB_Cycles = 255;
      RGB_On();
      if (RGB_OnDelay != 0)
      {
        RGB_LastChange = micros();
      }
    }

    // Do color fading
    if (RGB_Cycles >= 0)
    {
      if ((unsigned long)(micros()-RGB_LastChange) >= (RGB_ShortFade?RGBProg0_ShortFade_Interval:RGB_OnDelay+RGBProg0_LightBulbs_NormalFade_Interval))
      {
        RGB_LastChange = micros();
        if (RGB_OnDelay != 0)
        {
          RGB_OnDelay = 0;
        }
        if (RGB_Cycles > 0)
        {
          // Set the color values according to the remaining number of cycles
          Red_OldVal = Red_NowVal-Red_Diff*RGB_Cycles;
          Blue_OldVal = Blue_NowVal-Blue_Diff*RGB_Cycles;
          Green_OldVal = Green_NowVal-Green_Diff*RGB_Cycles;

          // Smooth the colors when difference is low
          if (Red_OldVal < 1 && Red_Diff != 0)
          {
            Red_OldVal = 1;
          }
          if (Blue_OldVal < 1 && Blue_Diff != 0)
          {
            Blue_OldVal = 1;
          }
          if (Green_OldVal < 1 && Green_Diff != 0)
          {
            Green_OldVal = 1;
          }

          RGB_Set();
        }
        else
        {
          // The cycles are over, set the final colors
          RGB_ShortFade = false;
          Red_OldVal = Red_NowVal;
          Green_OldVal = Green_NowVal;
          Blue_OldVal = Blue_NowVal;
          RGB_Set();
          RGB_Off();
        }
        RGB_Cycles--;
      }
    }
  }

  // RGB strip program 1's fade
  else if (RGB_Program == 1)
  {
    // Reset fading if color is changed
    if (RGB_Changed == true)
    {
      RGB_Changed = false;

      // Modes
      switch (RGBProg1_Mode)
      {
      // Power on mode
      case 0:
        if (RGB_Status == 1)
        {
          RGBProg1_Set(3, RGBProg0_LightBulbs_NormalFade_Interval, 0, 0, 255);
        }
        break;

      // Power off mode
      case 1:
        RGBProg1_Set(1, RGBProg0_LightBulbs_NormalFade_Interval, 0, 0, 0);
        break;

      // Fuchsia mode
      case 3:
        RGBProg1_Set(4, RGBProg1_Fade_Interval, 255, 0, 255);
        break;

      // Red mode
      case 4:
        RGBProg1_Set(5, RGBProg1_Fade_Interval, 255, 0, 0);
        break;

      // Yellow mode
      case 5:
        RGBProg1_Set(6, RGBProg1_Fade_Interval, 255, 255, 0);
        break;

      // Green mode
      case 6:
        RGBProg1_Set(7, RGBProg1_Fade_Interval, 0, 255, 0);
        break;

      // Aqua mode
      case 7:
        RGBProg1_Set(8, RGBProg1_Fade_Interval, 0, 255, 255);
        break;

      // Blue mode
      case 8:
        RGBProg1_Set(3, RGBProg1_Fade_Interval, 0, 0, 255);
        break;
      }
      RGB_On();
      if (RGB_OnDelay != 0)
      {
        RGB_LastChange = micros();
      }
    }

    // Do color fading
    if (RGB_Cycles >= 0)
    {
      if ((unsigned long)(micros()-RGB_LastChange) >= RGB_OnDelay+RGB_Interval)
      {
        RGB_LastChange = micros();
        if (RGB_OnDelay != 0)
        {
          RGB_OnDelay = 0;
        }
        if (RGB_Cycles > 0)
        {
          // Set the color values according to the remaining number of cycles
          Red_OldVal = Red_NowVal-Red_Diff*RGB_Cycles;
          Green_OldVal = Green_NowVal-Green_Diff*RGB_Cycles;
          Blue_OldVal = Blue_NowVal-Blue_Diff*RGB_Cycles;
          RGB_Set();
        }
        else
        {
          // The cycles are over, set the final colors
          Red_OldVal = Red_NowVal;
          Green_OldVal = Green_NowVal;
          Blue_OldVal = Blue_NowVal;
          RGB_Set();

          // If this isn't turn off cycle, continue changing the colors
          if (RGBProg1_Mode != 1)
          {
            RGB_Changed = true;
          }
          RGB_Off();
        }
        RGB_Cycles--;
      }
    }
  }

  // Save RGB state to EEPROM after some time
  if (RGB_ToSave == true)
  {
    if ((unsigned long)(micros()-RGB_LastChange) >= RGB_Saving_Delay)
    {
      RGB_ToSave = false;
      EEPROM.update(10, Red_TempVal);
      EEPROM.update(11, Green_TempVal);
      EEPROM.update(12, Blue_TempVal);
      EEPROM.update(13, RGB_Status);
      EEPROM.update(14, RGB_Program);
      EEPROM.update(15, RGB_Color[0]);
      EEPROM.update(16, RGB_Color[1]);
      EEPROM.update(17, RGB_Color[2]);
      EEPROM.update(18, RGB_Color[3]);
      EEPROM.update(19, RGB_Color[4]);
      EEPROM.update(20, RGB_Color[5]);
    }
  }

  // Turn off light relay after some time
  if (LightRelay_ChangedToOff == true)
  {
    if (LightRelay_Status == 1 && Red_NowVal == 0 && Green_NowVal == 0 && Blue_NowVal == 0 && LightBulbs_NowVal == 0)
    {
      // Turn off relay if all lights is off, relay is on and delay has passed
      if ((unsigned long)(micros()-RGB_LastChange) >= RGB_RelayDisabling_Delay)
      {
        LightRelay_ChangedToOff = false;
        LightRelay_Status = 0;
        digitalWrite(LightRelay_Pin, !LightRelay_Status);
        LightRelay_LastChange = millis();
      }
    }
    else
    {
      LightRelay_ChangedToOff = false;
    }
  }

  // Light bulbs' fade
  // Reset fading if brightness is changed
  if (LightBulbs_Changed == true)
  {
    LightBulbs_Changed = false;
    LightBulbs_Cycles = 255;

    // Set delay turning on
    if ((unsigned long)(millis()-LightRelay_LastChange) >= LightOnDelay_Interval && LightRelay_Status == 0 && RGB_OnDelay == 0)
    {
      LightBulbs_OnDelay = LightOn_Delay-RGBProg0_LightBulbs_NormalFade_Interval;
    }
    else if (RGB_OnDelay != 0)
    {
      LightBulbs_OnDelay = RGB_OnDelay;
    }
    if (LightBulbs_OnDelay != 0)
    {
      LightBulbs_LastChange = micros();
    }

    // Turn on light relay
    LightRelay_On();
  }

  // Do light bulbs brightness fading
  if (LightBulbs_Cycles >= 0)
  {
    if ((unsigned long)(micros()-LightBulbs_LastChange) >= LightBulbs_OnDelay+RGBProg0_LightBulbs_NormalFade_Interval)
    {
      LightBulbs_LastChange = micros();
      if (LightBulbs_OnDelay != 0)
      {
        LightBulbs_OnDelay = 0;
      }
      if (LightBulbs_Cycles > 0)
      {
        // Set the brightness value according to the remaining number of cycles
        LightBulbs_OldVal = LightBulbs_NowVal-LightBulbs_Diff*LightBulbs_Cycles;
        analogWrite(LightBulbs_Pin, LightBulbs_OldVal);
      }
      else
      {
        // The cycles are over, set the final brightness
        LightBulbs_OldVal = LightBulbs_NowVal;
        analogWrite(LightBulbs_Pin, LightBulbs_OldVal);
        LightRelay_Off();
      }
      LightBulbs_Cycles--;
    }
  }

  // Update the times if the status of the button 1 changes
  if (BounceButton1.update())
  {
    if (BounceButton1.read() == LOW)
    {
      Button_LastFirstChange = millis();
    }
    else
    {
      Button_LastSecondChange = millis();
    }
  }

  // Change RGB/light bulbs state if button 1 has been clicked
  if (Button_LastFirstChange > 0 && Button_LastSecondChange > 0)
  {
    // Change light bulbs state if button 1 has been clicked for less than clicking delay
    if ((unsigned long)(Button_LastSecondChange-Button_LastFirstChange) <= Button1_Delay)
    {
      // Turn on/off light bulbs
      LightBulbs_Level = (LightBulbs_Level > 0)?0:100;

      // Send state
      send(SendLightBulbsLevel.set(LightBulbs_Level));

      // Set light bulbs
      LightBulbs_Set();

      // Save state to EEPROM
      EEPROM.update(21, LightBulbs_Level);
    }

    // Change RGB strip's state if button 1 has been clicked for longer than clicking delay
    else
    {
      // Revert state
      RGB_Status = !RGB_Status;

      // Send informations
      send(SendRGBColor.set((RGB_Status == 0)?"000000":RGB_Color));
      send(SendRGBStatus.set(RGB_Status));

      // Set RGB strip
      if (RGB_Program == 0)
      {
        RGBProg0_Set();
      }
      else if (RGB_Program == 1)
      {
        RGBProg1_Mode = RGB_Status?0:1;
        RGB_Changed = true;
      }

      // Save state to EEPROM
      EEPROM.update(13, RGB_Status);
    }
    Button_LastSecondChange = 0;
    Button_LastFirstChange = 0;
  }

  // Change monitor relay state if button 2 has been clicked
  if (BounceButton2.update() && BounceButton2.read() == LOW)
  {
    // Revert state
    MonitorRelay_Status = !MonitorRelay_Status;

    // Send state
    send(SendMonitorRelayStatus.set(MonitorRelay_Status));

    // Set monitor relay
    digitalWrite(MonitorRelay_Pin, !MonitorRelay_Status);

    // Save state to EEPROM
    EEPROM.update(22, MonitorRelay_Status);
  }
}
