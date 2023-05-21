/* LaserText projector v1.0 by Ben Makes Everything
   https://youtu.be/u9TpJ-_hBR8
   This code is very badly optimized - I am working on improvements but feel free to use it for reference
*/
#include <PGMWrap.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial(8, 9); // RX, TX

int mirrorPin = 3; //Mirror tab counter sensor
int revPin = 2;    //Revolution tab counter sensor
int laserPin = 6;  // Laser output

volatile int mirrorFlag = 0; //Counts how many mirrors have passed lower sensor
int mirrorFlagOld = 0; //stores previous value of mirror flag

int frameOffset = 60; //Microsecond value for centering image, dependant on RPM
int centerVal = 800;
int avgWidth = 36;

const byte numChars = 22; //MAX characters value (2chars are used at the end of each string on bluettoth, so really its 20)
char myData[numChars]; // an array to store the received data
boolean newData = false; // for checking serial

const int rows = 12;   //Define rows (should be same as # of mirrors)
const int columns = 5; //Defines number of line segments

int mCount = 0;
int wait = 0; //On/off time of laser

//These are the Characters that can be displayed represented as 2D array
const int triangle[ rows ][ columns ] PROGMEM = { //Triangle shape
  {10, -290},      //1
  {35, -265},      //2
  {60, -240},      //3
  {85, -215},      //4
  {110, -190},     //5
  {135, -165},     //6
  {160, -140},     //7
  {185, -115},     //8
  {210, -90},      //9
  {235, -65},      //10
  {260, -40},      //11
  {285, -15}       //12
};

const int block[ rows ][ columns ] PROGMEM = { //Square shape
  {40},
  {40},
  {40},
  {40},
  {40},
  {40},
  {40},
  {40},
  {40},
  {40},
  {40},
  {40},
};

const int space[ rows ][ columns ] PROGMEM = { //Space bar
  { -20},
  { -20},
  { -20},
  { -20},
  { -20},
  { -20},
  { -20},
  { -20},
  { -20},
  { -20},
  { -20},
  { -20}
};

const int a[ rows ][ columns ] PROGMEM = {
  { -8, 16, -10}, //34us
  { -4, 24, -6},
  { -2, 28, -4},
  { -1, 8, -2, 8, -15},
  { -1, 8, -2, 8, -15},
  { -1, 8, -2, 8, -15},
  { -1, 33},
  { -1, 33},
  { -1, 8, -2, 8, -15},
  { -1, 8, -2, 8, -15},
  { -1, 8, -2, 8, -15},
  { -1, 8, -2, 8, -15}
};

const int b[ rows ][ columns ] PROGMEM = {
  { -6, 30},    //36us
  { -3, 33},
  {0, 8, -3, 8, -17},
  {0, 8, -3, 8, -17},
  {0, 8, -3, 8, -17},
  { -6, 30},
  { -6, 30},
  {0, 8, -3, 8, -17},
  {0, 8, -3, 8, -17},
  {0, 8, -3, 8, -17},
  { -3, 33},
  { -6, 30},
};

const int c[ rows ][ columns ] PROGMEM = {
  { -8, 16, -8}, //32us
  { -4, 24, -4},
  { -16, 8, -8},
  { -20, 8, -4},
  { -22, 8, -2},
  { -24, 8},
  { -24, 8},
  { -22, 8, -2},
  { -20, 8, -4},
  { -16, 8, -8},
  { -4, 24, -4},
  { -8, 16, -8},
};

const int d[ rows ][ columns ] PROGMEM = {
  { -12, 26, 0, 0, 0}, //38us
  { -6, 32, 0, 0, 0},
  { -3, 6, 0, 6, -24},
  { -1, 6, -2, 5, -24},
  { -1, 6, -3, 5, -23},
  { -1, 5, -5, 5, -22},
  { -1, 5, -5, 5, -22},
  { -1, 6, -4, 5, -22},
  { -1, 6, -3, 5, -23},
  { -3, 6, 0, 6, -24},
  { -6, 32, 0, 0, 0},
  { -12, 26, 0, 0, 0}
};

const int e[ rows ][ columns ] PROGMEM = {
  { -1, 31}, //32us
  { -1, 31},
  { -25, 7},
  { -25, 7},
  { -25, 7},
  { -10, 22},
  { -10, 22},
  { -25, 7},
  { -25, 7},
  { -25, 7},
  { -1, 31},
  { -1, 31}
};

const int f[ rows ][ columns ] PROGMEM = {
  { -1, 30, -1}, //32us
  { -1, 30, -1},
  { -23, 8, -1},
  { -23, 8, -1},
  { -23, 8, -1},
  { -8, 22},
  { -8, 22},
  { -23, 8, -1},
  { -23, 8, -1},
  { -23, 8, -1},
  { -23, 8, -1},
  { -23, 8, -1}
};

const int g[ rows ][ columns ] PROGMEM = {
  { -5, 30, -5}, //40us
  { -1, 36, -3},
  { -30, 6, -4},
  { -32, 6, -2},
  { -34, 6},
  { -34, 6},
  { -1, 14, 0, 6, -19},
  { -1, 14, 0, 6, -19},
  { -1, 8, -4, 6, -21},
  { -1, 8, -4, 6, -21},
  { -1, 36, -3},
  { -5, 30, -5},
};

const int h[ rows ][ columns ] PROGMEM = {
  { -1, 7, -4, 7, -13}, //32us
  { -1, 7, -4, 7, -13},
  { -1, 7, -4, 7, -13},
  { -1, 7, -4, 7, -13},
  { -1, 7, -4, 7, -13},
  { -1, 31},
  { -1, 31},
  { -1, 7, -4, 7, -13},
  { -1, 7, -4, 7, -13},
  { -1, 7, -4, 7, -13},
  { -1, 7, -4, 7, -13},
  { -1, 7, -4, 7, -13}
};

const int i[ rows ][ columns ] PROGMEM = {
  { -1, 31}, //32us
  { -1, 31},
  { -12, 6, -14},
  { -12, 6, -14},
  { -12, 6, -14},
  { -12, 6, -14},
  { -12, 6, -14},
  { -12, 6, -14},
  { -12, 6, -14},
  { -12, 6, -14},
  { -1, 31},
  { -1, 31},
};

const int j[ rows ][ columns ] PROGMEM = {
  { -1, 31},  //32us
  { -1, 31},
  { -10, 8, -14},
  { -10, 8, -14},
  { -10, 8, -14},
  { -10, 8, -14},
  { -10, 8, -14},
  { -10, 8, -14},
  { -10, 8, -1, 7, -8},
  { -10, 8, -1, 7, -8},
  { -10, 22},
  { -14, 18}
};

const int k[ rows ][ columns ] PROGMEM = {
  { -1, 7, -8, 7, -21}, //44us
  { -3, 7, -6, 7, -21},
  { -5, 7, -4, 7, -21},
  { -7, 7, -2, 7, -21},
  { -9, 7, 0, 7, -21},
  { -10, 34},
  { -10, 34},
  { -9, 7, 0, 7, -21},
  { -7, 7, -2, 7, -21},
  { -5, 7, -4, 7, -21},
  { -3, 7, -6, 7, -21},
  { -1, 7, -8, 7, -21}
};

const int l[ rows ][ columns ] PROGMEM = {
  { -14, 6, -12}, //32us
  { -14, 6, -12},
  { -14, 6, -12},
  { -14, 6, -12},
  { -14, 6, -12},
  { -14, 6, -12},
  { -14, 6, -12},
  { -14, 6, -12},
  { -14, 6, -12},
  { -14, 6, -12},
  {32},
  {32}
};

const int m[ rows ][ columns ] PROGMEM = {
  { -1, 2, -20, 2, -17}, //42us
  { -1, 4, -16, 4, -17},
  { -1, 6, -12, 6, -17},
  { -1, 8, -8, 8, -17},
  { -1, 10, -4, 10, -17},
  { -1, 12, 0, 12, -17},
  { -1, 41},
  { -1, 41},
  { -1, 8, -6, 8, -17},
  { -1, 8, -6, 8, -17},
  { -1, 8, -6, 8, -17},
  { -1, 8, -6, 8, -17},
};

const int n[ rows ][ columns ] PROGMEM = {
  { -1, 6, -10, 2, -21}, //40us
  { -1, 6, -8, 4, -21},
  { -1, 6, -4, 6, -23},
  { -1, 6, -1, 8, -24},
  { -1, 6, 0, 10, -23},
  { -1, 38, -1},
  { -1, 38, -1},
  { -1, 10, 0, 6, -23},
  { -1, 8, -1, 6, -24},
  { -1, 6, -4, 6, -23},
  { -1, 4, -8, 6, -21},
  { -1, 2, -10, 6, -21},
};

const int o[ rows ][ columns ] PROGMEM = {
  { -8, 20, -4, 0, 0}, //32us
  { -4, 28, 0, 0, 0},
  { -3, 6, 0, 6, -17},
  { -1, 6, -2, 6, -17},
  { -1, 6, -2, 6, -17},
  { -1, 6, -2, 6, -17},
  { -1, 6, -2, 6, -17},
  { -1, 6, -2, 6, -17},
  { -1, 6, -2, 6, -17},
  { -3, 6, 0, 6, -17},
  { -4, 28, 0, 0, 0},
  { -8, 20, -4, 0, 0}
};

const int p[ rows ][ columns ] PROGMEM = {
  { -10, 25, -1}, //36us
  { -6, 30},
  { -1, 4, -2, 5, -24},
  { -1, 4, -2, 5, -24},
  { -1, 4, -2, 5, -24},
  { -6, 30},
  { -10, 25, -1},
  { -30, 6},
  { -30, 6},
  { -30, 6},
  { -30, 6},
  { -30, 6}
};

const int q[ rows ][ columns ] PROGMEM = {
  { -8, 26, -8, 0, 0}, //42us
  { -4, 32, -6, 0, 0},
  { -3, 6, -2, 6, -25},
  { -3, 6, -2, 6, -25},
  { -3, 6, -2, 6, -25},
  { -3, 6, -2, 6, -25},
  { -3, 6, -2, 6, -25},
  { -3, 6, -2, 6, -25},
  { -8, 34, 0},
  { -11, 31, 0},
  { -1, 39, -2},
  { -1, 35, -6}
};

const int r[ rows ][ columns ] PROGMEM = {
  { -10, 27, -3}, //40us
  { -6, 32, -2},
  { -1, 4, -1, 5, -29},
  { -1, 4, -1, 5, -29},
  { -1, 4, -1, 5, -29},
  { -6, 34},
  { -10, 30},
  { -21, 19},
  { -16, 24},
  { -11, 29},
  { -6, 5, -1, 6, -22},
  { -1, 5, -4, 7, -23}
};

const int s[ rows ][ columns ] PROGMEM = {
  { -6, 30}, //36us
  { -1, 35},
  { -27, 9},
  { -27, 6, -3},
  { -20, 8, -8},
  { -12, 13, -11},
  { -10, 13, -13},
  { -4, 10, -22},
  { -5, 6, -25},
  { -1, 8, -27},
  { -1, 35},
  { -6, 30}
};

const int t[ rows ][ columns ] PROGMEM = {
  { -1, 31}, //32us
  { -1, 31},
  { -12, 6, -14},
  { -12, 6, -14},
  { -12, 6, -14},
  { -12, 6, -14},
  { -12, 6, -14},
  { -12, 6, -14},
  { -12, 6, -14},
  { -12, 6, -14},
  { -12, 6, -14},
  { -12, 6, -14}
};

const int u[ rows ][ columns ] PROGMEM = {
  { -1, 5, -2, 5, -21}, //34us
  { -1, 5, -2, 5, -21},
  { -1, 5, -2, 5, -21},
  { -1, 5, -2, 5, -21},
  { -1, 5, -2, 5, -21},
  { -1, 5, -2, 5, -21},
  { -1, 5, -2, 5, -21},
  { -1, 5, -2, 5, -21},
  { -1, 5, -2, 5, -21},
  { -1, 5, 0, 5, -23},
  { -7, 27},
  { -11, 23}
};

const int v[ rows ][ columns ] PROGMEM = {
  { -1, 5, -2, 5, -23}, //36us
  { -1, 5, -2, 5, -23},
  { -1, 5, -2, 5, -23},
  { -1, 5, -2, 5, -23},
  { -1, 5, -2, 5, -23},
  { -1, 5, -2, 5, -23},
  { -3, 5, 0, 5, -23},
  { -7, 29},
  { -11, 25},
  { -15, 21},
  { -19, 17},
  { -23, 13}
};

const int w[ rows ][ columns ] PROGMEM = {
  { -1, 8, -6, 8, -17}, //40us
  { -1, 8, -6, 8, -17},
  { -1, 8, -6, 8, -17},
  { -1, 8, -6, 8, -17},
  { -1, 41},
  { -1, 41},
  { -1, 12, 0, 12, -17},
  { -1, 10, -4, 10, -17},
  { -1, 8, -8, 8, -17},
  { -1, 6, -12, 6, -17},
  { -1, 4, -16, 4, -17},
  { -1, 2, -20, 2, -17},
};

const int x[ rows ][ columns ] PROGMEM = {
  { -1, 5, -22, 5, -13}, //46us
  { -4, 5, -16, 5, -16},
  { -7, 5, -10, 5, -19},
  { -10, 5, -6, 5, -20},
  { -13, 5, 0, 5, -23},
  { -18, 28},
  { -18, 28},
  { -13, 5, 0, 5, -23},
  { -10, 5, -6, 5, -20},
  { -7, 5, -10, 5, -19},
  { -4, 5, -16, 5, -16},
  { -1, 5, -22, 5, -13},
};

const int y[ rows ][ columns ] PROGMEM = {
  { -1, 5, -22, 5, -13}, //46us
  { -4, 5, -16, 5, -16},
  { -7, 5, -10, 5, -19},
  { -10, 5, -6, 5, -20},
  { -13, 5, 0, 5, -23},
  { -18, 28},
  { -24, 22},
  { -30, 6, -10},
  { -30, 6, -10},
  { -30, 6, -10},
  { -30, 6, -10},
  { -30, 6, -10},
};

const int z[ rows ][ columns ] PROGMEM = {
  { -1, 35}, //36us
  { -1, 35},
  { -3, 6, -27},
  { -6, 6, -24},
  { -9, 6, -21},
  { -12, 6, -18},
  { -15, 6, -15},
  { -18, 6, -12},
  { -21, 6, -9},
  { -24, 6, -6},
  { -1, 35},
  { -1, 35}
};

const int one[ rows ][ columns ] PROGMEM = {
  { -12, 20}, //32us
  { -12, 20},
  { -12, 6, -14},
  { -12, 6, -14},
  { -12, 6, -14},
  { -12, 6, -14},
  { -12, 6, -14},
  { -12, 6, -14},
  { -12, 6, -14},
  { -12, 6, -14},
  { -1, 31},
  { -1, 31},
};

const int two[ rows ][ columns ] PROGMEM = {
  { -5, 31}, //36us
  { -1, 35},
  { -1, 6, 0, 2, -27},
  { -1, 6, -29},
  { -1, 6, -29},
  { -6, 12, -18},
  { -15, 12, -9},
  { -26, 9, -1},
  { -29, 6, -1},
  { -29, 6, -1},
  { -1, 35},
  { -1, 35}
};

const int three[ rows ][ columns ] PROGMEM = {
  { -6, 30}, //36us
  { -3, 33},
  {0, 8, -28},
  {0, 8, -28},
  {0, 8, -28},
  { -6, 30},
  { -6, 30},
  {0, 8, -28},
  {0, 8, -28},
  {0, 8, -28},
  { -3, 33},
  { -6, 30},
};

const int four[ rows ][ columns ] PROGMEM = {
  { -1, 5, -2, 5, -19}, //32us
  { -1, 5, -2, 5, -19},
  { -1, 5, -2, 5, -19},
  { -1, 5, -2, 5, -19},
  { -1, 5, -2, 5, -19},
  { -1, 31},
  { -1, 31},
  { -1, 5, -26},
  { -1, 5, -26},
  { -1, 5, -26},
  { -1, 5, -26},
  { -1, 5, -26},
};

const int five[ rows ][ columns ] PROGMEM = {
  { -1, 30, -1}, //32us
  { -1, 30, -1},
  { -26, 5, -1},
  { -26, 5, -1},
  { -26, 5, -1},
  { -9, 23},
  { -7, 25},
  { -1, 5, -26},
  { -1, 5, -26},
  { -3, 5, -24},
  { -2, 29, -1},
  { -7, 24, -1}
};

const int six[ rows ][ columns ] PROGMEM = {
  { -5, 22, -13}, //40us
  { -1, 28, -11},
  { -28, 6, -6},
  { -32, 6, -2},
  { -34, 6},
  { -34, 6},
  { -10, 29, -1},
  { -6, 33, -1},
  { -1, 2, 0, 2, -35},
  { -1, 2, 0, 2, -35},
  { -1, 39},
  { -5, 35},
};

const int sev[ rows ][ columns ] PROGMEM = {
  { -1, 35}, //36us
  { -1, 35},
  { -3, 6, -27},
  { -6, 6, -24},
  { -9, 6, -21},
  { -12, 6, -18},
  { -15, 6, -15},
  { -18, 6, -12},
  { -21, 6, -9},
  { -24, 6, -6},
  { -27, 6, -3},
  { -30, 6}
};

const int egt[ rows ][ columns ] PROGMEM = {
  { -6, 34}, //40us
  { -3, 37},
  { -1, 2, 0, 2, -35},
  {0, 2, -2, 2, -34},
  { -1, 2, 0, 1, -36},
  { -6, 30, -4},
  { -6, 30, -4},
  { -1, 2, 0, 1, -36},
  {0, 2, -2, 2, -34},
  { -1, 2, 0, 2, -35},
  { -3, 37},
  { -6, 34},
};

const int nine[ rows ][ columns ] PROGMEM = {
  { -9, 27, -4}, //40us
  { -5, 35},
  { -1, 3, 0, 3, -33},
  { -1, 3, -1, 3, -32},
  { -1, 3, -1, 3, -32},
  { -1, 3, 0, 4, -32},
  { -1, 39},
  { -3, 33, -4},
  { -5, 3, -32},
  { -8, 3, -29},
  { -12, 3, -25},
  { -17, 3, -20}
};

const int sc[ rows ][ columns ] PROGMEM = { //semicolon
  { -1},
  { -1},
  {1},
  {1},
  { -1},
  { -1},
  { -1},
  { -1},
  {1},
  {1},
  { -1},
  { -1}
};

const int per[ rows ][ columns ] PROGMEM = {  //period
  { -1},
  { -1},
  { -1},
  { -1},
  { -1},
  { -1},
  { -1},
  { -1},
  { -1},
  { -1},
  {1},
  {1}
};

const int exc[ rows ][ columns ] PROGMEM = { //exclamation
  {1},
  {1},
  {1},
  {1},
  {1},
  {1},
  {1},
  {1},
  { -1},
  { -1},
  {1},
  {1}
};

int pulses[ rows ][ columns ];

void setup() {
  Serial.begin(9600);
  mySerial.begin(115200);
  pinMode(mirrorPin, INPUT_PULLUP); //Mirror counter
  pinMode(revPin, INPUT_PULLUP);    //Full rotation counter
  pinMode(laserPin, OUTPUT);        //Laser configured as output
  attachInterrupt(digitalPinToInterrupt(mirrorPin), ISR_mirror, RISING);
  attachInterrupt(digitalPinToInterrupt(revPin), ISR_rev, RISING);
}

void loop() {
  receiveData();
  showNewData();

  if (mirrorFlag != mirrorFlagOld) {
    mirrorFlagOld = mirrorFlag;
    if (mirrorFlag == 1) { //line 1
      delayMicroseconds(frameOffset + (centerVal - (strlen(myData)*avgWidth)));
      encodePulses(0);
    }
    else if (mirrorFlag == 2) { //line 2
      delayMicroseconds(frameOffset + 24 + (centerVal - (strlen(myData)*avgWidth)));
      encodePulses(1);
    }
    else if (mirrorFlag == 3) { //line 3
      delayMicroseconds(frameOffset + 41 + (centerVal - (strlen(myData)*avgWidth)));
      encodePulses(2);
    }
    else if (mirrorFlag == 4) { //line 4
      delayMicroseconds(frameOffset + 27 + (centerVal - (strlen(myData)*avgWidth)));
      encodePulses(3);
    }
    else if (mirrorFlag == 5) { //line 1
      delayMicroseconds(frameOffset - 15 + (centerVal - (strlen(myData)*avgWidth)));
      encodePulses(4);
    }
    else if (mirrorFlag == 6) { //line 1
      delayMicroseconds(frameOffset + 1 + (centerVal - (strlen(myData)*avgWidth)));
      encodePulses(5);
    }
    else if (mirrorFlag == 7) { //line 1
      delayMicroseconds(frameOffset + 4 + (centerVal - (strlen(myData)*avgWidth)));
      encodePulses(6);
    }
    else if (mirrorFlag == 8) { //line 1
      delayMicroseconds(frameOffset + 58 + (centerVal - (strlen(myData)*avgWidth)));
      encodePulses(7);
    }
    else if (mirrorFlag == 9) { //line 1
      delayMicroseconds(frameOffset + 24 + (centerVal - (strlen(myData)*avgWidth)));
      encodePulses(8);
    }
    else if (mirrorFlag == 10) { //line 1
      delayMicroseconds(frameOffset + 44 + (centerVal - (strlen(myData)*avgWidth)));
      encodePulses(9);
    }
    else if (mirrorFlag == 11) { //line 1
      delayMicroseconds(frameOffset + 13 + (centerVal - (strlen(myData)*avgWidth)));
      encodePulses(10);
    }
    else if (mirrorFlag == 12) { //line 1
      delayMicroseconds(frameOffset - 2 + (centerVal - (strlen(myData)*avgWidth)));
      encodePulses(11);
    }
  }
}

void ISR_mirror() {
  mirrorFlag++;
}

void ISR_rev() {
  mirrorFlag = 0;
}

void encodePulses(int mirrorNum) {
  for (int mCount = (strlen(myData) - 1); mCount >= 0; mCount--) {
    for (int segment = 0; segment < 5; segment++) {

      if (myData[mCount] == ' ') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&space[mirrorNum][segment]);
      }
      else if (myData[mCount] == '/') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&triangle[mirrorNum][segment]);
      }
      else if (myData[mCount] == ';') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&block[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'a' || myData[mCount] == 'A') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&a[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'b' || myData[mCount] == 'B') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&b[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'c' || myData[mCount] == 'C') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&c[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'd' || myData[mCount] == 'D') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&d[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'e' || myData[mCount] == 'E') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&e[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'f' || myData[mCount] == 'F') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&f[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'g' || myData[mCount] == 'G') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&g[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'h' || myData[mCount] == 'H') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&h[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'i' || myData[mCount] == 'I') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&i[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'j' || myData[mCount] == 'J') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&j[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'k' || myData[mCount] == 'K') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&k[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'l' || myData[mCount] == 'L') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&l[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'm' || myData[mCount] == 'M') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&m[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'n' || myData[mCount] == 'N') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&n[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'o' || myData[mCount] == 'O') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&o[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'p' || myData[mCount] == 'P') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&p[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'q' || myData[mCount] == 'Q') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&q[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'r' || myData[mCount] == 'R') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&r[mirrorNum][segment]);
      }
      else if (myData[mCount] == 's' || myData[mCount] == 'S') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&s[mirrorNum][segment]);
      }
      else if (myData[mCount] == 't' || myData[mCount] == 'T') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&t[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'u' || myData[mCount] == 'U') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&u[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'v' || myData[mCount] == 'V') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&v[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'w' || myData[mCount] == 'W') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&w[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'x' || myData[mCount] == 'X') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&x[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'y' || myData[mCount] == 'Y') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&y[mirrorNum][segment]);
      }
      else if (myData[mCount] == 'z' || myData[mCount] == 'Z') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&z[mirrorNum][segment]);
      }
      else if (myData[mCount] == '0') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&o[mirrorNum][segment]);
      }
      else if (myData[mCount] == '1') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&one[mirrorNum][segment]);
      }
      else if (myData[mCount] == '2') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&two[mirrorNum][segment]);
      }
      else if (myData[mCount] == '3') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&three[mirrorNum][segment]);
      }
      else if (myData[mCount] == '4') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&four[mirrorNum][segment]);
      }
      else if (myData[mCount] == '5') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&five[mirrorNum][segment]);
      }
      else if (myData[mCount] == '6') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&six[mirrorNum][segment]);
      }
      else if (myData[mCount] == '7') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&sev[mirrorNum][segment]);
      }
      else if (myData[mCount] == '8') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&egt[mirrorNum][segment]);
      }
      else if (myData[mCount] == '9') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&nine[mirrorNum][segment]);
      }
      else if (myData[mCount] == ':') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&sc[mirrorNum][segment]);
      }
      else if (myData[mCount] == '.') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&per[mirrorNum][segment]);
      }
      else if (myData[mCount] == '!') {
        pulses[mirrorNum][segment] = pgm_read_dword_near(&exc[mirrorNum][segment]);
      }
      //activate laser for specified intervals
      if (pulses[mirrorNum][segment] > 0) {
        digitalWrite(laserPin, HIGH);
        delayMicroseconds(abs(pulses[mirrorNum][segment]));
      }
      else if (pulses[mirrorNum][segment] < 0) {
        digitalWrite(laserPin, LOW);
        delayMicroseconds(abs(pulses[mirrorNum][segment]));
      }
      else {
        digitalWrite(laserPin, LOW);
      }
    }
  }
}

void receiveData() {
  static byte ndx = 0;
  char endMarker = '\n';
  char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();
    if (rc != endMarker) {
      myData[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
      myData[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

void showNewData() {
  if (newData == true) {
    newData = false;
  }
}
