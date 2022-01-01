#include <Keypad.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

#define BIGRELAY A3
#define SMALLRELAY A4

int espPin = A5;
int hallPin = A2;
int ledRed = A1;
int ledGreen = A0;

const int ROW_NUM = 3; //three rows
const int COLUMN_NUM = 3; //three columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'}
};

byte pin_rows[ROW_NUM] = {9, 8, 7}; //connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {6, 5, 4}; //connect to the column pinouts of the keypad

char picturenum;
char s;
int stage;
int wasOpened;

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );
SoftwareSerial espSlave(2,3); // rx, tx

File myFile;

void setup(){
  stage = 1;
  wasOpened = 0;

   // initialize the SMALLRELAY pin as an output:
  pinMode(SMALLRELAY, OUTPUT);
  digitalWrite(SMALLRELAY, LOW);

  // initialize the BIGRELAY pin as an output:
  pinMode(BIGRELAY, OUTPUT);
  digitalWrite(BIGRELAY, LOW);

  // initialize hall sensor pin as an input:
  pinMode(hallPin, INPUT);

  //initialize indicator lights:
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, LOW);

  // initialize pins to communicate with ESP32 CAM:
  pinMode(espPin,INPUT);
  espSlave.begin(115200);
  Serial.begin(9600);
  
  // initialize SD card to store homeroom:
  //Serial.print(F("Initializing SD card..."));
  if (!SD.begin(10)) {
    //Serial.println(F("initialization failed!"));
    while (1);
  }
  //Serial.println(F("initialization done."));
}


char storeHomeroom(char key){
  //save homeroom to sd card
  myFile = SD.open("data.txt", FILE_WRITE);
  
  if (myFile) {
    myFile.println("Homeroom: " + String(key));

  myFile.close();
  //Serial.println(F("Homeroom saved"));
  } 
  else {
  // if the file didn't open, print an error:
  //Serial.println(F("error opening data.txt for homeroom storage"));
  }
}

char storeWasteType(char key){
  //save waste type to sd card
  myFile = SD.open("data.txt", FILE_WRITE);
  
  if (myFile) {
    if (key == '1'){
      myFile.println("Waste Type: General Waste");
    }
    if (key == '2'){
      myFile.println("Waste Type: Compostable Waste");
    }
    if (key == '3'){
      myFile.println("Waste Type: Recycle Waste" );
    }
    myFile.close();
    //Serial.println(F("Waste Type saved"));
  } 
  else {
  // if the file didn't open, print an error:
  //Serial.println(F("error opening data.txt for waste type storage"));
  }
}


void takePictureStoreNum(){
   int done = 0;
   myFile = SD.open("data.txt", FILE_WRITE);
   
   if (myFile){
    // sends HIGH to ESP32 to wake up and take picture
    pinMode(espPin, OUTPUT);
    digitalWrite(espPin,HIGH);
    delay(100);
    digitalWrite(espPin,LOW);
    pinMode(espPin, INPUT);
    //Serial.println(F("Sent data to take picture"));

    while(done < 3){
      if (espSlave.available()){
        s = espSlave.read();
        if (s == '#'){
          done += 1;
        }
        myFile.print(s);
      }
    }
   }
   else{
    //Serial.println(F("error opening data.txt for picture number"));
   }
   myFile.close();
   //Serial.println(F("picture number saved"));
}

void unlockLayer(){
  digitalWrite(BIGRELAY, HIGH);
  delay(2000);
  digitalWrite(BIGRELAY, LOW);
}



void loop(){

  char key = keypad.getKey();
  

  if((key == '5' or key == '6') && stage == 1){
    storeHomeroom(key);
    digitalWrite(ledRed, HIGH);
    stage += 1;
    //Serial.println(F("Stage 1 complete"));
  }

  if((key == '1' or key == '2' or key == '3') && stage == 2){
    storeWasteType(key);
    digitalWrite(ledGreen, HIGH);
    digitalWrite(SMALLRELAY, HIGH);
    while (wasOpened == 0){
      if (digitalRead(hallPin) == 1){
        wasOpened += 1;
      }
    }
    delay(500);
    digitalWrite(SMALLRELAY, LOW);
    while (wasOpened == 1){
      if (digitalRead(hallPin) == 0){
        wasOpened += 1;
      }
    }
    stage += 1;
    //Serial.println(F("Stage 2 complete"));
  }
   
  if(stage == 3){
    takePictureStoreNum(); 
    wasOpened = 0;
    delay(2000);
    stage += 1;
    //Serial.println(F("Stage 3 complete"));
   }

  if(stage == 4){
    unlockLayer();
    digitalWrite(ledRed, LOW);
    digitalWrite(ledGreen, LOW);
    stage = 1;
    //Serial.println(F("Stage 4 complete"));
   }

}
