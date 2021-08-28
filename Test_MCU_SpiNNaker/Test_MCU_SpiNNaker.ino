#include <Servo.h>

Servo myservo;
//------------Receive Spikes------------//
int L[7] = { 0 };
int PreviousValue[7] = {0};
int resTWO = 88;       // received data via a serial transfer
int inde = 0;    // used for the index of data symbol array
int dec_data[18] = { 0 }; // 18 data symbols in a package
int cycle = 0;
bool ACK = false;
bool first_spike = true;
int pos[500]={9};
int num_spikes = 0;
int num_pos = 0;
//timer to calculate spike time and control motor
unsigned long ref_time;
int com[10]={0};  //array to store commands
int Num_com = 0;  //number of commands stored 
int ARMpos = 0;

//------------Receive events------------//
String inString = "";    // string to hold input
int x = 0;
int y = 0;
bool X_axis = true;
int Num_event = 0;  //number of events received from DVS
unsigned long previous_event;

// send spikes
bool L_state[7];
bool previous_level = false;
int ChangingBits[2] = {0,0};
int Num_packet = 0;
int ind_Num = 0;    //index of the number in the packet
int packets[50][11] = {0};
unsigned long previous_packet;

// set interval
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
const int interval = 1500;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
// set predefined spikes
int Num_spikes = 5;
int spikes[5] = {0, 2, 3, 5, 7};

/////////////////////////////////////////////////////////
//---------------------- Set Up -----------------------//
/////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);

  myservo.attach(39);
  //set 1.8V reference as VACC of the level shifter
  analogWriteResolution(12);
  analogWrite(DAC1, 2400);
//--------------------RECEIVE events----------------------//
  previous_event = micros();
  pinMode(11,OUTPUT);
  
//--------------------RECEIVE spikes-----------------------//
  // set digital pins to read the data from the SpiNNakerLink
  for (int i=22;i<=28;i++)
  {
    pinMode(i, INPUT); 
  }
  // set a digital pin to send ACK
  pinMode(29,OUTPUT);
  
  PreviousValue[0] = digitalRead(28);
  PreviousValue[1] = digitalRead(27);
  PreviousValue[2] = digitalRead(26);
  PreviousValue[3] = digitalRead(25);
  PreviousValue[4] = digitalRead(24);
  PreviousValue[5] = digitalRead(23);
  PreviousValue[6] = digitalRead(22);
    
//----------------------SEND-------------------------//
  // set digital pins to send the data to SpiNNaker
  for (int i=2;i<=8;i++)
  {
    pinMode(i, OUTPUT); 
    digitalWrite(i,LOW);
    L_state[i-2] = false;
  }
  
  // set a digital pin to receive ACK
  pinMode(10,INPUT);
  pinMode(12,OUTPUT);
  previous_packet = micros();

  //------------- Set Predefined Spikes -------------//
  for (Num_event = 0; Num_event < Num_spikes; ++Num_event) {
    x = spikes[Num_event];         
    switch(x)
    {
      case 0: packets[Num_event][0] = 0;
        break;
      case 1: packets[Num_event][0] = 1;
        break;
      case 2: packets[Num_event][0] = 1;
        break;
      case 3: packets[Num_event][0] = 0;
        break;
      case 4: packets[Num_event][0] = 1;
        break;
      case 5: packets[Num_event][0] = 0;
        break;
      case 6: packets[Num_event][0] = 0;
        break;
      case 7: packets[Num_event][0] = 1;
        break;
      default:
        break;
    }          
    packets[Num_event][1] = 0;
    packets[Num_event][2] = x;
    packets[Num_event][3] = 0;
    packets[Num_event][4] = 0;
    packets[Num_event][5] = 0;
    packets[Num_event][6] = 4;
    packets[Num_event][7] = 3;
    packets[Num_event][8] = 2;
    packets[Num_event][9] = 1;
    packets[Num_event][10] = 99;
  }   
}

/////////////////////////////////////////////////////////
//----------------------- Loop ------------------------//
/////////////////////////////////////////////////////////

void loop() {
  delayMicroseconds(1);     
        // current_time is the time in micro second
        unsigned long current_event = micros();

        if(current_event - previous_event > (interval / 2)) {
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
          analogWrite(11, 500);
          // digitalWrite(11,HIGH);
          previous_event = current_event;
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
        }
        else {
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
          analogWrite(11, 0);
          // digitalWrite(11,LOW);
          //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
        }
        
//--------------Send spikes-------------//  
  bool current_level;
  
  int IN_ack = digitalRead(10);

  if(IN_ack == 1)
  {
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    analogWrite(12, 1500);
    // digitalWrite(12,HIGH);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    current_level = true;
  }
  else
  {
    /~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    analogWrite(12, 0);
    // digitalWrite(12,LOW);
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    current_level = false;
  }

  if(previous_level == current_level)
  {
    unsigned long current_packet = micros();
    if(ind_Num==0)
    {
      if((current_packet-previous_packet)>interval)
      {
        sendSpike(current_level);
        previous_packet = current_packet; 
      }
    }
    else
    {
      sendSpike(current_level);
    }
  }   
  
//--------------Receive spikes-------------// 
  // read the output 7 pins from SpiNNaker Link
  L[0] = digitalRead(28);
  L[1] = digitalRead(27);
  L[2] = digitalRead(26);
  L[3] = digitalRead(25);
  L[4] = digitalRead(24);
  L[5] = digitalRead(23);
  L[6] = digitalRead(22);

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
  /*
  // plot received signal at 7 pins
  Serial.print(L[0]);
  Serial.print(",");
  Serial.print(L[1]);
  Serial.print(",");
  Serial.print(L[2]);
  Serial.print(",");
  Serial.print(L[3]);
  Serial.print(",");
  Serial.print(L[4]);
  Serial.print(",");
  Serial.print(L[5]);
  Serial.print(",");
  Serial.print(L[6]);
  Serial.print(",");
  */
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
  // compare current reading with previous ones
  bool showResult = compareTwo(L,PreviousValue);  

  if (showResult)  
  {
    // decode (2-to-7 decoding)  
    resTWO = decodeSeven(L,PreviousValue);
    if (resTWO==99)
    { 
      //detect spikes
      if(dec_data[1]==0)
      {
        if(first_spike)
        {
          ref_time = micros();
          first_spike = false;
        }
        unsigned long current_time = micros();  
        if(dec_data[2]>7)
        {
          dec_data[2] = dec_data[2] - 8;
        }
        com[Num_com] = 7 - dec_data[2];
        Num_com++;;
        
      }
      
      decodePackage(dec_data);
      inde = 0;
      cycle++;
      if(dec_data[1]==0)
      {
        for (int i=0; i<10; i++)
        {     
          dec_data[i] = 0;
        }
      }
      else
      {
        for (int i=0; i<18; i++)
        {     
          dec_data[i] = 0;
        }       
      }
    }
    else
    {
      dec_data[inde] = resTWO;
      inde++;
    }    

    // Send ACK to SpiNNaker 
    if (ACK)
    {
      digitalWrite(29,LOW);
    }
    else
    {
      digitalWrite(29,HIGH);
    } 
    ACK = !ACK;
    
    // update previous data
    for (int i=0; i<=6; i++)
    {
      PreviousValue[i] = L[i];
    }  
  }

  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
  // plot ACK and result
  if (showResult) {
    Serial.print(ACK);
    Serial.print(" -> ");
    Serial.println(resTWO);
  }
  else {
    Serial.println(ACK);
  }
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

  
  if(Num_com==10)
  {
    Num_com = 0;
  } 
}

/////////////////////////////////////////////////////////
//-------------------- Functions ----------------------//
/////////////////////////////////////////////////////////

/* Find the Number Appears Most Frequently in vet[] */
int mostFrequent(int vet[], int n)
{
  int count = 0;
  int most_fre = 0;
  int res = 0;
   for(int i=0; i<8; i++)
   {
      for(int k=0; k<n; k++)
      {
        if(vet[k]==i)
        {
          count++;
        }
      }
      if(count>=most_fre)
      {
        most_fre = count;
        res = i;
      }
      count = 0;
   }

  if(most_fre<=5)
  {
    res = ARMpos;
  }
   return res;
}

/* Send Spike to SpiNNaker */
void sendSpike(bool current_level)
{
    previous_level = !current_level;
    
    // obtain the 2 changing bits
    differentTwo(packets[Num_packet][ind_Num]);

    for(int i=0; i<2; i++)
    {
      int pin = ChangingBits[i];
      int ARD_pin = pin + 2;
        
      // change the state of arduino pins and store the state
      if(L_state[pin])
      {
        digitalWrite(ARD_pin,LOW);
      }
      else
      {
        digitalWrite(ARD_pin,HIGH);
      }
      L_state[pin] = !L_state[pin];
    }  
    if(ind_Num==10)
    {
      ind_Num = 0;  
      Num_packet++;
      if (Num_packet == Num_spikes) {
        Num_packet = 0; 
      }
    }
    else
    {
      ind_Num++;
    }
}

/* 2-to-7 Encoding */
void differentTwo(int Num_H)
{
  switch(Num_H)
  {
    case 0:
      ChangingBits[0] = 0;
      ChangingBits[1] = 4;
      break;
    case 1:
      ChangingBits[0] = 1;
      ChangingBits[1] = 4;
      break;
    case 2:
      ChangingBits[0] = 2;
      ChangingBits[1] = 4;
      break;
    case 3:
      ChangingBits[0] = 3;
      ChangingBits[1] = 4;
      break;
    case 4:
      ChangingBits[0] = 0;
      ChangingBits[1] = 5;
      break;
    case 5:
      ChangingBits[0] = 1;
      ChangingBits[1] = 5;
      break;
    case 6:
      ChangingBits[0] = 2;
      ChangingBits[1] = 5;
      break;
    case 7:
      ChangingBits[0] = 3;
      ChangingBits[1] = 5;
      break;
    case 8:
      ChangingBits[0] = 0;
      ChangingBits[1] = 6;
      break;
    case 9:
      ChangingBits[0] = 1;
      ChangingBits[1] = 6;
      break;
    case 10:
      ChangingBits[0] = 2;
      ChangingBits[1] = 6;
      break;
    case 11:
      ChangingBits[0] = 3;
      ChangingBits[1] = 6;
      break;
    case 12:
      ChangingBits[0] = 0;
      ChangingBits[1] = 1;
      break;
    case 13:
      ChangingBits[0] = 1;
      ChangingBits[1] = 2;
      break;
    case 14:
      ChangingBits[0] = 2;
      ChangingBits[1] = 3;
      break;
    case 15:
      ChangingBits[0] = 0;
      ChangingBits[1] = 3;
      break;
    case 99:
      ChangingBits[0] = 5;
      ChangingBits[1] = 6;
      break;
    default:
      Serial.print("\n\nError in 2-to-7 coding\n\n");
      break;
  }
}

/* Compare the different number of two binary array */
/*          if (num == 2) return true               */
/*          else          return false              */
bool compareTwo(int current[7], int previous[7])
{
  int Num_diff = 0;
  bool show = false;
  for (int i=0; i<=6; i++)
  {
    if (current[i] != previous[i])
    {
      Num_diff = Num_diff + 1;
    }
  }
  if (Num_diff == 2)
  {
    show = true;
  }
  else
  {
    show = false;
  }
  return show;
}

/* Decoding Package (Hex to Binary) */
void decodePackage(int dec_data[18])
{
  int package[72] = { 0 };  // a 72-bit package
  // package[i:i+3]
  for (int i=0; i<=17; i++)
  {
    switch(dec_data[i])
    {
      case 0:
        break;
      case 1:
        package[4*i] = 1;
        break;
      case 2:
        package[4*i+1] = 1;
        break;
      case 3:
        package[4*i] = 1;
        package[4*i+1] = 1;
        break;
      case 4:
        package[4*i+2] = 1;
        break;
      case 5:
        package[4*i+2] = 1;
        package[4*i] = 1;
        break;
      case 6:
        package[4*i+1] = 1;
        package[4*i+2] = 1;
        break;
      case 7:
        package[4*i+1] = 1;
        package[4*i+2] = 1;
        package[4*i] = 1;
        break;
      case 8:
        package[4*i+3] = 1;
        break;
      case 9:
        package[4*i] = 1;
        package[4*i+3] = 1;
        break;
      case 10:
        package[4*i+1] = 1;
        package[4*i+3] = 1;
        break;
      case 11:
        package[4*i] = 1;
        package[4*i+1] = 1;
        package[4*i+3] = 1;
        break;
      case 12:
        package[4*i+2] = 1;
        package[4*i+3] = 1;
        break;
      case 13:
        package[4*i] = 1;
        package[4*i+2] = 1;
        package[4*i+3] = 1;
        break;
      case 14:
        package[4*i+1] = 1;
        package[4*i+2] = 1;
        package[4*i+3] = 1;
        break;
      case 15:
        package[4*i] = 1;
        package[4*i+1] = 1;
        package[4*i+2] = 1;
        package[4*i+3] = 1;
        break;                                                
      default:
        Serial.println("Error in decoding package\n"); 
        break;
    }
  }
  
}

/* 2-to-7 Decoding */
int decodeSeven(int current[7], int previous[7])
{
  int result=77; //1010000
  int k=0;
  int arr[2]={9,9};
  //find the two changed bits
  for (int i=0; i<=6; i++)
  {
    if(current[i]!=previous[i])
    {
      arr[k] = i;
      k++;
    }
  }

  switch(arr[0])
  {
    case 0:
      if(arr[1]==1){result=12;}
      else if(arr[1]==3){result=15;}
      else if(arr[1]==4){result=0;}
      else if(arr[1]==6){result=8;}
      else if(arr[1]==5){result=4;}  
      else {
        Serial.println("Error in 2-to-7 decode");
      }
      break;
    case 1:
      if(arr[1]==2){result=13;}
      else if(arr[1]==4){result=1;}
      else if(arr[1]==5){result=5;}
      else if(arr[1]==6){result=9;}
      else {
        Serial.println("Error in 2-to-7 decode");
      }
      break;
    case 2:
      if(arr[1]==3){result=14;}
      else if(arr[1]==4){result=2;}
      else if(arr[1]==5){result=6;}
      else if(arr[1]==6){result=10;}
      else {
        Serial.println("Error in 2-to-7 decode");
      }
      break;
    case 3:
      if(arr[1]==4){result=3;}
      else if(arr[1]==5){result=7;}
      else if(arr[1]==6){result=11;}
      else {
        Serial.println("Error in 2-to-7 decode");
      }
      break;
    case 5:
      if(arr[1]==6){result=99;}
      else {
        Serial.println("Error in 2-to-7 decode");
      }
      break;
    default:
        Serial.println("Error in 2-to-7 decode_1");
        break;
  }    
  return result;
}
