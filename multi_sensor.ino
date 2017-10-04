#include <Wire.h>
#include <SFE_BMP180.h>
#include "RTClib.h"
#include <SPI.h>

// SFE_BMP180 object, called "pressure":
SFE_BMP180 pressure;

// how many milliseconds between grabbing data and logging it. 1000 ms is once a second
#define LOG_INTERVAL  1000 // mills between entries (reduce to take more/faster data)

// how many milliseconds before writing the logged data permanently to disk
// set it to the LOG_INTERVAL to write each time (safest)
// set it to 10*LOG_INTERVAL to write all data every 10 datareads, you could lose up to
// the last 10 reads if power is lost but it uses less power and is much faster!
#define SYNC_INTERVAL 1000 // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()

// the digital pins that connect to the LEDs
#define redLEDpin 3
#define greenLEDpin 2

// The analog pins that connect to the sensors
#define photocellPin 0           // analog 0
#define tempPin 1                // analog 1
#define BANDGAPREF 14            // special indicator that we want to measure the bandgap

#define aref_voltage 3.3         // we tie 3.3V to ARef and measure it with a multimeter!
#define bandgap_voltage 1.1      // this is not guaranteed but its not -too- off

RTC_DS1307 RTC; // define the Real Time Clock object

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

int avail = 0;

char t_str[] = "GET";

// Initialize read buffer
char buff[64];

int DHpin = 8;
byte dat [5];

/*!
 * \brief Read data from the DTH11 Sensor
 * \return
 */
inline byte read_data () {
  byte data;
  for (int i = 0; i < 8; i ++) {
    if (digitalRead (DHpin) == LOW) {
      while (digitalRead (DHpin) == LOW); // wait for 50us
      delayMicroseconds (30); // determine the duration of the high level to determine the data is '0 'or '1'
      if (digitalRead (DHpin) == HIGH)
        data |= (1 << (7 - i)); // high front and low in the post
      while (digitalRead (DHpin) == HIGH); // data '1 ', wait for the next one receiver
    }
  }
  return data;
}

/*!
 * \brief Read Humidity data from the DTH11 Sensor
 *
 * \return Humidity data in percentage (%)
 */
inline String DTH11Loop () {
  digitalWrite (DHpin, LOW); // bus down, send start signal
  delay (30); // delay greater than 18ms, so DHT11 start signal can be detected

  digitalWrite (DHpin, HIGH);
  delayMicroseconds (40); // Wait for DHT11 response

  pinMode (DHpin, INPUT);
  while (digitalRead (DHpin) == HIGH);
  delayMicroseconds (80); // DHT11 response, pulled the bus 80us
  if (digitalRead (DHpin) == LOW);
  delayMicroseconds (80); // DHT11 80us after the bus pulled to start sending data

  for (int i = 0; i < 4; i ++) // receive temperature and humidity data, the parity bit is not considered
    dat[i] = read_data ();

  pinMode (DHpin, OUTPUT);
  digitalWrite (DHpin, HIGH); // send data once after releasing the bus, wait for the host to open the next Start signal

  // Humidity value in percentage
  String out_str = String(dat [0], DEC);
  out_str += '.';
  out_str += String(dat [1], DEC);
//  out_str += '%';

//  Temperature in Farenheit, seems to be less precise than analog temp. sensor
//  out_str += " Temperature =";
//  out_str += String((dat [2] * 9 / 5) + 32, DEC);
//  out_str += '.';
//  out_str += String(dat [3], DEC);
//  out_str += 'F';

  return out_str;
}

/*!
 * \brief Read data from the SFE BMP180 Barometric Pressure Sensors
 *
 * \param pressure_sensor
 *
 * The pressure sensors class that is currently active
 *
 * \return Barometric pressure in millibar
 */
inline String BaroRead(SFE_BMP180 &pressure_sensor) {
  char status;
  double P = 0, T = 0;

  status = pressure_sensor.startPressure(3);
  if (status != 0) {
    delay(status);

    status = pressure_sensor.getPressure(P, T);
    if (status != 0) {

      // Pressure in millibar
      String out_str = String(P, 2);
      // Pressure in inch Hg
      // String out_str += String(P * 0.0295333727, 2);

      return out_str;
    } else {
      return "";
    }
  }
}

/*!
 * \brief Read the current from the Real Time Clock
 *
 * \param rtc_clock
 *
 * Real Time Clock class that is currently active
 *
 * \return The current time in the following format
 * Year/Month/Day Hour:Minute:Second
 */
inline String TimeRead( RTC_DS1307& rtc_clock ) {

  auto now = rtc_clock.now();

  String time_str = String(now.year(), DEC);
  time_str += "/";
  time_str += String(now.month(), DEC);
  time_str += "/";
  time_str += String(now.day(), DEC);
  time_str += " ";
  time_str += String(now.hour(), DEC);
  time_str += ":";
  time_str += String(now.minute(), DEC);
  time_str += ":";
  time_str += String(now.second(), DEC);

  return time_str;
}

/*!
 * \brief Read data from and analog light sensor
 *
 * \param light_sensor_pin
 *
 * Pin that the light sensor is currently attached to
 *
 * \return Light sensor reading in arbitrary units
 */
inline String LightSensorRead( int light_sensor_pin ) {

  analogRead( light_sensor_pin );
  delay(10);
  int photocellReading = analogRead( light_sensor_pin );

  //Light in arbitrary units ~ 0 - 1000
  String data_str = String( photocellReading, DEC );

  return data_str;

}

/*!
 * \brief Read data from analog three pin temperature sensor
 *
 * \param temp_sensor_pin
 *
 * Pin that the temperature sensor is currently attached to
 *
 * \param ref_voltage
 *
 * External reference voltage used to calibrate sensor
 *
 * \return Temperature sensor reading in degree Farenheit
 */
inline String TemperatureRead( int temp_sensor_pin, float ref_voltage ) {

  analogRead( temp_sensor_pin );
  delay(10);
  int tempReading = analogRead( temp_sensor_pin );

  // converting that reading to voltage, for 3.3v arduino use 3.3, for 5.0, use 5.0
  float voltage = tempReading * ref_voltage / 1024;
  float temperatureC = (voltage - 0.5) * 100 ;
  float temperatureF = (temperatureC * 9 / 5) + 32;

  String data_str = String(temperatureF);

  return data_str;
}

void setup(void)
{
  Serial.begin(115200);

  // use debugging LEDs
  pinMode(redLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);
  pinMode (DHpin, OUTPUT);

  digitalWrite(redLEDpin, HIGH);
  delay(10);

  // connect to RTC
  Wire.begin();

  if (!pressure.begin()) {
    Serial.println("Problem connecting to pressure sensor");
  }

  if (!RTC.begin()) {
    Serial.println("RTC failed");
  } else {
    //adjust RTC time based on time as read from uploading computer
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  digitalWrite(redLEDpin, LOW);

  // If you want to set the aref to something other than 5v
  analogReference(EXTERNAL);
}

void loop()
{
  DateTime now;

  // log milliseconds since starting
  uint32_t m = millis();

  avail = Serial.available(); //Serial.available is NOT a reliable way to measure the number of bytes in the serial buffer, it is only good at distinquishing between 0 bytes and >0 bytes.

  if (avail > 0)
  {
    digitalWrite(greenLEDpin, HIGH);

    memset(&buff[0], 0, sizeof(buff));
    Serial.readBytesUntil('\n', buff, sizeof(buff));

    if (!strcmp(buff, "LIGHT")) {

      Serial.println( LightSensorRead( photocellPin ) );

    } else if (!strcmp(buff, "TEMP")) {

      Serial.println( TemperatureRead( tempPin, aref_voltage ) );

    } else if (!strcmp(buff, "BARO")) {

      Serial.println( BaroRead(pressure) );

    } else if (!strcmp(buff, "HUMI")) {

      Serial.println( DTH11Loop() );

    } else if (!strcmp(buff, "TIME")) {

      Serial.println( TimeRead( RTC ) );

    } else {
      //
    }

    // Log the estimated 'VCC' voltage by measuring the internal 1.1v ref
    analogRead(BANDGAPREF);
    delay(10);
    int refReading = analogRead(BANDGAPREF);
    float supplyvoltage = (bandgap_voltage * 1024) / refReading;

    digitalWrite(greenLEDpin, LOW);
  }

}

