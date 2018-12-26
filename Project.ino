#include "LedControl.h"

#define PIN7 7

LedControl lc=LedControl(12,10,11,2);  // Pins: DIN,CLK,CS, # of Display connected

unsigned long delayTime = 500;  // Delay between Frames
unsigned long stepTime = 200;  
unsigned long flashTime = 500;

unsigned long now;

bool field[16][8];
bool temp[16][8];

bool states[4];
uint8_t pin_mask[4];
volatile uint8_t * pin_port[4];

enum Type{
  O,  
  T, 
  S, 
  I,
  Z, 
  L, 
  J
};

bool types[7][4][4][4] = {
  {
    {{1, 1, 0, 0},
     {1, 1, 0, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0}},

    {{1, 1, 0, 0},
     {1, 1, 0, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0}},

    {{1, 1, 0, 0},
     {1, 1, 0, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0}},

    {{1, 1, 0, 0},
     {1, 1, 0, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0}}
     
   },
  
  {
    {{1, 1, 1, 0}, 
     {0, 1, 0, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0}},

    {{0, 1, 0, 0}, 
     {1, 1, 0, 0},
     {0, 1, 0, 0},
     {0, 0, 0, 0}},

    {{0, 1, 0, 0}, 
     {1, 1, 1, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0}},

    {{1, 0, 0, 0}, 
     {1, 1, 0, 0},
     {1, 0, 0, 0},
     {0, 0, 0, 0}}

   },
  
  {
   {{0, 1, 1, 0},
    {1, 1, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}},
    
   {{1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 0, 0}},

   {{0, 1, 1, 0},
    {1, 1, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}},

   {{1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 0, 0}} 
     
  },

  {
   {{1, 1, 1, 1},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}},

   {{1, 0, 0, 0},
    {1, 0, 0, 0},
    {1, 0, 0, 0},
    {1, 0, 0, 0}},

   {{1, 1, 1, 1},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}},

   {{1, 0, 0, 0},
    {1, 0, 0, 0},
    {1, 0, 0, 0},
    {1, 0, 0, 0}}
  },



  {
    {{1, 1, 0, 0},
     {0, 1, 1, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0}},

    {{0, 1, 0, 0},
     {1, 1, 0, 0},
     {1, 0, 0, 0},
     {0, 0, 0, 0}},

    {{1, 1, 0, 0},
     {0, 1, 1, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0}},

    {{0, 1, 0, 0},
     {1, 1, 0, 0},
     {1, 0, 0, 0},
     {0, 0, 0, 0}}
  },

  {
    {{1, 1, 1, 0},
     {1, 0, 0, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0}},

    {{1, 1, 0, 0},
     {0, 1, 0, 0},
     {0, 1, 0, 0},
     {0, 0, 0, 0}},

    {{0, 0, 1, 0},
     {1, 1, 1, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0}},

    {{1, 0, 0, 0},
     {1, 0, 0, 0},
     {1, 1, 0, 0},
     {0, 0, 0, 0}}
  },

  {
    {{1, 1, 1, 0},
     {0, 0, 1, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0}},

    {{0, 1, 0, 0},
     {0, 1, 0, 0},
     {1, 1, 0, 0},
     {0, 0, 0, 0}},

    {{1, 0, 0, 0},
     {1, 1, 1, 0},
     {0, 0, 0, 0},
     {0, 0, 0, 0}},

    {{1, 1, 0, 0},
     {1, 0, 0, 0},
     {1, 0, 0, 0},
     {0, 0, 0, 0}}
  }

};



struct Item {
  int topX, topY;
  int rotation;
  Type type;
};



//#########################################################################
void setup()
{
  Serial.begin(9600);
  randomSeed(analogRead(0));
 
  lc.shutdown(0,false);  // Wake up displays
  lc.shutdown(1,false);
  lc.setIntensity(0,1);  // Set intensity levels
  lc.setIntensity(1,1);
  lc.clearDisplay(0);  // Clear Displays
  lc.clearDisplay(1);

  clearScreen();
  states[1] = states[1] = states[1] = states[1] = 0;

  for(int i = 0; i < 4; i++){
    pin_mask[i] = digitalPinToBitMask(7 - i);
    pin_port[i] = portInputRegister(digitalPinToPort(7 - i));
  }

  pinMode(7, INPUT);
  pinMode(6, INPUT);
  pinMode(5, INPUT);
  pinMode(4, INPUT);
}


//#########################################################################

void writeToScreen(bool screen[16][8]){
    for(int i = 0; i < 16; i++){
      for(int j= 0 ; j < 8; j++){
        int deviceNumber = (i < 8) ? 1 : 0;
        int row = j;
        int column = (deviceNumber == 1) ? (7 - i) : (15 - i);
        lc.setLed(deviceNumber, row, column, screen[i][j]);
      }
    }
}

void clearScreen(){
  for(int i = 0; i < 16; i++){
    for(int j = 0 ; j < 8; j++){
      field[i][j] = 0;
      temp[i][j] = 0;
    }
  }
}


void drawItem(Item item){
  for (int i = 0 ; i < 4; i++){
    for (int j = 0; j < 4; j++){
      if(types[item.type][item.rotation][i][j]){
        field[item.topX + i][item.topY + j] = 1;
      }
    }
  }
}

void drawItemToTemp(Item item){
  for (int i = 0 ; i < 4; i++){
    for (int j = 0; j < 4; j++){
      if(types[item.type][item.rotation][i][j]){
        temp[item.topX + i][item.topY + j] = 1;
      }
    }
  }
}

void clearTemp(){
  for (int i = 0; i < 16; i++){
    for(int j = 0; j < 8; j++){
      temp[i][j] = field[i][j];
    }
  }
}

void negTemp(){
  for (int i = 0; i < 16; i++){
    for(int j = 0; j < 8; j++){
      temp[i][j] = !field[i][j];
    }
  }
}

void applyTemp(){
  for (int i = 0; i < 16; i++){
    for(int j = 0; j < 8; j++){
      field[i][j] = temp[i][j];
    }
  }
}

void flash(int times){
  for(int i = 0; i < times; i++){
    if (i % 2){
      negTemp();
    }else{
      clearTemp();
    }
    writeToScreen(temp);
    delay(flashTime);
  }
  clearTemp();
}

void flashRow(int row, int times){
  clearTemp();
  for(int i = 0; i < times; i++){
    if(i % 2){
      for(int j = 0; j < 8 ; j++){
        temp[row][j] = !field[row][j];
      }
    }else{
      for(int j = 0; j < 8 ; j++){
        temp[row][j] = field[row][j];
      }
    }
    writeToScreen(temp);
    delay(flashTime);
  }
  for(int j = 0; j < 8 ; j++){
        temp[row][j] = field[row][j];
  }
}

//#########################################################################

Item createObject(){
  Item item;
  int type  =random(0, 7);
  item.rotation = random(0, 4);
  item.type = type;
  item.topX = 0;
  
  switch(type){
    case O:
      item.topY = random(0, 7);
      break;
    case I:
      if(item.rotation % 2 == 0)
        item.topY = random(0, 5);
      else
        item.topY = random(0, 8);
      break;
    case T:
    case S:
    case Z:
    case J:
    case L:
      if(item.rotation % 2 == 0)
        item.topY = random(0, 6);
      else
        item.topY = random(0, 7);
      break;
  }
  return item;
}

bool isBeneathEmpty(Item item){
  if (item.rotation % 2 == 0){ 
    switch (item.type){
      case O:
      case T:
      case S:
      case Z:
      case L:
      case J:
        if(item.topX + 1 == 15){
          return false;
        }
        break;
      case I:
        if(item.topX == 15){
          return false;
        }
        break;
    }
 }else{
    switch(item.type){
      case O:
        if(item.topX + 1 == 15){
          return false;
        }
        break;
      case T:
      case S:
      case Z:
      case L:
      case J:
         if(item.topX + 2 == 15){
          return false;
        }
        break;
      case I:
        if(item.topX + 3 == 15){
          return false;
        }
        break;
    }
 }
  
  for(int i = 0 ; i < 4; i++){
    for(int j = 0; j < 4; j++){
      if(types[item.type][item.rotation][i][j]){
        if(item.topX + i + 1 < 16 && item.topY + j < 8 && field[item.topX + i + 1][item.topY + j]){
          return false;
        }
      }
    }
  }
  return true;
}

bool isValid(Item item){
  for(int i = 0; i < 4; i++){
    for(int j = 0; j < 4; j++){
      if(types[item.type][item.rotation][i][j]){
        if(item.topX + i >= 16 && item.topY + j >= 8){
          return false;
        }
        if(field[item.topX + i][item.topY + j]){
          return false;
        }
      }
    }
  }
  return true;
}

void checkForCompleteRow(){
  for(int i = 15; i >= 0; i--){
    bool allGood = true;
    for(int j = 0; j < 8; j++){
      if(!field[i][j]){
        allGood = false;
        break;
      }
    }

    if(allGood){
      
      flashRow(i, 6);
      
      for(int k = i-1 ; k >= -1; k--){
        for(int s = 0 ; s < 8; s++){
          if (k >= 0)
            field[k+1][s] = field[k][s];
          else
            field[0][s] = 0;
        }
      }
 
      i++;
    }
  }

  clearTemp();
}

void fallDown(Item item){
  drawItemToTemp(item);
  writeToScreen(temp);  
  if(!isValid(item)){
    clearScreen();
    return;
  }
  
  while(1){
    for(int i = 0; i < 4; i++){
      bool readFromPin = ((*pin_port[i] & pin_mask[i]) != 0);
      if(!states[i]){
        if(readFromPin){
          states[i] = 1;
        }
      }else{
        if(!readFromPin){
          // Button has been released
          states[i] = 0;
          // Do something
          
          switch(i){
            case 0:
              rotateClockwise(item);
              break;
            case 1:
              moveLeft(item);
              break;
            case 2:
              moveRight(item);
              break;
            case 3:
              moveDown(item);
              break;  
          }
        }
      }
    }
    

    if(millis() - now > stepTime){
      now = millis();
      if(isBeneathEmpty(item)){
        item.topX += 1;
        clearTemp();
        drawItemToTemp(item);
        writeToScreen(temp);
      }else{
        // Reached the lowest it can go. 
        applyTemp();
        break;
      }   
    } 

    
  }
}

void rotateClockwise(Item item){
  
  Serial.print("Rotate");
   // TODO KIARASH
}

void moveLeft(Item item){
  Serial.print("Left");
}

void moveRight(Item item){
  
  Serial.print("Right");
  // TODO KIARASH
}

void moveDown(Item item){
  
  Serial.print("Down");
  // TODO KIARASH
}

void gameOverScreen(){
  // Game is over. Do something cool.
  delay(1000);
  clearScreen();
}

bool isGameOver(){
  // TODO
  return false;
}
//#########################################################################

void loop()
{
  int counter = 3;
  while(1){
    delay(delayTime);
    Item item = createObject();
    fallDown(item);
    checkForCompleteRow();
//    if(isGameOver()){
//      gameOverScreen();
//    }
  }
}





//    counter = (counter +1) % 4;
//    Item item;
//    item.type = O;
//    item.topY = (counter) * 2;
//    item.topX = 0; 
