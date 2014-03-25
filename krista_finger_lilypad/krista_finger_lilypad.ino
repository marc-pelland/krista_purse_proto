#include <Servo.h> 
#include <SoftwareSerial.h>

byte highbyte = 0;
byte lowbyte = 0; 
word checksum = 0;
byte highcheck = 0;
byte lowcheck = 0;
byte response = 0;
int state = 0;
word parameterin = 0;
word checksumReply = 0;

boolean communicationError = false;
boolean checksumCorrect = true; 
boolean ack = true;

byte lbyte = 0;
byte hbyte = 0;
byte checklbyte = 0;
byte checkhbyte = 0;

const int transmitDelay = 500;

Servo myservo;
int pos = 0;
int inPin = 10;
int buttonVal = 0;
int timer = 0;
int buttonPressed = 0;

#define rxPin 6
#define txPin 5

SoftwareSerial fingerSerial(rxPin, txPin);

void setup(){

//  fingerSerial.begin(9600); 

//  myservo.write(0);
  pinMode(inPin, INPUT);    // declare pushbutton as input
  digitalWrite(inPin, HIGH);
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  
  fingerSerial.begin(9600);
  Serial.begin(9600);  

  Serial.println(fingerSerial.available());
  
  Serial.println("-- opening the connection");
  openConnection();
  Serial.println("-- opening the connection - complete");
  blinkLED();

//  turnLock();
//  Serial.println("check enroll count");
//  getEnrollCount();
//  Serial.println("check enroll count - complete");
//turnOnLED();
}

void loop(){
   timer ++;
      Serial.println(timer);
   if (timer < 5) {
      turnOnLED();
   } else {
     turnOffLED();
   }
  
  buttonVal = digitalRead(inPin);  // read input value
  if (buttonVal == HIGH) {         // check if the input is HIGH (button released)
    digitalWrite(13, LOW);  // turn LED OFF
    timer = 0;
  } else {
    digitalWrite(13, HIGH);  // turn LED ON
  }
  
//  int i=0;
  
//  if (state == 1) { // enroll
//     enrollFinger();
//  } else if (state == 2) { // id
//    
  
    idFingerPrint();
    delay(100);
  
}

/*****************************************************************

  START METHODS FOR INTERACTION

******************************************************************/

void openConnection() { scannerCommand(0x01, 0); waitForReply(); }
void closeConnection() { scannerCommand(0x02, 0); waitForReply(); }

// Deletes all IDs (enrollments) from the database
// Returns: true if successful, false if db is empty
void deleteAll() {
  // ONLY CALL WHEN REALLY NEEDED
   scannerCommand(0x41, 0);
   waitForReply();
} 

// Deletes the specified ID (enrollment) from the database
// Parameter: 0-199 (id number to be deleted)
// Returns: true if successful, false if position invalid
void deleteById(int id) {
  scannerCommand(0x40, id);
  waitForReply();
}

// Checks the currently pressed finger against a specific ID
// Parameter: 0-199 (id number to be checked)
// Returns:
//	0 - Verified OK (the correct finger)
//	1 - Invalid Position
//	2 - ID is not in use
//	3 - Verified FALSE (not the correct finger)
void idFingerAgainstSpecific(int id) {
  scannerCommand(0x50, id);
  waitForReply();
}

// Checks the currently pressed finger against all enrolled fingerprints
// Returns:
//	0-199: Verified against the specified ID (found, and here is the ID number)
//	200: Failed to find the fingerprint in the database
int idFingerAgainstAll() {
  scannerCommand(0x51, 0);
  waitForReply();
  return parameterin;
}

void idFingerPrint() {
  
  if (isFingerPressed()) {
    
    // capture finger
    captureFinger(false);
      
    int id = idFingerAgainstAll();
    if (id < 200) {
      Serial.print("Verified ID:");
      Serial.println(id);
      
    turnLock();
      
//      blinkLED();
    } else {
      Serial.print("Finger not found - ");
      Serial.println(id);

      blinkLED();
      
    }
  }
 
}

void turnLock() {
  Serial.println("TURN LOCK");
    myservo.attach(9);
                        // in steps of 1 degree 
    myservo.write(0);              // tell servo to go to position in variable 'pos' 
    delay(150);                       // waits 15ms for the servo to reach the position 


    myservo.write(45);              // tell servo to go to position in variable 'pos' 
    delay(150);                       // waits 15ms for the servo to reach the position 


    myservo.write(0);              // tell servo to go to position in variable 'pos' 
    delay(150);           // waits 15ms for the servo to reach the position 
    
      myservo.detach();
 
}

// Checks to see if a finger is pressed on the FPS
// Return: true if finger pressed, false if not
bool isFingerPressed() {
  scannerCommand(0x26, 12);
  waitForReply();

  return (parameterin == 0);
}

// star the enrollment process by finding an id for the current user
void enrollFinger() {
  
  int enrollid = 0;
  bool okid = false;
  
  // find an open id
  for (int i = 0; i < 200; i ++) {
   if (checkEnrolled(i) == false) {

     enrollid = i;
     break;
   } 
  }
  
  Serial.println("NUMBER IS NOT ENROLLED, USE IT"); 
  startEnroll(enrollid);
  
}

// checks to see if the ID number is in use or not
// Parameter: 0-199
// Return: True if the ID number is enrolled, false if not
bool checkEnrolled(int id) {
  scannerCommand(0x21, id);
  waitForReply();
//  Serial.println(parameterin);
  return (parameterin == 0);
}



// Starts the Enrollment Process
// Parameter: 0-199
// Return:
//	0 - ACK
//	1 - Database is full
//	2 - Invalid Position
//	3 - Position(ID) is already used
void startEnroll(int id) {
  Serial.println("Starting enrollment process");
  
  // start the enrollment
  scannerCommand(0x22, id);
  waitForReply();
  
  Serial.println(">> press finger to begin");
  
  // delay while waiting for the finger press
  while(isFingerPressed() == false) delay(100);
  
  // capture a high quality finger touch
  bool bret = captureFinger(true);
  int iret = 0;
  
  // if there is a positive response (finger detected and captured);
  if (bret != false) {
    Serial.println("Remove finger");
    enroll1();
    while(isFingerPressed() == true) delay(100);
    Serial.println("Press same finger again");
    while(isFingerPressed() == false) delay(100);
    bret = captureFinger(true);
    
    if (bret != false) {
      Serial.println("Remove finger");
      enroll2();
      while(isFingerPressed() == true) delay(100);
      Serial.println("Press same finger again");
      while(isFingerPressed() == false) delay(100);
      bret = captureFinger(true);
      
      if (bret != false) {
        Serial.println("Remove finger");
        iret = enroll3();
        if (iret == 0) {
          Serial.println("Enrolling Successfull");
        } else {
          Serial.print("Enrolling Failed with error code:");
          Serial.println(iret);
        }
      } else {
        Serial.println("ERROR READING FINGER THIRD TIME");
      }
    } else {
      Serial.println("ERROR READING FINGER SECOND TIME");
    }
  } else {
    Serial.println("ERROR READING FINGER FIRST TIME");
  }
  
  blinkLED();
  
  state = 0;
  
}

// Gets the first scan of an enrollment
// Return: 
//	0 - ACK
//	1 - Enroll Failed
//	2 - Bad finger
//	3 - ID in use
void enroll1() {
  scannerCommand(0x23, 0);
  waitForReply();
}

// Gets the Second scan of an enrollment
// Return: 
//	0 - ACK
//	1 - Enroll Failed
//	2 - Bad finger
//	3 - ID in use
void enroll2() {
  scannerCommand(0x24, 0);
  waitForReply();
}

// Gets the Third scan of an enrollment
// Finishes Enrollment
// Return: 
//	0 - ACK
//	1 - Enroll Failed
//	2 - Bad finger
//	3 - ID in use
int enroll3() {
  scannerCommand(0x25, 0);
  waitForReply();
  return parameterin;
}

// Gets the number of enrolled fingerprints
// Return: The total number of enrolled fingerprints
void getEnrollCount() {
 scannerCommand(0x20, 0);
 waitForReply();
 Serial.println(response);
 Serial.println(parameterin);
}

void blinkLED() {
  turnOffLED();
  delay(500);
  turnOnLED();
  delay(500);
  turnOffLED();
  delay(500);
  turnOnLED();
  delay(500);
  turnOffLED();
}

// turn on the LED
void turnOnLED() {
//  Serial.println("TURN ON THE LED");
  scannerCommand(0x12, 1); //command turns on the LED in the sensor.
   waitForReply();
}

// turn off the LED
void turnOffLED() {
  Serial.println("TURN OFF THE LED");
  scannerCommand(0x12, 0); //command turns on the LED in the sensor.
   waitForReply();
}

// Captures the currently pressed finger into onboard ram use this prior to other commands
// Parameter: true for high quality image(slower), false for low quality image (faster)
// Generally, use high quality for enrollment, and low quality for verification/identification
// Returns: True if ok, false if no finger pressed
bool captureFinger(bool highquality) {
  
  Serial.println("capturing your finger print");
  
  if (highquality) {
    scannerCommand(0x60, 1);
  } else {
    scannerCommand(0x60, 0);
  }
  
  waitForReply();
  return (parameterin == 0);
}

/*****************************************************************

  END METHODS FOR INTERACTION

******************************************************************/


/******** SOME REALLY HELPFUL METHODS FROM SPARKFUN COMMENTs ********/


// send a message to the scanner
// @params
//	com - the byte to address
//	param - the value to send where needed
void scannerCommand(byte com, int param) { 
  valueToWORD(param);
  calcChecksum(com, highbyte, lowbyte);
  fingerSerial.write(0x55);
  fingerSerial.write(0xaa);
  fingerSerial.write(0x01);
  fingerSerial.write((byte)0);
  fingerSerial.write(lowbyte);
  fingerSerial.write(highbyte);
  fingerSerial.write((byte)0);
  fingerSerial.write((byte)0);
  fingerSerial.write(com);
  fingerSerial.write((byte)0);
  fingerSerial.write(lowcheck);
  fingerSerial.write(highcheck);
}

// receives data from the device.
void waitForReply(){ 
  communicationError = false;
  while(fingerSerial.available() == 0){}
  delay(transmitDelay);
  if(fingerSerial.read() == 0x55){
  } else {
    communicationError = true;
  }

  if(fingerSerial.read() == 0xAA){
  } else {
    communicationError = true;
  }

  if(fingerSerial.read() == 0x01){
  } else {
    communicationError = true;
  }

  if(fingerSerial.read() == 0x00){
  } else {
    communicationError = true;
  }

  lbyte = fingerSerial.read();
  hbyte = fingerSerial.read();

  parameterin = word(hbyte, lbyte);

  fingerSerial.read();
  fingerSerial.read();

  response = fingerSerial.read();

  if(response == 0x30){
    ack = true;
  } else {
    ack = false;
  }
  fingerSerial.read();

  checklbyte = fingerSerial.read();
  checkhbyte = fingerSerial.read();

  checksumReply = word(checkhbyte, checklbyte);

  if(checksumReply == 256 + lbyte + hbyte + response){
    checksumCorrect = true;
  } else {
    checksumCorrect = false;
  } 
  
}

// called from wait to verify the data
void calcChecksum(byte c, byte h, byte l){
  checksum = 256 + c + h + l; //adds up all the bytes sent
  highcheck = highByte(checksum); //then turns this checksum which is a word into 2 bytes
  lowcheck = lowByte(checksum);
}

//turns the word you put into it (the paramter in the code above) to two bytes
void valueToWORD(int v){ 
  highbyte = highByte(v); //the high byte is the first byte in the word
  lowbyte = lowByte(v); //the low byte is the last byte in the word (there are only 2 in a word)
}

