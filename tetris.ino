#include <stdlib.h>    

#define MAXY 8
#define MAXX 8
#define MINX 0
#define MAXTETSIZE 4
#define WORDSIZE 16
#define MAX_INT8 0xff
#define NUMBEROFTYPES 2
#define ROTATIONS 4

#define Down() tet[t][1] += 1
#define Up() tet[t][1] -= 1
#define Left() tet[t][0] -= 1
#define Right() tet[t][0] += 1

uint8_t tetrisMap[MAXY];
uint8_t lastOutput[MAXY];
uint8_t popLastframe;
//0 is up      
//1 is down
//2 is left
//3 is right
uint8_t rotation;
//0 is line       
//1 is block
//2 is t
//3 is L
uint8_t type;
uint8_t tet[4][2];

void turn();
void draw();
void gameEnd();
void newTetri();
void shiftDown();
void shiftRight();
void shiftLeft();
void rotate();
void rowCheck();

//===Copied from Matrix.h from Arduino Library===//
uint8_t _pinData;
uint8_t _pinClock;
uint8_t _pinLoad;
uint8_t* _buffer;
uint8_t _screens;
uint8_t _maximumX;

void MatrixStart(uint8_t, uint8_t, uint8_t, uint8_t = 0x0F, uint8_t = 1);
void setBrightness(uint8_t);
void write(uint8_t, uint8_t, uint8_t);
void clear(void);
void putByte(uint8_t);
void setRegister(uint8_t, uint8_t);
void syncRow(uint8_t);
void setScanLimit(uint8_t);
//===End Headers for Library===//

uint8_t mergeArrays();
uint8_t popcount(uint8_t x);

// the setup function runs once when you press reset or power the board
void setup() {
  MatrixStart(0, 2, 1, 0x09); 
  pinMode(40,INPUT);
  pinMode(39,INPUT);
  pinMode(38,INPUT);
  cli();    // switch interrupts off while messing with their settings  
  PCICR =0x01;          // Enable PCINT1 interrupt
  PCMSK1 = 0b00000111;
  sei();    // turn interrupts back on
}

// the loop function runs over and over again until power down or reset
void loop() {
  newTetri();

  while (1) {
    //draws the current board
    //shifts downwards
    shiftDown();
    draw();     
    //checks to see if a row is filled.
    rowCheck();
    delay(1000);
  }

}

ISR(PCINT0_vect) {    // Interrupt service routine. Every single PCINT8..14 (=ADC0..5) change
  // will generate an interrupt: but this will always be the same interrupt routine
  if(digitalRead(40)==1){
    rotate();
  }
  else if (digitalRead(39)==1){
    shiftRight();
  }
  else if (digitalRead(38)==1){ 
    shiftLeft();
  }    
  else{                                
    shiftDown();
  }
}

//Function to draw the output array to the console.
void draw() {
  uint8_t output;

  clear();

  for (int8_t i = 0; i < MAXY; i++) {
    //Ors the map and the last output to create the
    //-current output frame row by row.
    output = tetrisMap[i] | lastOutput[i];
    for (int8_t j = 0; j < MAXX; j++)
    {
      write(j, i, ((output & (1 << j)) >> j));
    }
  }
}
//Ends the game by clearing and resets
void gameEnd() {
  for (int8_t i = 0; i < MAXY; i++) {
    //Ors the map and the last output to create the
    //-current output frame row by row.
    for (int8_t j = 0; j < MAXX; j++)
    {
      write(j, i, 1);
      delay(10);
    }
  }
  for (int8_t i = 0; i < MAXY; i++) {
    //Ors the map and the last output to create the
    //-current output frame row by row.
    for (int8_t j = 0; j < MAXX; j++)
    {
      write(j, i, 0);
      delay(10);
    }
  }
  delay(1000);
  //Resets to defaults
  popLastframe = 0;
  memset(lastOutput, 0, MAXY);
  memset(tetrisMap, 0, MAXY);
  //Generate new tetri
  newTetri();
}
//Generates a new Tetri piece.
void newTetri() {
  //Makes the current turn output the last turn output.
  for (int8_t i = 0; i < MAXY; i++) {
    lastOutput[i] = lastOutput[i] | tetrisMap[i];
  }
  //Generates a random tetri
  uint8_t rand = random(4);
  switch (rand)
  {
    //It chooses line tetri
  case 0:
    rotation = 0;
    type = 0;

    for (int8_t i = 0; i < MAXTETSIZE; i++) {
      tet[i][0] = 4;  //col
      tet[i][1] = i;  //row
    }
    break;
    //It chooses block
  case 1:
    rotation = 0;
    type = 1;

    tet[0][0] = 4;  //col
    tet[0][1] = 0;  //row

    tet[1][0] = 4;  //col
    tet[1][1] = 1;  //row

    tet[2][0] = 5;  //col
    tet[2][1] = 0;  //row

    tet[3][0] = 5;  //col
    tet[3][1] = 1;  //row
    break;
    //It choose t
  case 2:
    rotation = 0;
    type = 2;

    tet[0][0] = 3;  //col
    tet[0][1] = 0;  //row

    tet[1][0] = 4;  //col
    tet[1][1] = 0;  //row

    tet[2][0] = 5;  //col
    tet[2][1] = 0;  //row

    tet[3][0] = 4;  //col
    tet[3][1] = 1;  //row
    break;
    //It choose L
  case 3:
    rotation = 0;
    type = 3;

    tet[0][0] = 3;  //col
    tet[0][1] = 0;  //row

    tet[1][0] = 4;  //col
    tet[1][1] = 0;  //row

    tet[2][0] = 5;  //col
    tet[2][1] = 0;  //row

    tet[3][0] = 3;  //col
    tet[3][1] = 1;  //row
    break;
  }
  //Checks if the game ends by checking if the point
  //-where the tet spawns is already taken.
  for (int8_t i = 0; i < MAXTETSIZE; i++) {
    if ((lastOutput[tet[i][1]] & (1 << tet[i][0])) >> tet[i][0] == 1) {
      gameEnd();
      return;
    }
  }
  //Merges the arrays afterwards.
  mergeArrays();
}

//Takes the tetrisMap and lastOutput arrays and merges them into
//-the output array
uint8_t mergeArrays() {
  //Varribles to hold the pop of the arrays
  uint8_t popCurrentframe = 0;
  //Takes the tet blocks and places them onto the
  //-tetrisMap on the position of thereof.
  for (int8_t i = 0, mask = 1; i < MAXY; i++) {
    tetrisMap[i] = 0;
    for (int8_t k = 0; k < MAXTETSIZE; k++) {
      if (i == tet[k][1]) {
        tetrisMap[i] |= (mask << (tet[k][0]));
      }
    }
  }
  //Counts the total amount of 1s' within the
  //-two arrays
  for (int8_t i = 0; i < MAXY; i++) {
    popCurrentframe += popcount(lastOutput[i] | tetrisMap[i]);
  }
  //If there is not a collision then it will make the 
  if (popLastframe <= popCurrentframe) {
    popLastframe = popCurrentframe;
  }
  //If the preveious count does not equals
  //-the next count then it will return
  //-a high. Else a low
  return popLastframe == popCurrentframe;
}
//Shifts right if it is possible to shift right
//-else it keep in place
void shiftRight() {
  if (tet[0][0] + 1 < MAXX &&
    tet[1][0] + 1 < MAXX &&
    tet[2][0] + 1 < MAXX &&
    tet[3][0] + 1 < MAXX) {
    for (int8_t t = 0; t < MAXTETSIZE; t++) {
      Right();
    }

    //Undoes if the arrays can't be merged
    if (!mergeArrays()) {
      for (int8_t t = 0; t < MAXTETSIZE; t++) {
        Left();
      }
      mergeArrays();
    }
  }
}
//Shifts left if it is possible to shift left
//-else it keep in place
void shiftLeft() {
  if (tet[0][0] - 1 > MINX &&
    tet[1][0] - 1 > MINX &&
    tet[2][0] - 1 > MINX &&
    tet[3][0] - 1 > MINX) {
    for (int8_t t = 0; t < MAXTETSIZE; t++) {
      Left();
    }
    //Undoes if the arrays can't be merged
    if (!mergeArrays()) {
      for (int8_t t = 0; t < MAXTETSIZE; t++) {
        Right();
      }
      mergeArrays();
    }
  }
}
//Shifts downwards if it is possable
//-else it will end the turn and it 
//-will generate a new tetris set
void shiftDown() {
  //Checks if it is at the bottom of the array
  if (tet[0][1] + 1 < MAXY &&
    tet[1][1] + 1 < MAXY &&
    tet[2][1] + 1 < MAXY &&
    tet[3][1] + 1 < MAXY) {
    //Shifts the tet downwards.
    for (int8_t t = 0; t < MAXTETSIZE; t++)
      Down();
    //If can't be merged then it will undo the
    //-merge and then it will create a new tetri
    if (!mergeArrays()) {
      for (int8_t t = 0; t < MAXTETSIZE; t++) {
        Up();
      }
      mergeArrays();
      newTetri();
    }
  }
  //If it is the bottom of the array,
  //-merges what there is and then create a new tetri
  else {
    newTetri();
  }

}
//Attempts to rotate the tetri blocks
void rotate() {
  switch (type)
  {
    //rotates line
  case 0:
    if (rotation % 2) {
      tet[0][0] += 1;
      tet[0][1] -= 1;

      //tet[1][n] is center block

      tet[2][0] -= 1;
      tet[2][1] += 1;

      tet[3][0] -= 2;
      tet[3][1] += 2;
    }
    else {
      tet[0][0] -= 1;
      tet[0][1] += 1;

      //tet[1][n] is center block

      tet[2][0] += 1;
      tet[2][1] -= 1;

      tet[3][0] += 2;
      tet[3][1] -= 2;
    }
    break;
    //does nothing because squre is idenity.
  case 1:
    break;
    //rotates t
  case 2:
    //[][][]
    //  []
    if (!rotation) {
      tet[0][0] += 1;
      tet[0][1] -= 1;

      tet[2][0] += 1;
      tet[2][1] -= 1;

      tet[3][0] += 1;
      tet[3][1] -= 1;
    }
    //  []
    //[][]
    //  []
    else if (rotation == 1) {
      tet[0][0] = 0;
      tet[0][1] -= 1;

      //tet[1][n] is center block

      tet[2][0] -= 1;
      tet[2][1] += 1;

      tet[3][0] -= 2;
      tet[3][1] += 2;
    }
    //  []
    //[][][]
    else if (rotation == 2) {
      tet[0][0] = 0;
      tet[0][1] -= 1;

      //tet[1][n] is center block

      tet[2][0] -= 1;
      tet[2][1] += 1;

      tet[3][0] -= 2;
      tet[3][1] += 2;
    }
    //[]
    //[][]
    //[]
    else {
      tet[0][0] = 0;
      tet[0][1] -= 1;

      //tet[1][n] is center block

      tet[2][0] -= 1;
      tet[2][1] += 1;

      tet[3][0] -= 2;
      tet[3][1] += 2;
    }
    break;
    //rotates l
  case 3:
    if (!rotation) {

    }
    else if (rotation == 1) {

    }
    else if (rotation == 2) {

    }
    else {

    }
    break;
  }
  rotation++;
}
//Checks rows and if the row is fill it clears
//-the row and then it will shifts downwards.
void rowCheck()
{
  uint8_t allClear = 1;
  while (allClear)
  {
    allClear = 0;
    for (int8_t k = MAXY - 1; k >= 0; k -= 1)
    {
      if (lastOutput[k] == -1)
      {
        for (int8_t j = k; j > 0; j -= 1) {
          lastOutput[j] = lastOutput[j - 1];
        }
        lastOutput[0] = 0;
        popLastframe -= WORDSIZE;
        allClear = 1;
      }
    }
  }
}
//Counts the amounts of 1 within byte x
uint8_t popcount(uint8_t x)
{
  x = x - ((x >> 1) & 0x55555555);
  x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
  x = (x + (x >> 4)) & 0x0F0F0F0F;
  x = x + (x >> 8);
  x = x + (x >> 16);
  return x & 0x0000003F;
}

//=====Start functions of the libarary=====//
// Matrix registers
#define REG_NOOP   0x00
#define REG_DIGIT0 0x01
#define REG_DIGIT1 0x02
#define REG_DIGIT2 0x03
#define REG_DIGIT3 0x04
#define REG_DIGIT4 0x05
#define REG_DIGIT5 0x06
#define REG_DIGIT6 0x07
#define REG_DIGIT7 0x08
#define REG_DECODEMODE  0x09
#define REG_INTENSITY   0x0A
#define REG_SCANLIMIT   0x0B
#define REG_SHUTDOWN    0x0C
#define REG_DISPLAYTEST 0x0F

void MatrixStart(uint8_t data, uint8_t clock, uint8_t load, uint8_t brightness, uint8_t screens /* = 1 */)
{
  // record pins for sw spi
  _pinData = data;
  _pinClock = clock;
  _pinLoad = load;

  // set ddr for sw spi pins
  pinMode(_pinClock, OUTPUT);
  pinMode(_pinData, OUTPUT);
  pinMode(_pinLoad, OUTPUT);

  // allocate screenbuffers
  _screens = screens;
  _buffer = (uint8_t*)calloc(_screens, 64);
  _maximumX = (_screens * 8);

  // initialize registers
  clear();             // clear display
  setScanLimit(0x07);  // use all rows/digits
  setBrightness(brightness); // maximum brightness 0x0F
  setRegister(REG_SHUTDOWN, 0x01);    // normal operation
  setRegister(REG_DECODEMODE, 0x00);  // pixels not integers
  setRegister(REG_DISPLAYTEST, 0x00); // not in test mode
}

void putByte(uint8_t data)
{
  uint8_t i = 8;
  uint8_t mask;
  while (i > 0) {
    mask = 0x01 << (i - 1);         // get bitmask
    digitalWrite(_pinClock, LOW);   // tick
    if (data & mask) {               // choose bit
      digitalWrite(_pinData, HIGH); // set 1
    }
    else {
      digitalWrite(_pinData, LOW);  // set 0
    }
    digitalWrite(_pinClock, HIGH);  // tock
    --i;                            // move to lesser bit
  }
}

void setRegister(uint8_t reg, uint8_t data)
{
  digitalWrite(_pinLoad, LOW); // begin
  for (uint8_t i = 0; i < _screens; ++i) {
    putByte(reg);  // specify register
    putByte(data); // send data
  }
  digitalWrite(_pinLoad, HIGH);  // latch in data
  digitalWrite(_pinLoad, LOW); // end
}

// syncs row of display with buffer
void syncRow(uint8_t row)
{
  if (!_buffer) return;

  // uint8_t's can't be negative, so don't test for negative row
  if (row >= 8) return;
  digitalWrite(_pinLoad, LOW); // begin
  for (uint8_t i = 0; i < _screens; ++i) {
    putByte(8 - row);                // specify register
    putByte(_buffer[row + (8 * i)]); // send data
  }
  digitalWrite(_pinLoad, HIGH);  // latch in data
  digitalWrite(_pinLoad, LOW); // end
}

void setScanLimit(uint8_t value)
{
  setRegister(REG_SCANLIMIT, value & 0x07);
}

// sets brightness of the display
void setBrightness(uint8_t value)
{
  setRegister(REG_INTENSITY, value & 0x0F);
}

void buffer(uint8_t x, uint8_t y, uint8_t value)
{
  if (!_buffer) return;

  // uint8_t's can't be negative, so don't test for negative x and y.
  if (x >= _maximumX || y >= 8) return;

  uint8_t offset = x; // record x
  x %= 8;             // make x relative to a single matrix
  offset -= x;        // calculate buffer offset

  // wrap shift relative x for nexus module layout
  if (x == 0) {
    x = 8;
  }
  --x;

  // record value in buffer
  if (value) {
    _buffer[y + offset] |= 0x01 << x;
  }
  else {
    _buffer[y + offset] &= ~(0x01 << x);
  }
}

// buffers and writes to screen
void write(uint8_t x, uint8_t y, uint8_t value)
{
  buffer(x, y, value);

  // update affected row
  syncRow(y);
}

// clears screens and buffers
void clear(void)
{
  if (!_buffer) return;

  // clear buffer
  for (uint8_t i = 0; i < 8; ++i) {
    for (uint8_t j = 0; j < _screens; ++j) {
      _buffer[i + (8 * j)] = 0x00;
    }
  }

  // clear registers
  for (uint8_t i = 0; i < 8; ++i) {
    syncRow(i);
  }
}




