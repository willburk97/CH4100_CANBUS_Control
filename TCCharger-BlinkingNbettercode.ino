//Code von Lennart O.

#include <mcp_can.h> 
#include <SPI.h>
#include <SimpleTimer.h>

#define SPI_CS_PIN 9 //CS Pin

int led = 7;
word outputvoltage = 1158; //set max Voltage to 116.2V (offset = 0.1) not pv_voltage if will turn off charger at 116 below
word outputcurrent = 300; //set max Current to 30A (offset = 0.1)
unsigned char voltbuf[30]; // buffer for binary-ish version of voltage.  i.e. 101011000 = 112v
unsigned int volth; // hundreds value
unsigned int voltt; //tens value
unsigned int volto; //ones value
unsigned int cyclecounter = 1; //tracks number of cycles since led started flashing i.e. 1010110 would take 7 cycles
unsigned long int sendId = 0x18E54024;    //0x18E54024 send to CH4100

uint8_t j=0;
uint8_t k;
unsigned char len = 0; //Length of received CAN message
unsigned char buf[8]; //Buffer for CAN message
unsigned long int receiveId; //ID of Charger
unsigned char errorct = 0; //tracks number of cycles chargers not charging. Charger starts at around 17
byte chargeron = 0xFB; //yes(on) = FC, no(off) = FB or seemingly anything else - start charger at FB(off)

MCP_CAN CAN(SPI_CS_PIN); //CS Pin for SPI

SimpleTimer timer1; //timer Object generation

//Functions

/************************************************
** Function name:           canRead
** Descriptions:            read CAN message
*************************************************/

void canRead(){

  if(CAN_MSGAVAIL == CAN.checkReceive()){ //Check for messages

    CAN.readMsgBuf(&len, buf); // read data, len: data length, buf: data buffer

    receiveId = CAN.getCanId(); //CAN-ID lesen

    if(receiveId == 0x18EB2440){ 
//      Serial.println("CAN Data received from Charger!");

      Serial.print("CAN ID: ");
      Serial.print(receiveId, HEX); //Output ID

      Serial.print(" / CAN Data: ");
      for(int i = 0; i<len; i++){ //Output Data

        if( buf[i] < 0x10){ // display zero/null if only one digit
          Serial.print("0");
        }
        Serial.print(buf[i],HEX);
        Serial.print(" ");          // Spaces

      }

      Serial.println(); //Prints Carriage Return
      if (bitRead(buf[1],0) == 1) Serial.print("Charger off ");
      if (bitRead(buf[1],0) == 0) Serial.print("Charger on  ");
      if (bitRead(buf[1],0) == 1) errorct++;
      Serial.println(errorct);
      if (chargeron == 0xFC & errorct > 28) chargeron = 0xFB; // if 'commanded' on but found off a couple times, force off to prevent cycling 17 after 1st on, ~21 after 2nd on
            
      Serial.print("Battery voltage: ");
      float pv_voltage = (((float)buf[3]*256.0) + ((float)buf[2]))/10.0; //highByte/lowByte + offset
      Serial.print(pv_voltage);
      Serial.println(" V");
      Serial.print("Charging Current: ");
      float pv_current = (3200-(((float)buf[5]*256.0) + ((float)buf[4])))/10.0; //highByte/lowByte + offset
      Serial.print(pv_current);
      Serial.println(" A");
      if (millis() > 10000 & millis() < 16000) chargeron = 0xFC; // wait 10 seconds before turning on charger
      if(pv_voltage > 116 || pv_voltage < 80) chargeron = 0xFB; // if voltage reaches this level turn off charger

      if(cyclecounter==1){
        volth = pv_voltage/100;                      //hundreds
        voltt = pv_voltage/10 - volth*10;            //tens
        volto = pv_voltage - volth*100 - voltt*10;   //ones

        for(k=0; k<20; k++) { //clear blinking buffer
          voltbuf[k] = 0;
        }

        k=0;
        while(k<volth) { //hundreds
          voltbuf[k] = 1;
          k++;
        }

        voltbuf[k] = 0;
        j = k;k=0;
        while(k<voltt) { //tens
          voltbuf[j+k+1] = 1;
          k++;
        }

        voltbuf[j+k+1] = 0;
        j = j+k+1;k=0;
        while(k<volto) { //ones
          voltbuf[j+k+1] = 1;
          k++;
        }

        voltbuf[j+k+1] = 0;
        j = j+k+1;

      }

      if(voltbuf[cyclecounter-1]==1) Blink();
      cyclecounter++;
      if(cyclecounter>j+5) cyclecounter = 1;


      switch (buf[0]) { //Read out error byte
        case B00000001: Serial.println("Error: CAN_BUS Error");break;
        case B00010000: Serial.println("Error: No Input Voltage");break;
      }

      switch (buf[1]) { //Read out error byte
        case B00000101: Serial.println("Error: Battery Not Connected");break;
      }

    }

  }

}

void Blink(){
  digitalWrite(led,HIGH);
  delay(150);
  digitalWrite(led,LOW);
}



/************************************************
** Function name:           canWrite
** Descriptions:            write CAN message
*************************************************/

String canWrite(unsigned char data[8], unsigned long int id){

  byte sndStat = CAN.sendMsgBuf(id, 1, 8, data); //Send message (ID, extended Frame, Datalength, Data)
Serial.print(data[0],HEX); Serial.print(" ");
  if(sndStat == CAN_OK) //Status byte for transmission
    return "CAN message sent successfully to charger";
//      return data;
  else
    return "Error during message transmission to charger";
}


/************************************************
** Function name:           myTimer1
** Descriptions:            function of timer1
*************************************************/

void myTimer1() { //Cyclic function called by the timer

  unsigned char voltamp[8] = {chargeron, lowByte(outputvoltage), highByte(outputvoltage), lowByte(3200-outputcurrent), highByte(3200-outputcurrent),0xFF,0xFF,0xFF};  //Generate the message
  Serial.println(canWrite(voltamp, sendId)); //Send message and output results
  canRead(); //Read output from charger
  
  Serial.println(); //Carriage Return

}

/************************************************
** Function name:           setup
** Descriptions:            Arduino setup
*************************************************/

void setup() {
  pinMode(led, OUTPUT);
  Serial.begin(115200); //Serial Port Initialize

  while(CAN_OK != CAN.begin(CAN_500KBPS)){ //CAN Bus initialisieren
    Serial.println("CAN Initialization Failure, Restart");
    delay(200);
  }

  Serial.println("CAN Initialization Successful");

  timer1.setInterval(950, myTimer1); //Define the time and function of the timer.

}

/************************************************
** Function name:           loop
** Descriptions:            Arduino loop
*************************************************/

void loop() {

  timer1.run(); //Timer start

}
