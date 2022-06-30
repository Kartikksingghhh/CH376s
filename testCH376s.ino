#include <SoftwareSerial.h>

byte computerByte;
byte USB_Byte;
int timeOut = 2000;
String wrData = "Hello Kartik";

SoftwareSerial USB(10, 11); 


void setup() 
{
  Serial.begin(9600);                                  
  USB.begin(9600);                                    
}

void loop() 
{
  if(Serial.available())
  {
    computerByte = Serial.read();                  
    
    if(computerByte==49)
    {                          
      checkConnection(0x01);                           
    } 
    
    if(computerByte==50)
    {               
      set_USB_Mode(0x06);                              
    }
    
    if(computerByte==51)
    {               
      resetALL();                                     
    }
    
    if(computerByte==52)
    {               
      writeData(wrData);                  
    }
    
    if(computerByte==53)
    {               
      readData();                           
    }
  }
}

void checkConnection(byte value)
{
  USB.write(0x57);
  USB.write(0xAB);
  USB.write(0x06);
  USB.write(value);
  
  if(waitForResponse("checking connection"))
  {       
    if(getResponseFromUSB()==(255-value))
    {
       Serial.println(">Connection to CH376S was successful.");
    } 
    else 
    {
      Serial.print(">Connection to CH376S - FAILED.");
    }
  }
}


void set_USB_Mode (byte value)
{
  USB.write(0x57);
  USB.write(0xAB);
  USB.write(0x15);
  USB.write(value);
  
  delay(20);
  
  if(USB.available())
  {
    USB_Byte=USB.read();
    if(USB_Byte==0x51)
    {                                  
        Serial.println("set_USB_Mode command acknowledged");  
        USB_Byte = USB.read();
        
        if(USB_Byte==0x15)
        {                              
          Serial.println("USB is present");                                 
        } 
        else 
        {
          Serial.print("USB Not present. Error code:");   
          Serial.print(USB_Byte, HEX);
          Serial.println("H");
        }
        
    } else {
        Serial.print("CH3765 error!   Error code:");
        Serial.print(USB_Byte, HEX);
        Serial.println("H");
    }   
  }
  delay(20);
}

void resetALL()
{
    USB.write(0x57);
    USB.write(0xAB);
    USB.write(0x05);
    Serial.println("The CH376S module has been reset !");
    delay(200);
}

void writeData(String data)
{
  resetALL();                     
  set_USB_Mode(0x06);             
  diskConnectionStatus();

  byte dataLength = (byte) data.length();        
  Serial.println(data);
  Serial.print("Data Length:");
  Serial.println(dataLength);
  delay(100);
  
  USB.write(0x57);
  USB.write(0xAB);
  USB.write(0x3C);
  USB.write((byte) dataLength);
  USB.write((byte) 0x00);
  if(waitForResponse("setting data Length"))
  {     
    if(getResponseFromUSB()==0x1E)
    {                
      USB.write(0x57);
      USB.write(0xAB);
      USB.write(0x2D);
      USB.print(data);                             
  
      if(waitForResponse("writing data to file"))
      {   
      }
      Serial.print("Write code (normally FF and 14): ");
      Serial.print(USB.read(),HEX);                
      Serial.print(",");
      USB.write(0x57);
      USB.write(0xAB);
      USB.write(0x3D);                             
      if(waitForResponse("updating file size"))
      {   
      }
      Serial.println(USB.read(),HEX);              
    }
  }
}

void readData()
{
  resetALL();                     
  set_USB_Mode(0x06);             
  diskConnectionStatus();

  byte firstByte = 0x00;                     
  byte numBytes = 0x40;                     
 
  while(setByteRead(numBytes))
  {               
    USB.write(0x57);
    USB.write(0xAB);
    USB.write(0x27);                          
    if(waitForResponse("reading data"))
    {     
        firstByte=USB.read();                 
        while(USB.available())
        {
          Serial.write(USB.read());            
          delay(1);                           
        }
    }
    if(!continueRead())
    {                      
      break;                                   
    }
  }
  Serial.println();
  Serial.println("NO MORE DATA");
}


boolean setByteRead(byte numBytes)
{
  boolean bytesToRead=false;
  int timeCounter = 0;
  USB.write(0x57);
  USB.write(0xAB);
  USB.write(0x3A);
  USB.write((byte)numBytes);   
  USB.write((byte)0x00);
  if(waitForResponse("setByteRead"))
  {        
    if(getResponseFromUSB()==0x1D)
    {        
      bytesToRead=true;
    }
  }
  return(bytesToRead);
}


boolean continueRead()
{
  boolean readAgain = false;
  USB.write(0x57);
  USB.write(0xAB);
  USB.write(0x3B);
  if(waitForResponse("continueRead"))
  {        
     if(getResponseFromUSB()==0x14)
     {         
       readAgain=true;
     }
  }
  return(readAgain);
}

void diskConnectionStatus()
{
  Serial.println("Checking USB disk connection status");
  USB.write(0x57);
  USB.write(0xAB);
  USB.write(0x30);

  if(waitForResponse("Connecting to USB disk"))
  {       
    if(getResponseFromUSB()==0x14)
    {               
       Serial.println(">Connection to USB OK");
    } 
    else 
    {
      Serial.print(">Connection to USB - FAILED.");
    }
  }
}

boolean waitForResponse(String errorMsg)
{
  boolean bytesAvailable = true;
  int counter=0;
  while(!USB.available())
  {     
    delay(1);
    counter++;
    if(counter>timeOut)
    {
      Serial.print("TimeOut waiting for response: Error while: ");
      Serial.println(errorMsg);
      bytesAvailable = false;
      break;
    }
  }
  delay(1);
  return(bytesAvailable);
}

byte getResponseFromUSB()
{
  byte response = byte(0x00);
  if (USB.available())
  {
    response = USB.read();
  }
  return(response);
}
