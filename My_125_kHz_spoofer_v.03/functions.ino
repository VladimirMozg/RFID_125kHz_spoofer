void WriteHeader(void)
{
  // a header consists of 9 one bits
  RFIDdata[datapointer++]=1; 
  RFIDdata[datapointer++]=1;  
  RFIDdata[datapointer++]=1;  
  RFIDdata[datapointer++]=1; 
  RFIDdata[datapointer++]=1; 
  RFIDdata[datapointer++]=1; 
  RFIDdata[datapointer++]=1; 
  RFIDdata[datapointer++]=1; 
  RFIDdata[datapointer++]=1; 
}


void WriteData(byte nibble)
{
  byte data;
  byte rowsum=0;
  for (int i=4; i>0; i--)
  {
    if ((nibble& 1<<i-1) ==0)  
    {
      data=0; 
    }
    else 
    {
      data=1;
      rowsum++;  // increment the checksum value
      colsum[i-1]++; // increment the column checksum
    }


    RFIDdata[datapointer++]= data;
    #ifdef SERIALDEBUG 
      Serial.print((int) data); 
    #endif
     
  }
  // write the row checksum out
  if ((rowsum%2)==0)  
  {
    RFIDdata[datapointer++]=0; 
    #ifdef SERIALDEBUG 
      Serial.print((int)0); 
    #endif
    
  }
  else
  {  
    RFIDdata[datapointer++]=1; 
    #ifdef SERIALDEBUG 
      Serial.print((int)1); 
    #endif
  }

    #ifdef SERIALDEBUG 
      Serial.println(); 
    #endif

}


void WriteChecksum(void)
{
  byte data;
  byte rowsum=0;
  for (int i=4; i>0; i--)
  {
    if ((colsum[i-1]%2) ==0)  
    {
      RFIDdata[datapointer++]=0; 
     #ifdef SERIALDEBUG 
      Serial.print((int)0); 
     #endif
    }
    else
    {
      RFIDdata[datapointer++]=1; 
      #ifdef SERIALDEBUG 
      Serial.print((int) 1); 
      #endif
    }  
  }

  // write the stop bit
  RFIDdata[datapointer++]=0; 

      #ifdef SERIALDEBUG 
      Serial.print((int)0); 
      #endif

}





void BuildCard(void)
{
  // load up the RFID array with card data
  // intitalise the write pointer
  datapointer=0;

  WriteHeader();
  // Write facility
  WriteData(facility[0]);
  WriteData(facility[1]);
 
  // Write cardID
  WriteData(cardID[0]);
  WriteData(cardID[1]);
  WriteData(cardID[2]);
  WriteData(cardID[3]);
  WriteData(cardID[4]);  
  WriteData(cardID[5]);
  WriteData(cardID[6]);  
  WriteData(cardID[7]);

  WriteChecksum();
}


void TransmitManchester(int cycle, int data)
{

  if(cycle ^ data == 1)
  {
    digitalWrite(COIL, HIGH);
  }
  else
  {
    digitalWrite(COIL, LOW);  
  }
}

void EmulateCard(void)
{
  #ifdef SERIALDEBUG 
  Serial.println("Emulate Card Entered"); 
  #endif  // enter a low power modewritedataLEDS(0);  // turn off the LEDs
  
  BuildCard();
  
  #ifdef SERIALDEBUG 
  Serial.println(); 
  for(int i = 0; i < 64; i++)
  {
    if (RFIDdata[i]==1) Serial.print("1"); 
    else if (RFIDdata[i]==0) Serial.print("0"); 
    else Serial.print((int)RFIDdata[i]); 
  } 
  Serial.println(); 
  #endif  
  

  while (1)
  {
    for(int i = 0; i < 64; i++)
    {
      TransmitManchester(0, RFIDdata[i]);
      delayMicroseconds(bittime);
      TransmitManchester(1, RFIDdata[i]);
      delayMicroseconds(bittime); 
    } 
    //if (keypad.getKey()) break;
  }
}



   
