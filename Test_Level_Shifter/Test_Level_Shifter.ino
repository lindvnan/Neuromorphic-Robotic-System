 bool flag = true;
 int T[256][8] = {0}, A[256][8] = {0};

void setup() {
  Serial.begin(9600);
  //set 1.8V reference as VACC of the level shifter
  analogWriteResolution(12);
  analogWrite(DAC1, 2400);
  int index = 0;
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  for(int i0 = 0; i0 <= 1; ++i0) {
      for(int i1 = 0; i1 <= 1; ++i1) {
        for(int i2 = 0; i2 <= 1; ++i2) {
          for(int i3 = 0; i3 <= 1; ++i3) {
            for(int i4 = 0; i4 <= 1; ++i4) {
              for(int i5 = 0; i5 <= 1; ++i5) {
                for(int i6 = 0; i6 <= 1; ++i6) {
                  for(int i7 = 0; i7 <= 1; ++i7) {
                    T[index][0] = i0;
                    T[index][1] = i1;
                    T[index][2] = i2;
                    T[index][3] = i3;
                    T[index][4] = i4;
                    T[index][5] = i5;
                    T[index][6] = i6;
                    T[index][7] = i7;
                    ++index;
                  }
                }
              }
            }
          }
        }
      }
  }
  for (int i=2;i<=8;i++)
  {
    pinMode(i, OUTPUT); 
  }
  pinMode(10,OUTPUT);

  for (int i=22;i<=28;i++)
  {
    pinMode(i, INPUT); 
  }
  pinMode(29,INPUT);
  
  bool led;
  
  int ard_pin_diff0 = 2, ard_pin_diff1 = 22;
  int ard_pin7_0 = 10, ard_pin7_1 = 29;
  
  for (int i = 0; i <= 255; ++i) {
    for (int j = 0; j <= 6; ++j) {
      digitalWrite(j + ard_pin_diff0,T[i][j]);
    }
    digitalWrite(ard_pin7_0,T[i][7]);
    for (int j = 0; j <= 6; ++j) {
      A[i][j] = digitalRead(j + ard_pin_diff1);
    }
    A[i][7] = digitalRead(ard_pin7_1);  

    digitalWrite(12, led);
    led = !led;
  }

 bool oneflag = true;
  for (int i =0; i <= 255; ++i) {
    for (int j = 0; j <= 7; ++j) {
      if (T[i][j] != A[i][j]) {
        flag = false;
        oneflag = false;
      }
    }
    for(int k =0; k <= 7; ++k) {
        Serial.print(T[i][k]);
        Serial.print(" ");
      }
      if (oneflag) {
        // If the result meet the expectation, shows "=="
        Serial.print("== ");
      }
      else { 
        // If the result does not meet the expectation, shows "->"
        Serial.print("-> ");
        oneflag = true;
      }
      for(int k =0; k <= 7; ++k) {
        Serial.print(A[i][k]);
        Serial.print(" ");
      }
      Serial.println("");
  }

  // Shows the test result
  if (flag) {
    Serial.println("Success!");
  }
  else {
    Serial.println("False!");
  }
  digitalWrite(11, HIGH);
}

void loop() {
}
