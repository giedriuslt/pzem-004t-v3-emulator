#define CMD_LEN 8
#define VAL_LEN 20
#define CMD_DELAY 0 // You can set to 0 if you don't want to wait. Response delay emulation.
#define PzemSerial Serial1 //Serial to talk to module, Set it to correct serial for you device
#define DebugSerial Serial //Serial to send debug info, Set it to correct serial for you device

uint8_t myAddr = 1;
long startTime = 0;
uint8_t command[CMD_LEN];
uint8_t values[VAL_LEN] = {0x08, 0x98, //220.0V
      0x0A, 0xA7, 0x00, 0x00, //2.727A
      0x17, 0x70, 0x00, 0x00, //600.0W
      0x00, 0x00, 0x00, 0x00, //0Wh
      0x01, 0xF4, //50.0Hz
      0x00, 0x50, //0.80pf
      0x00, 0x00  //0 no alarm
      };

void setup() 
{
  PzemSerial.begin(9600);
  DebugSerial.begin(115200);
}

// Pre computed CRC table
static const uint16_t crcTable[] PROGMEM = {
    0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
    0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
    0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
    0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
    0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
    0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
    0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
    0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
    0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
    0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
    0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
    0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
    0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
    0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
    0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
    0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
    0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
    0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
    0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
    0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
    0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
    0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
    0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
    0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
    0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
    0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
    0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
    0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
    0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
    0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
    0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
    0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040
};

/*!
 * Calculate the CRC16-Modbus for a buffer
 * Based on https://www.modbustools.com/modbus_crc16.html
 *
 * @param[in] data Memory buffer containing the data to checksum
 * @param[in] len  Length of the respBuffer
 *
 * @return Calculated CRC
*/
uint16_t CRC16(const uint8_t *data, uint16_t len)
{
  uint8_t nTemp; // CRC table index
  uint16_t crc = 0xFFFF; // Default value

  while (len--)
  {
    nTemp = *data++ ^ crc;
    crc >>= 8;
    crc ^= (uint16_t)pgm_read_word(&crcTable[nTemp]);
  }
  return crc;
}

void setCRC(uint8_t *buf, uint16_t len){
  if(len <= 2) // Sanity check
    return;

  uint16_t crc = CRC16(buf, len - 2); // CRC of data

  // Write high and low byte to last two positions
  buf[len - 2] = crc & 0xFF; // Low byte first
  buf[len - 1] = (crc >> 8) & 0xFF; // High byte second
}

void printData(uint8_t *buf, uint16_t length)
{
  for (int i=0; i<length; i++)
  {
    if (buf[i] < 16) {
      DebugSerial.print("0");
    }
    DebugSerial.print(buf[i], HEX);
    DebugSerial.print(' ');
  }
  if (length>0)
  {
    DebugSerial.println("");
  }
}

void respData(uint8_t cmd, uint16_t regfrom, uint16_t regcount)
{
  const uint8_t resp_size = 32;
  uint8_t data[resp_size];
  
  uint8_t send_size = 0;
  data[send_size++] = myAddr;
  data[send_size++] = cmd;
  data[send_size++] = regcount*2;

  values[1]+=random(0,11)-5; //randomize voltage by +-0.5V
  values[3]+=random(0,11)-5; //randomize current by +-5mA
  values[7]+=random(0,11)-5; //randomize power by +-0.5W
  
  long currentTime = millis();
  long diff = currentTime - startTime;
  if (diff > 6000) //increase energy by 1Wh every 6 seconds, simulating 600W load
  {
    uint8_t diffWh = diff/6000;
    startTime += diffWh*6000;

    values[11]+=diffWh;
    if (values[11] < diffWh)
    {
      values[10]+=1;
    }    
  }

  values[15]+=random(0,3)-1; //randomize frequency 50Hz +-0.1Hz
  if (values[15]>0xF5 || values[15]<0xF3)
  {
    values[15] = 0xF4;    
  }
  values[17]+=random(0,5)-2; //randomize power factor between [0.75; 0.85]
  if (values[17]>0x55 || values[17]<0x4B)
  {
    values[17] = 0x50;    
  }


  for (int i=regfrom*2; i<regfrom*2+regcount*2;i++)
  {
    data[send_size++] = values[i];    
  }

  send_size+=2;
  setCRC(data, send_size);

  delay(CMD_DELAY);
  PzemSerial.write(data, send_size);
  PzemSerial.flush();
  printData(data, send_size);
}


void respAddress(uint8_t cmd, uint16_t regfrom, uint16_t regcount)
{
  const uint8_t resp_size = 32;
  uint8_t data[resp_size];
  
  uint8_t send_size = 0;
  data[send_size++] = myAddr;
  data[send_size++] = cmd;
  data[send_size++] = regcount*2;
  data[send_size++] = myAddr<<8;
  data[send_size++] = myAddr;  

  send_size+=2;
  setCRC(data, send_size);

  delay(CMD_DELAY);
  PzemSerial.write(data, send_size);
  PzemSerial.flush();
  printData(data, send_size);
}

void respReset(uint8_t command)
{
  uint8_t send_size = 4;
  values[11] = 0; //reset energy to 0
  values[10] = 0;
  uint8_t data[4];
  data[0] = myAddr;
  data[1] = 0x42;
  setCRC(data, 4);
  delay(CMD_DELAY);
  PzemSerial.write(data,send_size);
  PzemSerial.flush();
  printData(data, send_size);  
}

void respError(uint8_t illegalCmd)
{
  const int send_size = 5;
  uint8_t data[send_size];
  data[0] = myAddr;
  data[1] = 0x84;
  data[2] = illegalCmd;
  setCRC(data, send_size);
  delay(CMD_DELAY);
  PzemSerial.write(data,send_size);
  PzemSerial.flush();
  //printData(data, sizeof(data));  
}

void process()
{
  uint8_t data[CMD_LEN];
  uint8_t i = 0;

  delay(20); //wait for all data
  while (PzemSerial.available() && i<8)
  { 
    uint8_t incomingByte = PzemSerial.read();
    data[i++] = incomingByte;
    if (i==4 && data[1] == 0x42)
    {
      break; // found reset command
    }
  }
  while (PzemSerial.available())
  {  //discard if anything remains
    PzemSerial.read();
  }     
    
  if(i > 2 && ((uint16_t)data[i-1]<<8)+data[i-2] == CRC16(data, i-2) )
  {
    DebugSerial.print("Command: ");
    printData(data, i);    
    switch (data[1]) 
    {
      case 0x04: 
        DebugSerial.print("Sending data: ");        
        respData(data[1], (uint16_t)data[2]<<8 | data[3],  (uint16_t)data[4]<<8 | data[5]);
        break; 
      case 0x03: 
        DebugSerial.print("Sending adress: ");        
        respAddress(data[1], (uint16_t)data[2]<<8 | data[3],  (uint16_t)data[4]<<8 | data[5]);
        break; 
      case 0x42: 
        Serial.print("Reseting energy: ");        
        respReset(data[1]);
        break; 
      default:
        DebugSerial.println("Sending error: ");
        respError(data[1]);
        while (PzemSerial.available())
        { //discard if anything remains
          PzemSerial.read();
        }
        break; 
    }
  }
  else
  {
    DebugSerial.print("Bad command: ");
    printData(data, i); 
    DebugSerial.print("Bad crc:");
    if (i>2)
    {
      DebugSerial.print(" expected: ");
      DebugSerial.println(CRC16(data,i-2), HEX);
      DebugSerial.print(" got:");
      DebugSerial.println( ((uint16_t)data[i-1]<<8)+data[i-2] , HEX);
    }
    
    while (PzemSerial.available())
    {
      //discard if anything remains
      PzemSerial.read();
    }     
  }
}
void loop()
{
  if(PzemSerial.available()) 
  {
    DebugSerial.println("Trying to process command");
    process();
  }
  delay(50);
}