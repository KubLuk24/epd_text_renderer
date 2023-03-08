#include <SPI.h>
#define RST_PIN         8
#define DC_PIN          9
#define CS_PIN          10
#define BUSY_PIN        7
#define EPD_WIDTH       128
#define EPD_HEIGHT      296

#define UWORD  unsigned int
#define UBYTE  unsigned char

enum pos {top, bottom};
int reset_pin = RST_PIN;
int dc_pin = DC_PIN;
int cs_pin = CS_PIN;
int busy_pin = BUSY_PIN;
int width = EPD_WIDTH / 8;
int height = EPD_HEIGHT;

const unsigned char A[] = { 0x01, 0xEE, 0xEE, 0x01 };
const unsigned char B[] = { 0x00, 0x66, 0x66, 0x99 };
const unsigned char C[] = { 0x81, 0x7E, 0x7E, 0x3C };
const unsigned char D[] = { 0x00, 0x7E, 0x7E, 0x81 };
const unsigned char E[] = { 0x00, 0x76, 0x76, 0x76 };
const unsigned char F[] = { 0x00, 0xF6, 0xF6, 0xF6 };
const unsigned char G[] = { 0x81, 0x7E, 0x76, 0x8E };
const unsigned char H[] = { 0x00, 0xE7, 0xE7, 0x00 };
const unsigned char I[] = { 0x7E, 0x00, 0x7E, 0xFF };
const unsigned char J[] = { 0x8F, 0x7E, 0x7E, 0x80 };
const unsigned char K[] = { 0x00, 0xF7, 0xF7, 0x08 };
const unsigned char L[] = { 0x00, 0x7F, 0x7F, 0x7F };
const unsigned char M[] = { 0x00, 0xFD, 0xFD, 0x00 };
const unsigned char N[] = { 0x00, 0xF3, 0xCF, 0x00 };
const unsigned char O[] = { 0x81, 0x7E, 0x7E, 0x81 };
const unsigned char P[] = { 0x00, 0xF6, 0xF6, 0xF9 };
const unsigned char Q[] = { 0x81, 0x7E, 0x3E, 0x81 };
const unsigned char R[] = { 0x00, 0xF6, 0xF6, 0x09 };
const unsigned char S[] = { 0x79, 0x76, 0x76, 0x89 };
const unsigned char T[] = { 0xFE, 0x00, 0xFE, 0xFF };
const unsigned char U[] = { 0x80, 0x7F, 0x7F, 0x80 };
const unsigned char V[] = { 0xF0, 0x0F, 0x0F, 0xF0 };
const unsigned char X[] = { 0x18, 0xE7, 0xE7, 0x18 };
const unsigned char Y[] = { 0xFC, 0x03, 0x03, 0xFC };
const unsigned char Z[] = { 0x1E, 0x6E, 0x76, 0x78 };
const unsigned char* const letters[] = { A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, X, Y, Z };

int skip = 0;

void DigitalWrite(int pin, int value) {
    digitalWrite(pin, value);
}

void SendData(unsigned char data) {
    DigitalWrite(dc_pin, HIGH);
    SpiTransfer(data);
}

void SendCommand(unsigned char command) {
    DigitalWrite(dc_pin, LOW);
    SpiTransfer(command);
}

void DisplayFrame(const UBYTE *blackimage, const UBYTE *ryimage) {
    SendCommand(0x10);
    for (UWORD j = 0; j < height; j++) {
        for (UWORD i = 0; i < width; i++) {
          SendData(pgm_read_byte(&blackimage[i + (j*width)]));
        }
    }
    SendCommand(0x92);
    
    SendCommand(0x13);
    for (UWORD j = 0; j < height; j++) {
        for (UWORD i = 0; i < width; i++) {
          SendData(pgm_read_byte(&ryimage[i + (j*width)]));
        }
    }
    SendCommand(0x92);

    SendCommand(0x12);
    WaitUntilIdle();
}

void SpiTransfer(unsigned char data) {
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(data);
    digitalWrite(CS_PIN, HIGH);
}



void WaitUntilIdle(void) {
  unsigned char busy;
  Serial.print("e-Paper busy \r\n ");
  do
  {
    SendCommand(0x71);
    busy = digitalRead(busy_pin);
    busy =!(busy & 0x01);        
  }
  while(busy); 
  Serial.print("e-Paper busy release \r\n ");
  delay(200);
}

int IfInit(void) {
    pinMode(CS_PIN, OUTPUT);
    pinMode(RST_PIN, OUTPUT);
    pinMode(DC_PIN, OUTPUT);
    pinMode(BUSY_PIN, INPUT); 
    SPI.begin();
    SPI.beginTransaction(SPISettings(7000000, MSBFIRST, SPI_MODE0));
    return 0;
}

int Init(void) {
    if (IfInit() != 0) {
        return -1;
    }
    Reset();

    SendCommand(0x04);//power on
    WaitUntilIdle();//waiting for the electronic paper IC to release the idle signal

    SendCommand(0x00);//panel setting
    SendData(0x0f);//default data
    SendData(0x89);//128x296,Temperature sensor, boost and other related timing settings

    SendCommand(0x61);//Display resolution setting
    SendData (0x80);
    SendData (0x01);
    SendData (0x28);

    SendCommand(0X50);//VCOM AND DATA INTERVAL SETTING      
    SendData(0x77);//WBmode:VBDF 17|D7 VBDW 97 VBDB 57   
                            //WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7
    return 0;
}

void Reset(void) {
    DigitalWrite(reset_pin, HIGH);
    delay(200);   
    DigitalWrite(reset_pin, LOW);                //module reset    
    delay(5);
    DigitalWrite(reset_pin, HIGH);
    delay(200);    
}
void Clear(void) {
    SendCommand(0x10);
    for (UWORD j = 0; j < height; j++) {
        for (UWORD i = 0; i < width; i++) {
            SendData(0xff);
        }
    }

    SendCommand(0x13);
    for (UWORD j = 0; j < height; j++) {
        for (UWORD i = 0; i < width; i++) {
            SendData(0xff);
        }
    }
    
    SendCommand(0x12);
    WaitUntilIdle();

}

void renderBlack(const unsigned char* character, pos position, int left, int curlimit){
  int col = 0;

  if (position == top) {
    int line = 0;
    for (int i = 0; i < (width * height) - skip; i++) {
      if (line == 15 - curlimit) {
        SendData(col > sizeof(character) + 1 ? 0xFF : character[col]);
        col++;
        line = 0;
      } else {
        SendData(0xFF);
        line++;
      }

      if (left != 0 && col > sizeof(character) + 1) {
      for (int x = 0; x < width; x++) { SendData(0xFF); }
      skip = i + 1;
      break;
      }
    }
  }
} 

char* mixString(char* str) {
  int len = strlen(str);
  char* mixedStr = new char[len + 1];

  int numLines = (len % 26 == 0) ? (len / 26) : (len / 26 + 1);

  for (int line = 0; line < numLines; line++) {
    for (int i = 0; i < 26; i++) {

      int index = line + i * numLines;
      if (index >= len) {
        break;
      }

      int mixedIndex = (i % 2) * numLines + (i / 2);
      mixedStr[mixedIndex] = str[index];
    }
  }

  mixedStr[len] = '\0';
  return mixedStr;
}

void extractString(char* str, char* extractedStr) {
  char* openingPos = strchr(str, '¤');
  if (openingPos == NULL) {
    return;
  }

  char* closingPos = strchr(openingPos + 1, '¤');
  if (closingPos == NULL) {
    return;
  }

  int len = closingPos - openingPos - 1;
  strncpy(extractedStr, openingPos + 1, len);
  extractedStr[len] = '\0';
}


void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.print("2.9inch b V3 e-Paper init \r\n ");
  if (Init() != 0) {
      Serial.print("e-Paper init failed\r\n ");
      return;
  }

  Clear();

  SendCommand(0x10);
  char str[] = "Lorem ipsum dolor sit amet";
  
  for (int i = 0; i < sizeof(str); i++) {
    str[i] = toupper(str[i]);
  }

  int totallen = 0;
  int lines = 0;

  for (int i = 0; i < sizeof(str)-1; i++) {
    char c = str[i];
    if (c == ' ') totallen += 3;
    else if (c >= 'A' && c <= 'Z') totallen +=4;
  }

  if (totallen > height) {
    while (totallen >= 0) {
      lines++;
      totallen -= height;
    }

  }

  
  int left = sizeof(str);

    for (int i = 0; i < sizeof(str)-1; i++) {
        char c = str[i];
        if (c == ' ') {
        for (int x = 0; x < width * 3; x++) { SendData(0xFF); }
        } else if (c == '¤') {
          lines++;
        } else if (c >= 'A' && c <= 'Z') {
          Serial.println(c);
          left--;
          renderBlack(letters[c-'A'], top, left, lines);
        } else { /* debug handling can go here, not necessary. */}
    }

SendCommand(0x13);
for (UWORD j = 0; j < height; j++) {
  for (UWORD i = 0; i < width; i++) {
    SendData(0xFF);
  }
}

  SendCommand(0x92);

  SendCommand(0x12);
  WaitUntilIdle();
  skip = 0;
}

void loop() {
}
