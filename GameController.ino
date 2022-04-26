// Game Controller Arduino for CryptoColor Game
//
//
// References:
//    NeoPixel: https://elearn.ellak.gr/mod/book/view.php?id=4561&chapterid=2406 ,
//              https://github.com/adafruit/Adafruit_NeoPixel 
//    NRF24L01: https://create.arduino.cc/projecthub/muhammad-aqib/nrf24l01-interfacing-with-arduino-wireless-communication-0c13d4 
//              https://howtomechatronics.com/tutorials/arduino/arduino-wireless-communication-nrf24l01-tutorial/
//    Random: https://www.arduino.cc/reference/en/language/functions/random-numbers/random/
//    Reset: https://www.theengineeringprojects.com/2015/11/reset-arduino-programmatically.html
//    
//
// ---------------------------------------------------------------------------------------------------------------------------------

// Add the libraries required
#include <LiquidCrystal.h>
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Player will be sending data like this:
struct Data_Package {
  char guess[8] = {'H'};
  int ID = 0;
  int score = 0;
  int reset = 0;
  bool gameOver = false;
};

// Data variables for players
Data_Package data1;
Data_Package data2;
Data_Package data3;


// Button pin to restart a game
const int buttonPin = A1;
int buttonVal = 0;


// LED Strip pin
const int ledStick = 2;
const int numLED = 8;
// Start the LED strip
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(numLED, ledStick, NEO_GRB + NEO_KHZ800);


// Set up LED Display
const int rs = 4, en = 9, d4 = 8, d5 = 7, d6 = 6, d7 = 5;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


// NRF24L01 module pins
const int NRF24L01_CE = 3;
const int NRF24L01_CS = 10;
const int NRF24L01_MOSI = 11;
const int NRF24L01_MISO = 12;
const int NRF24L01_SCK = 13;

// Start the RF24 radio
RF24 radio(NRF24L01_CE, NRF24L01_CS);

// Address of this node in Octal format (04,031,etc.)
const byte thisAddr[6] = "00001"; // Controller
const byte player1[6] = "00001"; // Player 1
const byte player2[6] = "00003"; // Player 2
const byte player3[6] = "00004"; // Player 3

// This determines if the controller should listen for guesses
bool br = false;


// Used for reading value of button
unsigned long r;


// Time variables
const int MAXTIME = 80;
int currTime = 0;


// The array of words available in the game
const int sizeArrayWords = 16;
String currentWord = "";
const String words[] = {"dog", "cat", "grow", "bottle", "water", "gum", "glasses", "shirt", "photo", "coin", "phone", "key", "pencil", "case", "notebook", "laptop"};
//const String words[] = {"gum", "cat"};
//const int sizeArrayWords = 2;


// Boolean variable to see if someone has won the game
bool isWin = false;

// This function is used to reset the arduino
void(* resetFunc) (void) = 0;


void setup() {

  // Being serial
  Serial.begin(9600);

  // LCD Display
  lcd.begin(16,2);

  // LED Strip
  pixels.begin(); // Initializes the library
  pixels.show(); // Sets the LEDs off

  // NRF24L01 module
  radio.begin();
  radio.openReadingPipe(0, thisAddr);
  radio.openWritingPipe(thisAddr);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

  // For random number
  randomSeed(analogRead(0));

  // Print a welcoming message
  lcd.print("Welcome to");
  lcd.setCursor(0, 1);
  lcd.print("CryptoColor!");
  delay(2000);
  // Print that the game is going to being soon
  lcd.setCursor(0, 0);
  lcd.print("The game is");
  lcd.setCursor(0, 1);
  lcd.print("starting soon!");
  delay(4000);
  currTime = MAXTIME;
}


// This starts when all the players are ready.
void loop() {
  
  // Check to see if button is pressed
  isPressed();

  // 1. Select the word
  selectWord();
  
  // Check to see if button is pressed
  isPressed();

  // Start listening
  radio.startListening();

  // 4. Listen for guesses
  while(!br && currTime > 0) {

    // 3. Start the timer (80 seconds)
    if (currTime == 1) {
      lcd.clear();
      lcd.print("Time Left: " + String(currTime));
      currTime--;
      br = true;
    } else {
      lcd.clear();
      lcd.print("Time Left: " + String(currTime));
      currTime--;
    }
    
    // Read the incoming data from all the Players
    radio.read(&data1, sizeof(data1));
    if (data1.ID == 1) {
      Serial.println("Data recieved: ");
      Serial.println(data1.guess);
      Serial.println(data1.ID);
      Serial.println(data1.score);
    }
//    radio.read(&data2, sizeof(data2));
//    radio.read(&data3, sizeof(data3));
//    
    // 5. If guess is recieved, check to see if it's correct
    if(isCorrect(data1.guess)) {
      // 6. If it is, add points to the player's score
      sendPoint(data1.ID, 1);
    } else {
      // 7. If not, send a value to display a message
      sendPoint(data1.ID, 0);
    }
//
//    if(isCorrect(data2.guess)) {
//      sendPoint(data2.ID, 1);
//    } else {
//      sendPoint(data2.ID, 0);
//    }
//
//    if(isCorrect(data3.guess)) {
//      sendPoint(data3.ID, 1);
//    } else {
//      sendPoint(data3.ID, 0);
//    }

    // "Make a sec pass"
    delay(1000);
    if(br == true) {
      lcd.clear();
      lcd.print("Time Left: 0");
      lcd.setCursor(0, 1);
      lcd.print(currentWord);
    }
    
  } // End of while


  radio.stopListening();
  // Check to see if button is pressed
  isPressed();

  // 8. Do not recieve guesses and see if someone won
  if(isWin) {
    isWin = false;
    // reset the game
    delay(5000);
    resetFunc();
  } else {
    // 9. If time is up, display the answer to the players and turn off the LED lights
//    radio.write(&currentWord, sizeof(currentWord));
//    radio.write(&currentWord, sizeof(currentWord));
//    radio.write(&currentWord, sizeof(currentWord));
    pixels.clear();
    pixels.show();
  }

  // Check to see if button is pressed
  isPressed();
 
} // END




// This function is used to select a word and display it on the LED strip
void selectWord() {

  pixels.clear();

  lcd.clear();
  lcd.print("Selecting a");
  lcd.setCursor(0, 1);
  lcd.print("random word...");

  delay(1500);
  
  // 1. Select a random word
  long randomNum = random(0, sizeArrayWords); // From 0 to sizeArrayWords-1
  currentWord = words[randomNum]; // Save it to the currentWord variable
  
  // 2. Parse through the word and output the correct colors in the LED lights
  char W[8];
  currentWord.toCharArray(W, 8);
  // pixels.Color takes RGB values and pixels go from 0-7

  Serial.println("Word: " + currentWord);
  
  for(int i = 0; i < sizeof(currentWord); ++i) {

    if(W[i] > 96 && W[i] < 100) {
      pixels.setPixelColor(i, pixels.Color(255, 0, 0)); // RED - ABC
    } else if (W[i] > 99 && W[i] < 103) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 255)); // BLUE - DEF
    } else if (W[i] > 102 && W[i] < 106) {
      pixels.setPixelColor(i, pixels.Color(0, 255, 0)); // GREEN - GHI
    } else if (W[i] > 105 && W[i] < 110) {
      pixels.setPixelColor(i, pixels.Color(255, 255, 255)); // WHITE - JKLM
    } else if (W[i] > 109 && W[i] < 113) {
      pixels.setPixelColor(i, pixels.Color(255, 255, 0)); // YELLOW - NOP
    } else if (W[i] > 112 && W[i] < 117) {
      pixels.setPixelColor(i, pixels.Color(128, 0, 128)); // PURPLE - QRST
    } else if (W[i] > 116 && W[i] < 123) {
      pixels.setPixelColor(i, pixels.Color(255, 165, 0)); // ORANGE - UVWXYZ
    } else {
      // Then it is not valid. Do nothing.
    }
  }
  pixels.setBrightness(5);
  pixels.show(); // Send the updated pixel color to hardware

  currentWord.toCharArray(data1.guess, sizeof(data1.guess));
  data1.ID = 1;
  data1.score = 0;
  data1.reset = -1;
  data1.gameOver = false;
  
  radio.write(&data1, sizeof(data1));
  radio.write(&data1, sizeof(data1));
  radio.write(&data1, sizeof(data1));
  
}


// This function is used to see if the player's guess is correct
bool isCorrect(String guess) {
  if (guess == currentWord) {
    Serial.println("CORRECT");
    return true;
  } else {
    return false;
  }
}


// This function is used to send the point of 1 or 0
void sendPoint(int ID, int point) {

  if (ID == 1) {
    String r = "STRING";
    r.toCharArray(data1.guess, sizeof(data1.guess));
    data1.ID = 1;
    int oldScore = data1.score;
    data1.score = oldScore + point;
    Serial.println("New Score: ");
    Serial.println(data1.score);
    data1.reset = 0;
    data1.gameOver = false;
    if(isThereWin(ID, data1.score)) {
      // Send data to everyone
      data1.gameOver = true;
      radio.stopListening();
      radio.write(&data1, sizeof(data1));
      //radio.write(&data2, sizeof(data2));
      //radio.write(&data3, sizeof(data3));
    } else {
      radio.stopListening();
      radio.write(&data1, sizeof(data1));
      radio.write(&data1, sizeof(data1));
      radio.write(&data1, sizeof(data1));
    }
//  } else if (ID == 2) {
//    data2.score += point;
//    if(isThereWin(ID, data2.score)) {
//      // Send data to everyone
//      radio.write(&data1, sizeof(data1));
//      radio.write(&data2, sizeof(data2));
//      radio.write(&data3, sizeof(data3));
//    } else {
//      radio.write(&data2, sizeof(data2));
//    }
//  } else if (ID == 3) {
//    data3.score += point;
//    if(isThereWin(ID, data3.score)) {
//      // Send data to everyone
//      radio.write(&data1, sizeof(data1));
//      radio.write(&data2, sizeof(data2));
//      radio.write(&data3, sizeof(data3));
//    } else {
//      radio.write(&data3, sizeof(data3));
//    }
  } else {
    // It is not a valid ID
  }

  radio.startListening();
  
}


// This function checks if the winning condition is met
bool isThereWin(int ID, int points) {
  if (points == 10) {
    // Update the bool variable in Data_Package
    data1.gameOver = true;
    data2.gameOver = true;
    data3.gameOver = true;
    // Update variable for controller
    isWin = true;
    return true;
  }
  return false;
}


// This function is used to see if the button is pressed
void isPressed() {
  
  int isPress = analogRead(buttonPin);
  if(isPress > 1000) { // It's pressed
    // Send a number to the players to reset their arduino
    String r = "press";
    r.toCharArray(data1.guess,sizeof(data1.guess));
    data1.ID = 1;
    data1.score = 0;
    data1.reset = -1;
    data1.gameOver = false;

//    r.toCharArray(data2.guess,sizeof(data2.guess));
//    data2.ID = 2;
//    data2.score = 0;
//    data2.reset = -1;
//    data2.gameOver = false;
//
//    r.toCharArray(data3.guess,sizeof(data3.guess));
//    data3.ID = 3;
//    data3.score = 0;
//    data3.reset = -1;
//    data3.gameOver = false;

    radio.write(&data1, sizeof(data1));
    radio.write(&data1, sizeof(data1));
    radio.write(&data1, sizeof(data1));
    //radio.write(&data2, sizeof(data2));
    //radio.write(&data3, sizeof(data3));
    // Reset this Arduino
    lcd.clear();
    lcd.print("Restarting...");
    delay(1000);
    resetFunc();
  }
  
}
