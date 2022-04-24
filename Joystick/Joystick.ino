int VRx = A0;
int VRy = A1;
int SW = 2;

int xPosition = 0;
int yPosition = 0;
int SW_state = 0;
int mapX = 0;
int mapY = 0;

void setup() {
  Serial.begin(9600); 
  pinMode(VRx, INPUT);
  pinMode(VRy, INPUT);
  pinMode(SW, INPUT_PULLUP); 
  
}

void loop() {
  xPosition = analogRead(VRx);
  yPosition = analogRead(VRy);
  SW_state = digitalRead(SW);
  xPosition = map(xPosition, 0, 1023, -512, 512);
  yPosition = map(yPosition, 0, 1023, -512, 512);
  
  Serial.print("X: ");
  Serial.print(xPosition);
  Serial.print(" | Y: ");
  Serial.print(yPosition);
  Serial.print(" | Button: ");
  Serial.println(SW_state);
  // ---------------
  if (yPosition < -20) {
    Serial.println("UP");
  }
  else if (yPosition > 500) {
    Serial.println("DOWN");
  }
  if (xPosition < -20) {
    Serial.println("LEFT");
  }
  else if (xPosition > 500) {
    Serial.println("RIGHT");
  }
  if (SW_state == LOW) {
    Serial.println("A");
  }
  delay(100);
  
}
