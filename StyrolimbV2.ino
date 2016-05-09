/* Styrolimb v2
 * Gary S
 *Copyright (c) 2016 
 */
#include <IRremote.h>
#include <Servo.h>
#define IRPin 2
IRrecv control(IRPin);
Servo Rotary,Tilt,Elevation,Claw;
String signal(long data);
short mode,S[4];
short loops[4][100];
short loopfocus;
bool clawin;
long holding,lastsignal,holding_loop;
const long pwr=(-522174273),etr=(-522184983),one=(-522182433),two=(-522149793),three=(-522166113),diru=(-522189063),dird=(-522156423),dirl=(-522148263),dirr=(-522172743); //Power, Enter, One, Two, Three, Direction (Up, Down, Left, Right)
long chkIR();
void manipulatepos(long sig);
void writepos();
bool loopsarefull();
void manageloops(bool del);
int getLastElement(short loopelement);

void setup() {
  Serial.begin(9600);
  Serial.println("Communication Started.");
  control.enableIRIn();
  Rotary.attach(A0);
  Tilt.attach(A1);
  Elevation.attach(A2);
  Claw.attach(A3);
  S[0] = 0; //Servo 1 (Rotary) : max
  S[1] = 30; //Servo 2 (Tilt)
  S[2] = 30; //Servo 3 (Elevation)
  S[3] = 45; //Servo 4 (Claw)
  mode = 0; //0 = Control, 1 = Programming, 2 = Looping
  holding = 0;
  holding_loop = 0;
  loopfocus = 0;
  clawin = true;
  for(int x=0;x<4;x++) {
    memset(loops[x],-1,sizeof(loops[x]));
  }
}

void loop() {
  lastsignal=chkIR();
  Serial.println("Mode: "+(String)mode);
  Serial.println("Received:" + signal(lastsignal));
  Serial.println("Rotary: "+(String)S[0]+" | Tilt: "+(String)S[1]+" | Elevation: "+(String)S[2]+" | Claw: "+(String)S[3]);
  Serial.println("Looping Elements: "+(String)getLastElement(0));
  if(!mode) { //If mode == 0 (Control)
    Serial.println("CONTROL MODE");
    manipulatepos(lastsignal);
    writepos();
    if(lastsignal==one) {
      mode = 1;
    }
  } else if(mode==1) { //If mode == 1 (Programming)
    //Start position, median positions, final position w/ claw (Press enter to log positions)
    Serial.println("PROGRAMMING MODE");
    manipulatepos(lastsignal);
    writepos();
    if(lastsignal==one) { //Log
      if(!loopsarefull()) { //Loops are NOT full
        manageloops(0); //Add
      } else {

      }
    } else if(lastsignal==two) {
      manageloops(1); //Delete
    } else if(lastsignal==three) {
      mode = 2; //Stop Programming
    }
  } else if(mode==2) { //If mode == 2 (Looping)
    Serial.println("LOOPING MOOE");
    if(lastsignal==one) { //Stop looping & go to manual control. 
      mode = 0;
    }
    if((millis() - holding_loop)>=400) {
      if(loopfocus==(getLastElement(0)-1)) {
        loopfocus = 0;
      } else {
        loopfocus++;
      }
      S[0] = loops[0][loopfocus];
      S[1] = loops[1][loopfocus];
      S[2] = loops[2][loopfocus];
      S[3] = loops[3][loopfocus];
      writepos();
      holding_loop = millis();
      Serial.println("Set holding loop");
    }
  }
}

String signal(long data) {
  switch(data) {
    case pwr: {
      return F("Power");
    }break;
    case diru: {
      return F("Up");
    }break;
    case dird: {
      return F("Down");
    }break;
    case dirl: {
      return F("Left");
    }break;
    case dirr: {
      return F("Right");
    }break;
    case etr: {
      return F("Enter");
    }break;
    case one: {
      return F("One");
    }break;
    case two: {
      return F("Two");
    }break;
    case three: {
      return F("Three");
    }break;
    default: { //0
      return "NaN";
    } break;
  }
}

long chkIR() {
  decode_results results;
  if(control.decode(&results)) {
    control.resume();
    return results.value;
  } else {
    return 0;
  }
}

void manipulatepos(long sig) {
    //if((millis() - holding)>=1) {
    if(true) {
      switch(sig) {
        case diru: { //Up
          if(S[1]!=90) {
            S[1]+=5;
            if(S[2]!=90) {
              S[2]+=5;
            }
          }
        }break;
        case dird: { //Down
          if(S[1]!=0) {
            S[1]-=5;
            if(S[2]!=0) {
              S[2]-=5;
            }
          }
        }break;
        case dirl: { //Left
          if(S[0]!=0) {
            S[0]-=5;
          }
        }break;
        case dirr: { //Right
          if(S[0]!=90) {
            S[0]+=5;
          }
        }break;
        case etr: { //Center
          if(S[3]==90&&clawin) {
            clawin = false;
          } else if(S[3]==0&&!clawin) {
            clawin = true;
          }
          if(clawin) {
            S[3]+=5;
          } else if(!clawin) {
            S[3]-=5;
          }
        }break;
      }
      holding = millis();
    }
}

void writepos() { //----------------REPLACE AnalogWrites with SERVO WRITES when servos are connected-----------------------//
  Rotary.write(S[0]);
  Tilt.write(S[1]);
  Elevation.write(S[2]);
  Claw.write(S[3]);
  /*analogWrite(A0,map(S[0],0,90,0,225));
  analogWrite(A1,map(S[1],0,90,0,225));
  analogWrite(A2,map(S[2],0,90,0,225));
  analogWrite(A3,map(S[3],0,90,0,225));*/
}

bool loopsarefull() {
  for(int x=0;x<4;x++) {
    if(loops[x][99]>-1){
      return true;
    }
  }
  return false;
}

void manageloops(bool del) {
  if(!del) { //Add one
    if(getLastElement(0)!=-1) {
       loops[0][getLastElement(0)] = S[0];
       loops[1][getLastElement(1)] = S[1];
       loops[2][getLastElement(2)] = S[2];
       loops[3][getLastElement(3)] = S[3];
    }
  } else { //Delete last one 
    loops[0][getLastElement(0)] = -1;
    loops[1][getLastElement(1)] = -1;
    loops[2][getLastElement(2)] = -1;
    loops[3][getLastElement(3)] = -1;
  }
}

int getLastElement(short loopelement) {
  for(int x=0;x<100;x++) {
    if(loops[loopelement][x]==-1) {
      return x;
    }
  }
  return -1;
}

/* Styrolimb v2
 * Gary S
 *Copyright (c) 2016 
 */
