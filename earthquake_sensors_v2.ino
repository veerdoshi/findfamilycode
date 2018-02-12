#include "Adafruit_FONA.h"
#include <LiquidCrystal.h>
#include<Wire.h>

//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
float earthquakesign;
float earthquakestart;
const int MPU_addr=0x68;  // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ;
//int16_t Tmp,GyX,GyY,GyZ;

#define sendto "14082187162"
#define FONA_RX  3
#define FONA_TX  2
#define FONA_RST 9

char replybuffer[255];

#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA_3G fona = Adafruit_FONA_3G(FONA_RST);

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);
uint8_t type;    // placeholder to return type of FONA

uint8_t gStatus; // FONA GPRS status
uint8_t nStatus; // FONA network status
uint8_t sStatus; // FONA power status
uint8_t dStatus; // FONA search status

// RSSI variables
uint8_t n; // RSSI status
int8_t r;  // signal mapped to dBm

// for sending piecemeal AT commands

char apn[] = "NXTGENPHONE";
int port = 80;
int8_t answer;
int data_size = 0;
int end_file = 0;
char aux_str[100];
char tmp_str[100];
char data[800];
int x = 1;
long previous;
int magnitude;

void setup() {


  Serial.begin(115200);
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true); 


  gStatus; // GPRS status
  nStatus; // Network status
  sStatus; // FONA status
  
  Serial.println(F("Ready"));
  delay(2000);
  
  // Turn FONA on
  fonaOn(); //sets sStatus to 1 if fona is turned on successfully
  
  
  if(sStatus == 1) { // FONA is on
    Serial.println(F("Delay 15 sec. ")); 
    delay(10000); // Give FONA time to register 
    // Check registration
    fonaReg();
    
    if(nStatus == 1) { 
      // Turn GPRS on 
      Serial.println(F("GPRS on 3s "));
      delay(3000);  
      GPRS_on(); // function will set gStatus to 1 if GPRS turns on okay. 
    }
  }
  sendSMSAlert();
}

void loop() {
  // put your main code here, to run repeatedly:
  
  if(x==1) {
    sensorsetup();
    x=0;
  }
  quakechecker();
}


void sendData(){
  if(gStatus) 
  { // GPRS turned on okay
        // ready to post;
        // read parameters
        // Establish HTTP connection
        answer = sendATcommand("AT+CHTTPSSTART","OK", 5000);
        delay(3000);
        
        if (answer == 1) 
        {
          Serial.println(F("GPRS post 3s"));
          delay(2000);
          // post the string to ThingSpeak
          GPRS_post();
        }  
        else 
        {
          Serial.println("HTTP not connected");
            /////
          // Turn FONA on
           fonaOn(); //sets sStatus to 1 if fona is turned on successfully
           if(sStatus == 1) { // FONA is on
            Serial.println(F("Delay 15 sec. ")); 
            delay(5000); // Give FONA time to register 
            // Check registration
            fonaReg();
            if(nStatus == 1) { 
              // Turn GPRS on 
          //    Serial.println(F("GPRS on 3s "));
          //    delay(3000);  
              GPRS_on(); // function will set gStatus to 1 if GPRS turns on okay. 
            }
          }
    
          ////
          
        }
  } 
}

void sendSMSAlert() {
 // String location = gps();
 // String messages = location;  
 String messages = "ALERT: Earthquake in San Jose, CA. GET TO SAFETY.";
  char txt[messages.length() + 1];
  messages.toCharArray(txt, messages.length() + 1 );
  //Serial.println(txt);
    flushSerial();
    //Serial.println(sendto);
    //Serial.println(txt);
    if (!fona.sendSMS(sendto, txt)) {
    Serial.println(F("Failed"));
    } else {
    Serial.println(F("Sent!"));
    }
    delay(3000);
}

void flushSerial() {
  while (Serial.available())
    Serial.read();
}

void sensorsetup() {
/* veer
  lcd.begin(16,2);
  lcd.clear();
  lcd.print("Starting Up");
  lcd.setCursor(0,1);
  lcd.print("Sensors...");
  */
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
  AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)     
  AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  /*veer
  Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  */
  
  earthquakestart = sqrt(AcX+AcY+AcZ); 
  delay(750);
  /* veer
  for (int y = 5; y >= 0; y--) {
    lcd.clear();
    lcd.print(y);
    delay(1000);
  }
  */

}

void quakesensor() {
  
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
 
  AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)     
  AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  /* veer
  Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
 */
  /*
  Serial.print("AcX = "); Serial.print(AcX);
  Serial.print(" | AcY = "); Serial.print(AcY);
  Serial.print(" | AcZ = "); Serial.print(AcZ);
  Serial.print(" | Tmp = "); Serial.print(Tmp/340.00+36.53);  //equation for temperature in degrees C from datasheet
  Serial.print(" | GyX = "); Serial.print(GyX);
  Serial.print(" | GyY = "); Serial.print(GyY);
  Serial.print(" | GyZ = "); Serial.println(GyZ);
  */
  earthquakesign = sqrt(AcX+AcY+AcZ);
  magnitude = earthquakesign *10;
  
 // lcd.clear();
 // lcd.print(AcX); lcd.print("  "); lcd.print(AcY); lcd.print("  "); lcd.print(AcZ); lcd.print("  ");
 /* veer
    lcd.setCursor(0,1);
    lcd.print(earthquakesign);
    */
 // lcd.print(GyX); lcd.print("  "); lcd.print(GyY); lcd.print("  "); lcd.print(GyZ); lcd.print("  ");  
  /*  veer
    delay(600);
    lcd.clear();
    */
}

void alarmsound() {

  for (int z = 0; z <= 10; z++) { 
      for(double x = 0; x < 0.92; x += 0.01){
        tone(8, sinh(x+8.294), 10);
        delay(1);
      }

      for(double x = 0; x < 0.183258; x += 0.002){
        tone(8, sinh(-5* (x-1.8420681)), 10);
        delay(2);
      }
    }  
    
}

void quakechecker() {
  quakesensor();
//  Serial.print("quakesign = "); Serial.println(earthquakesign); 
  if (earthquakesign < (earthquakestart-10)) {
/* veer
    lcd.clear();
    lcd.print("|||| ALERT ||||");
    lcd.setCursor(0,1);
    lcd.print(" | EARTHQUAKE |"); 
 */ 
    alarmsound();
    sendSMSAlert();
    sendData();
    delay(2000);  
  }else if (earthquakesign > (earthquakestart+10)) {
  /* veer
    lcd.clear();
    lcd.print("|||| ALERT ||||");
    lcd.setCursor(0,1);
    lcd.print(" | EARTHQUAKE |"); 
    */
    alarmsound();
    sendSMSAlert();
    sendData();
    delay(2000);   
  }
}


void fonaOn() {

  Serial.println(F("Checking FONA"));
  fonaSerial->begin(4800);
  
  if(! fona.begin(*fonaSerial)) { // FONA is off at start of loop
    Serial.println(F("No FONA "));
    // next block added per Rick's recommendation
    // to call/check fona.begin after FONA on key toggle
    
    if (! fona.begin(*fonaSerial)) {
      Serial.println(F("Still no FONA"));
      delay(2000);
      sStatus = 0;
    }
    
    else {     
      Serial.println(F("FONA now on"));
      delay(2000);
      sStatus = 1;
    }
  }
  
  // all is good   
  else { // FONA is on at start of loop
    Serial.println(F("FONA Found"));
    delay(2000);
    sStatus = 1;
  }
}


void fonaOff() {
  
  // following line added per Adafruit recommendation
  fonaSerial->end();
  Serial.println(F("Turn FONA Off "));
  delay(2000);
  sStatus =0;
}

void GPRS_on() { 

  // Turn GPRS on //
  fona.setGPRSNetworkSettings(F("NXTGENPHONE"));
  
  Serial.println(F("Entering GPRS "));
  delay(2000);
  
  fona.setHTTPSRedirect(true);
 
  // turn GPRS on
  if (!fona.enableGPRS(true)) {
    Serial.println(F("Failed to turn on"));
    gStatus = 0;
  } 
  else {
    Serial.println(F("GPRS on"));
    gStatus = 1;
  }
}

void GPRS_post() {
  
  Serial.println(F("GPRS on; delay 5s"));
  delay(5000);
  Serial.println(F("Posting..."));
  delay(2000);
  
  char test[] = "AT+CHTTPACT=?";
  Serial.println(test);
  answer = sendATcommand(test,"+CHTTPACT: \"ADDRESS\",(1-65535)",5000);  
  Serial.println();
  
  
   
  sprintf(aux_str,"AT+CHTTPACT=\"23.23.128.216\",%d",port); 
 
  Serial.println(aux_str);
  answer = sendATcommand(aux_str, "+CHTTPACT: REQUEST", 60000);
  Serial.println();
  Serial.println(answer);
  
  // send HTTP GET
  
  sprintf(tmp_str, "{\"magnitude\":%d}", magnitude);
  sprintf(aux_str, "POST /quake/%s HTTP/1.1\r\nHost:findfamily.herokuapp.com\r\nConnection: close\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s", "sensor1", strlen(tmp_str), tmp_str);
  Serial.println(aux_str);
  
  char ctrlZ[2];
  ctrlZ[0] = 0x1A;    
  ctrlZ[1] = 0x00; 
  answer = sendATcommand(aux_str, "origin", 2000); 
  Serial.println();
  Serial.println(answer);
  answer = sendATcommand(ctrlZ,"+CHTTPACT: DATA,",60000); 
  Serial.println();
  Serial.println(answer);
  delay(5000);

  
  fona.HTTP_GET_end();
  // /// Code to POST ends /// 

} 

void GPRS_off() {

 /// Turn GPRS off ///

 if (!fona.enableGPRS(false))  {
  Serial.println(F("GPRS off bad. "));
 } 
 else {
  Serial.println(F("GPRS off ok. "));
  delay(1000);
 }
}

void callStat() {

 // ////screenClear();
 int8_t callstat = fona.getCallStatus();
 delay(2000);
}

void netStat() {
  
  ////screenClear();
  // read the network/cellular status
  nStatus = fona.getNetworkStatus();
  Serial.println(F("Network status "));
  Serial.print(nStatus);
  Serial.print(F(": "));
  // Serial.setCursor(0,1);
  if (nStatus == 0) Serial.println(F("Not reg"));
  if (nStatus == 1) Serial.println(F("Reg (home)"));
  if (nStatus == 2) Serial.println(F("Searching"));
  if (nStatus == 3) Serial.println(F("Denied"));
  if (nStatus == 4) Serial.println(F("Unknown"));
  if (nStatus == 5) Serial.println(F("Reg roam"));  
}

void fonaRSSI() {
  
  // read the RSSI
  n = fona.getRSSI();
  r;
  
  Serial.println(F("RSSI = ")); Serial.println(n); Serial.println(": ");
  if (n == 0) r = -115;
  if (n == 1) r = -111;
  if (n == 31) r = -52;
  if ((n >= 2) && (n <= 30)) {
    r = map(n, 2, 30, -110, -54);
  }
  
  Serial.println(r); Serial.println(F(" dBm"));
}

void fonaReg() {
    
  while(1) {
    netStat();
    delay(2000);
    
    if(nStatus == 1) {
      ////screenClear();
      Serial.println(F("Registered"));
      break;
    }
      
    else {        
      ////screenClear();
      fonaRSSI();
      Serial.println(F("Searching @ "));
      Serial.println(r);
      delay(3000);
      dStatus += 1;
      
      if(dStatus > 5) {
        ////screenClear();
        Serial.println(F("Time Out"));
        delay(3000);  
        break;
      }
    }    
  }
}


int8_t sendATcommand(char* ATcommand, char* expected_answer1, unsigned int timeout)
{
  uint8_t x=0,  answer=0;
  char response[100];
  unsigned long previous;
  memset(response, '\0', 100);    // Initialize the string
  delay(100);
  
  while( fonaSS.available() > 0) fonaSS.read();    // Clean the input buffer
  fonaSS.println(ATcommand);    // Send the AT command 
  x = 0;
  previous = millis();
  
  // this loop waits for the answer
  do {
  
    if(fonaSS.available() != 0){    
      if (x == 100) {
        strncpy(response, response +1, 99);
        response[99] = fonaSS.read(); 
      }
      
      else {
        response[x] = fonaSS.read();
        Serial.print(response[x]);
        x++;
      }
   
      // check if the desired answer is in the response of the module
      if (strstr(response, expected_answer1) != NULL) {
        answer = 1;
      }
    }
  }
  // Waits for the asnwer with time out
  
  while((answer == 0) && ((millis() - previous) < timeout));    
  return answer;
}
