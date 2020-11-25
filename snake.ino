#include <TFT.h>
#include <SPI.h>
#include <string.h>

#define CS   10
#define DC   8
#define RESET  9
#define VRx A0
#define VRy A1
#define SW A2

TFT display = TFT(CS, DC, RESET);

struct coords{
  int x;
  int y;
  struct coords* next, * prev;
}*start = NULL, *konec = NULL;

int winScore=5;
int posX=5;
int score=0;
float moveTime = 200; 
const int sensitivity = 300; 
int startSnakeLength = 5; 
const int Left = 5; 
const int Down = 5;
int maxLeft;
int maxDown;
float oldTime = 0;
float dT; 
int headPosition[2];
int applePosition[2];
int movingDirections[2] = {1, 0};
float actualMoveTime;
int snakeLength = 5;
int drawnLength = 0;
int scoreGame = 0; 

void setupGame(){
  clearSnake();
  score=0;
  posX=5;
  snakeLength = startSnakeLength;
  drawnLength=0;
  actualMoveTime = moveTime;
  display.fill(255, 255, 255);
  headPosition[0] = random(0, display.width()/Left)*Left;
  headPosition[1] = random(0, display.height()/Down)*Down;
  maxDown = (display.height()/Down)*Down;
  maxLeft = (display.width()/Left)*Left;
  moveHead(headPosition[0], headPosition[1]);
  spawnApple();
}

void spawnApple(){
  do{
    applePosition[0] = random(0, display.width()/Left)*Left;
    applePosition[1] = random(0, display.height()/Down)*Down;
  }
  while(selfhit(applePosition[0], applePosition[1]));
  display.fill(0, 0, 255);
  display.stroke(0,0,255);
  display.rect(applePosition[0], applePosition[1], Left, Down);
}

void clearSnake(){
  display.noStroke();
  display.background(0,0,0);
  
  struct coords *p = start;
  if(start != NULL){
    while(p != NULL){
      if (start != konec){
          start = start->next;
          start->prev = NULL; 
      }else{
          start = NULL;
          konec = NULL;
      }
      struct coords* t = p->next;
      delete p;
      p = t;
    }
  }
}

void moveHead(int x, int y){
  struct coords* tmp = new struct coords;
  tmp->x = x;
  tmp->y = y;
  if (start == NULL){
      start = tmp;
      konec = tmp;
      start->next = NULL;
      start->prev = NULL;
  }else{
      start->prev = tmp;
      tmp->next = start;
      start = tmp;
      tmp->prev = NULL;
  }
  if(applePosition[0] == x && applePosition[1] == y && score<winScore ){
    posX=posX+5;
    score++;
    display.stroke(0,0,255);
    display.setTextSize(1);
    display.text("I" ,posX,5);
    display.noStroke();
  }
  while(score>=winScore){
      display.background(0,0,0); 
      display.stroke(0,255,255);
      display.setTextSize(1);
      display.text("VICTORY!" ,64,64);
      display.noStroke();
      delay(3000);
      setupGame();
    }
  display.noStroke();
  display.fill(75, 160, 125);
  display.rect(x, y, Left, Down);
}

void removeTail(){
  display.noStroke();
  display.fill(0, 0, 0);
  display.rect(konec->x, konec->y, Left, Down);
  struct coords* p = konec;
  konec = konec->prev;
  konec->next = NULL;
  delete p;
}

bool selfhit(int x, int y){
  struct coords* p = start;
  while(p != NULL){
    if(p->x == x && p->y == y){
      return true;  
    }
    p=p->next;
  }
  return false;  
}

short move(bool r){
  if(actualMoveTime < 0){
    headPosition[0] += (movingDirections[0] * Left);
    headPosition[1] += (movingDirections[1] * Down);
    if(headPosition[0] > maxLeft){
        return 2;
    }else if(headPosition[0] < 0){
        return 2;
    }else if(headPosition[1] > maxDown){
        return 2;
    }else if(headPosition[1] < 0){
        return 2;
    }else if(selfhit(headPosition[0], headPosition[1])){
      return 2;
    }

    
    if(r){
        removeTail();
    }else{
      drawnLength++;  
    }
    moveHead(headPosition[0], headPosition[1]);
    if(headPosition[0] == applePosition[0] && headPosition[1] == applePosition[1]){
      snakeLength++;
      spawnApple();  
    }
    actualMoveTime = moveTime;
    return 1;
  }else{
    actualMoveTime -= dT;
    return 0;
  }
}

void checkDirections(int x, int y){
    if(x > sensitivity && x > y && x > -y) {
      movingDirections[1] = 0;
      movingDirections[0] = 1;
    } else if (x < -sensitivity && x < y && x < -y) {
      movingDirections[1] = 0;
      movingDirections[0] = -1;
    }else if(y > sensitivity && y > x && y > -x) {
       movingDirections[1] = 1;
      movingDirections[0] = 0;
    } else if(y < -sensitivity && y < x && y < -x) {
      movingDirections[1] = -1;
      movingDirections[0] = 0;
    }
}

float displayT(){
  float currentTime = millis();
  float dT = currentTime - oldTime;
  oldTime = currentTime;
  return dT;
}

void introAnim(){
  display.stroke(0,0,255);
  display.setTextSize(2);
  display.text("Snake",5,5);
  delay(1000);
  display.setTextSize(1);
  display.stroke(255,255,255);
  display.text("by Jaka German",5,20);
  delay(2500);
}


void setup() {
  Serial.begin(9600);
  pinMode(VRy, INPUT);
  pinMode(VRy, INPUT);
  pinMode(SW, INPUT); 

  display.begin();
  display.background(0,0,0); 
  display.setRotation(3);
  introAnim();
  display.noStroke();
  setupGame();
}

void loop() {
  int xPosition = analogRead(VRx);
  int yPosition = analogRead(VRy);
  int SW_state = digitalRead(SW);
  int mapX = map(xPosition, 0, 1023, -512, 512);
  int mapY = map(yPosition, 0, 1023, -512, 512);
  dT = displayT();

  checkDirections(mapX, mapY);
  short moveStatus = move((drawnLength == snakeLength));

  if(moveStatus == 2){
    setupGame();
    return;  
  }
  delay(5);
}
