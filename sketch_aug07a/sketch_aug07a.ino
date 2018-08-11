#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <SoftwareSerial.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int value;
    struct Node *next;
} ListNode;

typedef struct StackHead {
    int size;
    ListNode *top;
} Stack;

void LCD_init(void);
void initDisks(int rod_pos);
void drawSelectBox(int rod_pos);
void drawPickedDisk(int disk_length, int rod_pos);
void drawDisk(int disk_length, int rod_pos, int stack_pos);
void drawGrid(void);
int isSuccess(void);
void stackOverallInit(void);
void flagInit(void);


Stack *initStack();
void freeStack(Stack *rod);
void pushElement(Stack *rod, ListNode *disk);
ListNode *popElement(Stack *rod);
int getSize(Stack *rod);
int isEmpty(Stack *rod);
ListNode *initNode(int value);
void printStack(Stack *rod);
void drawStack(Stack *rod, int rod_pos);
void drawOverallStack();

U8G2_ST75256_JLX256160_2_4W_SW_SPI u8g2(U8G2_MIRROR, /* clock=*/ 13, /* data=*/ 11, /* cs=*/ 10, /* dc=*/ 9, /* reset=*/ 8);

int FIRST_ROD_POS = 35;
int ROD_GAP = 75;
int BASE_BAR_Y_POS = 140;
int DISK_HEIGHT = 20;
int MAX_DISK_LENGTH = 60;

#define rxPin 6
#define txPin 7
SoftwareSerial mySerial = SoftwareSerial(rxPin, txPin);
//SoftwareSerial musicSerial = SoftwareSerial(2, 3);
Stack *diskState[3];
ListNode *movedNode = NULL;
int select_box_index = 0;
int picked_flag = 0;
int error_flag = 0;
int success_flag = 0;
int reset_flag = 0;
int INIT_ROD_INDEX = 0;

void stackOverallInit(void) {
  Stack *rod1 = initStack();
  Stack *rod2 = initStack();
  Stack *rod3 = initStack();
  ListNode *n1 = initNode(60);
  ListNode *n2 = initNode(40);
  ListNode *n3 = initNode(20);
  pushElement(rod1, n1);
  pushElement(rod1, n2);
  pushElement(rod1, n3);
  diskState[0] = rod1;
  diskState[1] = rod2;
  diskState[2] = rod3;
}

void flagInit(void) {
  movedNode = NULL;
  select_box_index = 0;
  picked_flag = 0;
  error_flag = 0;
  success_flag = 0;
  reset_flag = 0;
}

// Follow the routine given by SPI example (DigitalPotControl)
void setup(void) {
  u8g2.begin();
  LCD_init();
  // Init the rod stack
  stackOverallInit();
  flagInit();
  
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  
  mySerial.begin(9600);
  Serial.begin(9600);
  //musicSerial.begin(9600);
}

void loop(void) {
  u8g2.firstPage();
  do {
    drawGrid();
    drawSelectBox(select_box_index);
    drawOverallStack();
    if (error_flag == 1) {
      u8g2.setCursor(30, 70);
      u8g2.print("[Error] Disk Disorder!");
      error_flag = 0;
    }
    if (success_flag == 1) {
      u8g2.setCursor(30, 70);
      u8g2.print("[Success] Excellent! :)");
    }

    if (reset_flag == 1) {
      int i;
      for (i = 0; i < sizeof(diskState)/sizeof(Stack *); ++i) {
        freeStack(diskState[i]);
      }
      stackOverallInit();
      flagInit();
    }
  } while ( u8g2.nextPage() );

  //mySerial.listen();
  if (mySerial.available() > 0) {
    char cmd = mySerial.read();
    Serial.println(cmd);
    // Left (Left/None) => A
    // Right (Right/None) => B
    // EMG (PickUp/PutDown) => C/D (1/0)
    switch (cmd)
    {
      case 'A':
        select_box_index--;
//        if (select_box_index < 0) select_box_index = 0;
        if (select_box_index < 0) select_box_index = 2;
        break;
      case 'B':
        select_box_index++;
        if (select_box_index > 2) select_box_index = 2;
        break;
      case 'C':
        picked_flag = 1;
        break;
      case 'D':
        picked_flag = 0;
        break;
      case 'R':
        reset_flag = 1;
        //musicSerial.listen();
        //musicSerial.write('O');
        break;
      default:
        break;
    }
  }
}

void drawGrid(void)
{
  u8g2.setCursor(0, 12);
  u8g2.print("=======Tower of Hanoi=======");

  u8g2.drawFrame(0, BASE_BAR_Y_POS, 225, 5); // bottom bar

  int rodPos[] = {FIRST_ROD_POS, FIRST_ROD_POS + ROD_GAP, FIRST_ROD_POS + 2 * ROD_GAP};
  int i;
  for (i = 0; i < sizeof(rodPos) / sizeof(int); ++i)
  {
    u8g2.drawLine(rodPos[i], 40, rodPos[i], BASE_BAR_Y_POS); // draw rods
    u8g2.drawLine(rodPos[i] + 1, 40, rodPos[i] + 1, BASE_BAR_Y_POS);
  }
}

void drawDisk(int disk_length, int rod_pos, int stack_pos)
{
  // Calculate the starting position of the disk.
  // rod_pos is in [0, 2]
  // stack_pos is in [0, 2]
  // disk_length is even
  int x, y;
  x = FIRST_ROD_POS + (rod_pos * ROD_GAP) - disk_length / 2;
  y = BASE_BAR_Y_POS - ((getSize(diskState[rod_pos]) - 1 - stack_pos) + 1) * DISK_HEIGHT;
  u8g2.drawBox(x, y, disk_length, DISK_HEIGHT);
}

void drawPickedDisk(int disk_length, int rod_pos)
{
  int x, y;
  x = FIRST_ROD_POS + (rod_pos * ROD_GAP) - disk_length / 2;
  y = 20;
  /*
  Serial.println(x);
  Serial.println(y);
  Serial.println(disk_length);
  Serial.println(DISK_HEIGHT);*/
  u8g2.drawBox(x, y, disk_length, DISK_HEIGHT);
}

void drawSelectBox(int rod_pos)
{
  int x1, y1, x2, y2, x3, y3, x4, y4;
  x1 = FIRST_ROD_POS + (rod_pos * ROD_GAP) - MAX_DISK_LENGTH / 2 - 2;
  y1 = 15;

  x2 = x1 + 2 * (MAX_DISK_LENGTH / 2 + 1) + 1;
  y2 = 15;

  x3 = x1;
  y3 = BASE_BAR_Y_POS;

  x4 = x2;
  y4 = BASE_BAR_Y_POS;

  u8g2.drawLine(x1, y1, x2, y2);
  u8g2.drawLine(x2, y2, x4, y4);
  u8g2.drawLine(x1, y1, x3, y3);

  if (picked_flag == 1)
  {
    if (movedNode == NULL) {
      movedNode = popElement(diskState[select_box_index]);
    }
    if (movedNode != NULL) {
      drawPickedDisk(movedNode->value, select_box_index);
    }
  } else {
    if (movedNode != NULL) {
      if (movedNode->value > diskState[select_box_index]->top->value) {
        error_flag = 1;
        drawPickedDisk(movedNode->value, select_box_index);
      } else {
        pushElement(diskState[select_box_index], movedNode);
        movedNode = NULL; 
        if (isSuccess() == 1 && success_flag == 0) {
          success_flag = 1;
          mySerial.write('K');
        } else {
          mySerial.write('S'); // indicate that one successful step has been taken
          Serial.println('S');
        }
      }
    }
  }
}

void initDisks(int rod_pos)
{
  drawDisk(20, rod_pos, 0);
  drawDisk(40, rod_pos, 1);
  drawDisk(60, rod_pos, 2);
}

void LCD_init(void)
{
  u8g2.setFont(u8g2_font_ncenB10_tr);
}

void printStack(Stack *rod) {
    Serial.println("==========Stack Content==========\n");
    ListNode *it = rod->top;
    while (it) {
      Serial.println(it->value);
      it = it->next;
    }
    Serial.println("\n=================================\n");
}

ListNode *initNode(int value) {
    ListNode *result = (ListNode *) malloc(sizeof(ListNode));
    result->value = value;
    result->next = NULL;
    return result;
}

Stack *initStack() {
    Stack *result = (Stack *) malloc(sizeof(Stack));
    result->size = 0;
    result->top = NULL;
    return result;
}

void freeStack(Stack *rod) {
    while (rod->size > 0) {
        ListNode *topNode = popElement(rod);
        free(topNode);
    }
    free(rod);
}

void pushElement(Stack *rod, ListNode *disk) {
    rod->size++;
    disk->next = rod->top;
    rod->top = disk;
}

ListNode *popElement(Stack *rod) {
    if (isEmpty(rod) == 1) {
        return NULL;
    } else {
        rod->size--;
        ListNode *topNode = rod->top;
        rod->top = rod->top->next;
        return topNode;
    }
}

int getSize(Stack *rod) {
    return rod->size;
}

int isEmpty(Stack *rod) {
    if (rod->size == 0) {
        return 1;
    } else {
        return 0;
    }
}

void drawStack(Stack *rod, int rod_pos) {
  ListNode *it = rod->top;
  int stack_pos = 0;
  while (it) {
    drawDisk(it->value, rod_pos, stack_pos);
    ++stack_pos;
    it = it->next;
  }
}

void drawOverallStack() {
  int i;
  for (i = 0; i < sizeof(diskState)/sizeof(Stack *); ++i) {
    drawStack(diskState[i], i);
  }
}

int isSuccess(void) {
  if (select_box_index != INIT_ROD_INDEX) {
    Stack *current_stack = diskState[select_box_index];
    ListNode *it = current_stack->top;
    if (it->next == NULL) {
      return 0;
    } else {
      while (it && it->next) {
        if (it->value > it->next->value) return 0;
        it = it->next;
      } 
      return (getSize(diskState[select_box_index]) == 3) ? 1 : 0;
    }
  }
}
