// Player Arduino for CryptoColor Game
#include <LiquidCrystal.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <RF24Network.h>
// Button pin to submit player's guess
const int guessButtonPin = A2;
// Button to delete player's guess
const int deleteButtonPin = A0;
int stateOpen = 0;

// Red and Green LED lights
const int redPin = 6;
const int greenPin = 5;
int letterCounter=0;
int cursorCounter=0;
int oldScore = 0;
int newScore = 0;
bool gameOver =true;

// Address of this node in Octal format ( 04,031, etc)
const uint16_t this_node = 02;
const uint16_t controller = 00;

// The LCD's layout is seen as the following in terms of (column, row):
//        (0, 0) . . . (15, 0)
//        (0, 1) . . . (15, 1)
const int rs = 10, en = 9, d4 = 3, d5 = 4, d6 = 7, d7 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); // Set up the LCD display
char chararr[]={'#','a','b','c','d','e','f','g','h','i','j','k','l', 'm','n','o','p','q','r','s','t','u','v','w','x','y','z' }; 
String answer ="";
char realAnswer[8] ={};
// This is the struct that will be sent to the main arduino

struct Data_Package {
  char guess[8] ={};
  int ID = 3;
  int score = 0;
  int reset = 0;
  bool gameOver =false;
 
 };

 //NRF24L01 module pins
const int NRF24L01_CE = A5;
const int NRF24L01_CS = A4;
const int NRF24L01_MOSI = 11;
const int NRF24L01_MISO = 12;
const int NRF24L01_SCK = 13;

const byte address[6] = "00001";

 // Start the RF24
RF24 radio(NRF24L01_CE, NRF24L01_CS);
// Include radio in the network
RF24Network network(radio);
RF24NetworkHeader header0(controller);


 Data_Package dataPackage;
 
// Joystick to let the player select their letters for their guess

// this one may look wrong in device, but it works
int joyX = A1;
int joyY = A3;
int joySwitch = 2;


int xPosition = 0;
int yPosition = 0;
int switchState = 0;
int mapX = 0;
int mapY = 0;
//String realAnswer="";
// The NRF24L01 - 2.4G Wireless Transceiver Module



void setup() {
  // Buttons are input
  pinMode(guessButtonPin, INPUT);
  pinMode(deleteButtonPin, INPUT);

  // LED lights are output
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
 

  // Set up the LCD's number of columns and rows
  lcd.begin(16, 2);
  lcd.print("welcome to game");
  delay(2000);

  // Set up the joystick
  Serial.begin(9600);

//
  pinMode(joyX, INPUT);
  pinMode(joyY, INPUT);
  pinMode(joySwitch, INPUT_PULLUP);

  // Connect to the Master arduino
  radio.begin();
  radio.openWritingPipe(address);
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

  
}
void resultGame(){
  if(newScore == 1 ){
    digitalWrite(greenPin,HIGH);
    lcd.clear();
    lcd.print("You Won !");
    delay(5000);
    }
  else{

     digitalWrite(redPin,HIGH);
     lcd.clear();
     lcd.print("You lost !");
     delay(5000);
     
    
    }
  }
void getScore(){  
   if (radio.available()) {
  radio.startListening();
  radio.read(&dataPackage,sizeof(dataPackage));
  
   }
//  newScore = dataPackage.score;
  delay(500);
  Serial.println("string: ");
  Serial.println(dataPackage.guess);

  Serial.println(dataPackage.score);
  Serial.println("testing string: ");
  Serial.println(dataPackage.guess);
  char temp[8]={};
  answer.toCharArray(temp,sizeof(temp));
  Serial.println("temp");
  Serial.println(temp);
//  Serial.println(String(dataPackage.guess) == String(temp));
  
   
  
  if(String(realAnswer) == String(temp) ){
    lcd.print("here");
    newScore= newScore+1;
   Serial.println("newScore");
    Serial.println(newScore);
    }

    resultGame();
  


//  if(newScore - oldScore == 1){
//    lcd.clear();
//    lcd.print("Good guess");
//    // This one contains the score
//    lcd.setCursor(5,1);
//    lcd.print("score: ");
//    lcd.print(newScore);
// // result =true;
//  } else if (newScore == oldScore) {
//    lcd.clear();
//    lcd.print("Wrong guess");
//    // This one contains the score
//    lcd.setCursor(5,1);
//    lcd.print("score: ");
//    lcd.print(newScore);


//  }
// oldScore = newScore; 
}



void sendData(){
   Serial.println("this is a testing");
   radio.stopListening();
   answer.toCharArray(dataPackage.guess,sizeof(dataPackage.guess));
   radio.write(&dataPackage,sizeof(dataPackage));
   getScore();
}


void(* resetFunc) (void) = 0;

void loop() {

  


  
 // receive
  //  restart the game
   if (radio.available()) {
   radio.read(&dataPackage, sizeof(dataPackage));

      if(dataPackage.reset == 1){
        resetFunc();
        lcd.clear();
        lcd.print("Retset...");
        delay(1000);
      
   } //reset
   
    Serial.println("data: ");
    Serial.println(dataPackage.guess);
    Serial.println(dataPackage.ID);
    Serial.println(dataPackage.score);
    Serial.println(dataPackage.gameOver);
    Serial.println(dataPackage.reset);
    
    radio.stopListening();
    memcpy(realAnswer, dataPackage.guess, sizeof(dataPackage.guess));
    newScore=dataPackage.score;
    Serial.println("new");
    Serial.println(newScore);
    

   
//    if(newScore - oldScore == 1){
//      lcd.clear();
//      lcd.print("Good guess");
//      // This one contains the score
//      lcd.setCursor(5,1);
//      lcd.print("score: ");
//      lcd.print(newScore);
//      delay(1000);
//      lcd.clear();
//      lcd.print("Next round..");
//      oldScore = newScore;
//      //resultGame();
//      }

          

 while(dataPackage.reset==-1 && dataPackage.gameOver == 0){

  if(analogRead(deleteButtonPin)>900){
    lcd.clear();
    answer="";
  }


 
  
  delay(120);

  xPosition = analogRead(joyX);
  yPosition = analogRead(joyY);
  switchState = digitalRead(joySwitch);
  mapX = map(xPosition, 0, 1023, -512, 512);
  mapY = map(yPosition, 0, 1023, -512, 512);

  
  lcd.clear();


  if(mapY > 400){
    //lcd.print("show");
    letterCounter++;  
    
    if(letterCounter>26)
      letterCounter = 0; 
  }
  if(mapY < -400){
    //lcd.print("show");
    letterCounter--;  
    if(letterCounter<0)
      letterCounter = 26; 
  }
//  if(mapX > 400){
//    //lcd.print("show");
//    cursorCounter++;  
//    if(cursorCounter >16)
//      cursorCounter=0;
//    //Serial.println(cursorCounter);
//    
//  }
//   if(mapX < -400){
//    //lcd.print("show");
//    cursorCounter--;  
//    if(cursorCounter <0)
//      cursorCounter=16;
//    //Serial.println(cursorCounter);
//    }

//  
//  
  lcd.setCursor(cursorCounter,0);
  lcd.print(chararr[letterCounter]);
  if(switchState == 0){
    answer = answer+chararr[letterCounter];
  }   
  lcd.setCursor(0,1);
  lcd.print(answer);
  
  // Serial.println(analogRead(guessButtonPin));
  if(analogRead(guessButtonPin)>900){
      sendData();
  }
  

} 
    

 
    
  } // radio
  
}
