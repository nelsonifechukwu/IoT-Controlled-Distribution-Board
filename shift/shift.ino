int dataPin =  5;//32; //5
int latchPin = 18;//23; //18
int clockPin = 19;// 33; //19

byte led = 0b00000000;
int dt = 1000;

void setup() {
  Serial.begin(9600);
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
}

void loop() {

  // changeState(led);
  // delay(3000);
  // ~led;

   for (int i =0; i<=7; i++)
 { bitSet(led, i);
  changeState(led);
  delay(3000);
  led = 0b00000000;
  }
 
}

void changeState(byte states){
digitalWrite(latchPin, LOW);
shiftOut(dataPin, clockPin, LSBFIRST, states);
digitalWrite(latchPin, HIGH);

}