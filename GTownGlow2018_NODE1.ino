/* NODE 1 is the "leader" of the group of figures. 
 * 
 * Code for Georgetown Glow 2018 outdoor light sculpture, "Friends."  
 * Using 5 NeoPixel strands to form a gesture drawing figure. 
 * Borrowed heavily from https://learn.adafruit.com/multi-tasking-the-arduino-part-3/put-it-all-together-dot-dot-dot
 * Also used some parts of @GreenMoonArt icicle code at https://gist.github.com/GreenMoonArt/9e850706f56dbaa505eeecf89679c60b
 * 
 * 915 MHz RF info: https://learn.sparkfun.com/tutorials/rfm69hcw-hookup-guide
 * RFM96 Library: https://github.com/LowPowerLab/RFM69
 *
 * There are three figures in the piece. The below code is for the first figure -- Node 1. It determines a color, draws itself, and transmits the color to Node 2. 
 * If Node 2 is listening and receive the color, it will do likewise. Node 2 may be in the middle of a routine and may not "hear" the transmission. 
 * Node 2 will transmit to Node 3. Just like friends, they will get together (in this case, share the aame color) when the can. But they also do their own things. 
 */

// Pattern types supported:
enum  pattern { NONE, RAINBOW_CYCLE, SCANNER, FADE, OFF };   // THEATER_CHASE, COLOR_WIPE, 
// Patern directions supported:
enum  direction { FORWARD, REVERSE };

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Using Arduino Mega
#define PIN0 32  // body
#define PIN1 42   // head

#define PIN2 38   // right arm - middle
#define PIN2up 36   // right arm - waving, top
#define PIN2down 40   // right arm - waving, bottom

#define PIN3 34   // left arm
#define PIN4 44   // right leg
#define PIN5 46   // left leg

#define HeadLEDs 90         //90
#define BodyLEDs 55         //55
#define RightArmLEDs 73     //73
#define RightArmLEDsUp 36   //36
#define RightArmLEDsDown 36 //36
#define LeftArmLEDs 74      //74
#define RightLegLEDs 82     //82
#define leftLegLEDs 82      //82

const int numberOfStrips = 8;

// initialize strips using an array
// if you use a number of strips differing from 5, then the array needs to be adjusted accordingly
Adafruit_NeoPixel strip[numberOfStrips] = 
{
  Adafruit_NeoPixel(HeadLEDs, PIN1, NEO_GRB + NEO_KHZ800),

  Adafruit_NeoPixel(RightArmLEDsUp, PIN2up, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(RightArmLEDs, PIN2, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(RightArmLEDsDown, PIN2down, NEO_GRB + NEO_KHZ800),

  Adafruit_NeoPixel(LeftArmLEDs, PIN3, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(RightLegLEDs, PIN4, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(leftLegLEDs, PIN5, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(BodyLEDs, PIN0, NEO_GRB + NEO_KHZ800)
  };


// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
    public:

    // Member Variables:  
    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern

    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position

    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern

    void (*OnComplete)();  // Callback on completion of pattern

    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
    :Adafruit_NeoPixel(pixels, pin, type)
    {
        OnComplete = callback;
    }

    // Update the pattern
    void Update()
    {
        if((millis() - lastUpdate) > Interval) // time to update
        {
            lastUpdate = millis();
            switch(ActivePattern)
            {
                case RAINBOW_CYCLE:
                    RainbowCycleUpdate();
                    break;
                //case THEATER_CHASE:
                //    TheaterChaseUpdate();
                //    break;
                //case COLOR_WIPE:
                //    ColorWipeUpdate();
                //    break;
                case SCANNER:
                    ScannerUpdate();
                    break;
                case FADE:
                    FadeUpdate();
                    break;
                case OFF:
                    TurnLEDsOffUpdate();
                default:
                    break;
            }
        }
    }

    // Increment the Index and reset at the end
    void Increment()
    {
        if (Direction == FORWARD)
        {

           Index++;
           if (Index >= TotalSteps)
            {
                Index = 0;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the completion callback
                }
            }
        }
        else // Direction == REVERSE
        {
            --Index;
            if (Index <= 0)
            {
                Index = TotalSteps-1;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
    }

    // Reverse pattern direction
    void Reverse()
    {
        if (Direction == FORWARD)
        {
            Direction = REVERSE;
            Index = TotalSteps-1;
        }
        else
        {
            Direction = FORWARD;
            Index = 0;
        }
    }

    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = RAINBOW_CYCLE;
        Interval = interval;
        TotalSteps = 255;
        Index = 0;
        Direction = dir;
    }

    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
        }
        show();
        Increment();
    }


    // Initialize for a SCANNNER
    void Scanner(uint32_t color1, uint8_t interval)
    {
        ActivePattern = SCANNER;
        Interval = interval;
        TotalSteps = (numPixels() - 1) * 2;
        Color1 = color1;
        Index = 0;
    }
 
    // Update the Scanner Pattern
    void ScannerUpdate()
    { 
        for (int i = 0; i < numPixels(); i++)
        {
            if (i == Index)  // Scan Pixel to the right
            {
                 setPixelColor(i, Color1);
            }
            else if (i == TotalSteps - Index) // Scan Pixel to the left
            {
                 setPixelColor(i, Color1);
            }
            else // Fading tail
            {
                 setPixelColor(i, DimColor(getPixelColor(i)));
            }
        }
        show();
        Increment();
    }
    
    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = FADE;
        Interval = interval;
        TotalSteps = steps;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Fade Pattern
    void FadeUpdate()
    {
        // Calculate linear interpolation between Color1 and Color2
        // Optimise order of operations to minimize truncation error
        uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
        uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
        uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;
        
        ColorSet(Color(red, green, blue));
        show();
        Increment();
    }


    // Turn off all NeoPixel LEDs
    void TurnLEDsOff()
    {
      ActivePattern = OFF;
    }
    
    void TurnLEDsOffUpdate()
    {

      ColorSet(Color(0, 0, 0));

      setPixelColor(Index, 0, 0, 0); 
      show();

      Increment();
    }


   
    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    uint32_t DimColor(uint32_t color)
    {
        // Shift R, G and B components one bit to the right
        uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
        return dimColor;
    }
 
    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
        for (int i = 0; i < numPixels(); i++)
        {
            setPixelColor(i, color);
        }
        show();
    }

    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
        return (color >> 16) & 0xFF;
    }

    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
        return (color >> 8) & 0xFF;
    }

    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
        return color & 0xFF;
    }

    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
        WheelPos = 255 - WheelPos;
        if(WheelPos < 85)
        {
            return Color(255 - WheelPos * 3, 0, WheelPos * 3);
        }
        else if(WheelPos < 170)
        {
            WheelPos -= 85;
            return Color(0, WheelPos * 3, 255 - WheelPos * 3);
        }
        else
        {
            WheelPos -= 170;
            return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
        }
    }
};


void Strand1Complete();
void Strand2Complete();
void Strand3Complete();
void Strand3upComplete();
void Strand3downComplete();
void Strand4Complete();
void Strand5Complete();
void Strand6Complete();

// Define some NeoPatterns for the two rings and the Strand3
//  as well as some completion routines
NeoPatterns Strand1(RightLegLEDs, PIN4, NEO_GRB + NEO_KHZ800, &Strand1Complete);  //  - 20
NeoPatterns Strand2(LeftArmLEDs, PIN3, NEO_GRB + NEO_KHZ800, &Strand2Complete);

NeoPatterns Strand3(RightArmLEDs, PIN2, NEO_GRB + NEO_KHZ800, &Strand3Complete);
NeoPatterns Strand3up(RightArmLEDsUp, PIN2up, NEO_GRB + NEO_KHZ800, &Strand3upComplete);
NeoPatterns Strand3down(RightArmLEDsDown, PIN2down, NEO_GRB + NEO_KHZ800, &Strand3downComplete);

NeoPatterns Strand4(HeadLEDs, PIN1, NEO_GRB + NEO_KHZ800, &Strand4Complete);      //  - 50
NeoPatterns Strand5(leftLegLEDs, PIN5, NEO_GRB + NEO_KHZ800, &Strand5Complete); 

NeoPatterns Strand6(BodyLEDs, PIN0, NEO_GRB + NEO_KHZ800, &Strand6Complete); 




// Send serial input characters from one RFM69 node to another
// Based on RFM69 library sample code by Felix Rusu
// http://LowPowerLab.com/contact
// Modified for RFM69HCW by Mike Grusin, 4/16

// This sketch will show you the basics of using an
// RFM69HCW radio module. SparkFun's part numbers are:
// 915MHz: https://www.sparkfun.com/products/12775
// 434MHz: https://www.sparkfun.com/products/12823

// See the hook-up guide for wiring instructions:
// https://learn.sparkfun.com/tutorials/rfm69hcw-hookup-guide

// Uses the RFM69 library by Felix Rusu, LowPowerLab.com
// Original library: https://www.github.com/lowpowerlab/rfm69
// SparkFun repository: https://github.com/sparkfun/RFM69HCW_Breakout

// Include the RFM69 and SPI libraries:

#include <RFM69.h>
#include <SPI.h>

// Addresses for this node. CHANGE THESE FOR EACH NODE!

#define NETWORKID     0   // Must be the same for all nodes (0 to 255)
#define MYNODEID      1   // My node ID (0 to 255)
#define TONODEID      2   // Destination node ID (0 to 254, 255 = broadcast)

// RFM69 frequency, uncomment the frequency of your module:

//#define FREQUENCY   RF69_433MHZ
#define FREQUENCY     RF69_915MHZ

// AES encryption (or not):

#define ENCRYPT       true // Set to "true" to use encryption
#define ENCRYPTKEY    "RAINBOWGLOWARDUI" // Use the same 16-byte key on all nodes - if using this code, change the key to your own

// Use ACKnowledge when sending messages (or not):
#define USEACK        false // Request ACKs or not

// Packet sent/received indicator LED (optional):
#define LED           9 // LED positive pin

// Create a library object for our RFM69HCW module:
RFM69 radio;

void setup()
{
  // Open a serial port so we can send keystrokes to the module:

  Serial.begin(9600);
  Serial.print("Node ");
  Serial.print(MYNODEID, DEC);
  Serial.println(" ready");  

  // Set up the indicator LED (optional):
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  // Initialize the RFM69HCW:
  radio.initialize(FREQUENCY, MYNODEID, NETWORKID);
  radio.setHighPower(); // Always use this for RFM69HCW

  // Turn on encryption if desired:
  if (ENCRYPT)
    radio.encrypt(ENCRYPTKEY);

  // initialize the random number generator 
  randomSeed(analogRead(A3));


// Initialize all the pixelStrips
  Strand1.begin();
  Strand2.begin();
  Strand3.begin();
  Strand3up.begin();
  Strand3down.begin();
  Strand4.begin();
  Strand5.begin();
  Strand6.begin();

  // Start with all strands dark
  Strand1.Fade(Strand1.Color(0,0,0), Strand1.Color(0,0,0), 4, 10);
  Strand2.Fade(Strand2.Color(0,0,0), Strand2.Color(0,0,0), 4, 10);
  Strand3.Fade(Strand3.Color(0,0,0), Strand3.Color(0,0,0), 4, 10);
  Strand4.Fade(Strand4.Color(0,0,0), Strand4.Color(0,0,0), 4, 10);
  Strand5.Fade(Strand5.Color(0,0,0), Strand5.Color(0,0,0), 4, 10);
  Strand6.Fade(Strand6.Color(0,0,0), Strand6.Color(0,0,0), 4, 10);

  //set strips to less than full (255) brightness
  //Strand1.setBrightness(200);

  // Update the led strands
  Strand1.Update();
  Strand2.Update();    
  Strand3.Update();
  Strand3up.Update();
  Strand3down.Update();
  Strand4.Update();
  Strand5.Update();
  Strand6.Update();

}




// using a photoresistor in series with a 1 kOhm resistor to turn on strips in darkness only
//int darknessThreshold = 10;  //configure to your preferences 

int drawingCount = 0; // count how many time figure was drawn
int colorCount = 0;
int sharedColorCounter = 0;
long previousMillis = 0;
long interval = 3000;
bool blnNewSharedColor = false;
String randomColor = "";
int randomColorNum = 0;
int r = 0; int g = 0; int b = 0;
uint32_t currentColor = strip[0].Color(0, 0, 0);
bool blnPlusMinus = 0;
int morphAmount = random(10, 50);
int morphCount = 0; // keep track of number of times we morph
bool specialRoutine = false;  // rainbow and scanner



void loop()
{
  // Set up a "buffer" for characters that we'll send:
  // static char sendbuffer[62];

  // Choose a color randomly from the below selection
  // https://htmlcolorcodes.com/color-names/
  /*
red    rgb(255, 0, 0)
green  rgb(0, 128, 0)
blue   rgb(0, 0, 255)

yellow rgb(255, 255, 0)
orange rgb(255, 165, 0)
purple rgb(128, 0, 128))

pink     rgb(255, 192, 203)
hotpink  rgb(255, 105, 180)
maroon   rgb(128, 0, 0)
olive    rgb(128, 128, 0)
lime     rgb(0, 255, 0)
aqua     rgb(0, 255, 255)
teal     rgb(0, 128, 128)
navy     rgb(0, 0, 128)
fuchsia  rgb(255, 0, 255)
silver   rgb(192, 192, 192)
gold     rgb(255, 215, 0)
salmon   rgb(250, 128, 114)
coral    rgb(255, 127, 80)
khaki    rgb(240, 230, 140)
lavender rgb(230, 230, 250)
indigo   rgb(75, 0, 130)
chartreuse rgb(127, 255, 0)
olivedrab rgb(107, 142, 35)
cyan     rgb(0, 255, 255)
aquamarine  rgb(127, 255, 212)
turquoise rgb(64, 224, 208)
steelblue rgb(70, 130, 180)
  */


    r = 0; g = 0; b = 0;

    //randomColor = colorMatrix[ random(0, 28) ];   //String
    randomColorNum = random(0, 29);

    switch (randomColorNum) {
    case 0:    // aqua
      randomColor = "aqu";
      r = 0; g = 255; b = 255;
      break;

    case 1:    // aqua marine
      randomColor = "aqm";
      r=127; g=255; b=212;
      break;
    case 2: 
      randomColor = "blu";
      r=0; g=0; b=255;
      break;
    case 3: 
      randomColor = "cha";
      r=127; g=255; b=0;
      break;
    case 4: 
      randomColor = "cor";
      r=255; g=127; b=80;
      break;
    case 5: 
      randomColor = "cya";
      r=0; g=255; b=255;
      break;
    case 6: 
      randomColor = "fuc";
      r=255; g=0; b=255;
      break;
    case 7: 
      randomColor = "gol";
      r=255; g=215; b=0;
      break;
    case 8: 
      randomColor = "gre";
      r=0; g=128; b=0;
      break;
    case 9: 
      randomColor = "hot";
      r=255; g=105; b=180;
      break;
    case 10: 
      randomColor = "ind";
      r=75; g=0; b=130;
      break;
    case 11: 
      randomColor = "kha";
      r=240; g=230; b=140;
      break;
    case 12: 
      randomColor = "lav";
      r=230; g=230; b=250;
      break;
    case 13: 
      randomColor = "lim";
      r=0; g=255; b=0;
      break;
    case 14: 
      randomColor = "mar";
      r=128; g=0; b=0;
      break;
    case 15: 
      randomColor = "nav";
      r=0; g=0; b=128;
      break;
    case 16: 
      randomColor = "oli";
      r=128; g=128; b=0;
      break;
    case 17: 
      randomColor = "odr";
      r=107; g=142; b=35;
      break;
    case 18: 
      randomColor = "org";
      r=255; g=165; b=0;
      break;
    case 19: 
      randomColor = "pin";
      r=255; g=20; b=147;
      break;
    case 20: 
      randomColor = "pur";
      r=128; g=0; b=128;
      break;
    case 21: 
      randomColor = "red";
      r=255; g=0; b=0;
      break;
    case 22: 
      randomColor = "sal";
      r=250; g=128; b=114;
      break;
    case 23: 
      randomColor = "sil";  // lighter than default
      r=152; g=152; b=152;
      break;
    case 24: 
      randomColor = "ste";
      r=70; g=130; b=180;
      break;
    case 25: 
      randomColor = "tea";
      r=0; g=128; b=128;
      break;
    case 26: 
      randomColor = "tur";
      r=64; g=224; b=208;
      break;
    case 27: 
      randomColor = "yel";
      r=255; g=255; b=0;
      break;
    case 28: 
      randomColor = "rbw";   // Rainbow
      specialRoutine = true;
        r = 50; g = 50; b = 50;  // !!! delete once implemented
      break;
    case 29: 
      randomColor = "sca";  // Scanner
      specialRoutine = true;
        r = 50; g = 50; b = 50;  // !!! delete once implemented
      break;


    default:                // silver
      randomColor = "sil";
      r = 130; g = 70; b = 150;

      break;
  }
    



    int sendlength = 3;
    char theColor[sendlength];
    //theColor[0] = 'r'; theColor[1] = 'e'; theColor[2] = 'd';
    theColor[0] = randomColor.charAt(0); theColor[1] = randomColor.charAt(1); theColor[2] = randomColor.charAt(2);


    // Translate RGB to strip color
    //uint32_t currentColor = strip[0].Color(r, g, b);
    currentColor = strip[0].Color(r, g, b);


          Serial.println("-------");
          Serial.print("r: ");
          Serial.println(r);
          Serial.print("color: ");
          Serial.println(theColor);
          //Serial.println(randomColor);
          Serial.print("Current Color: ");
          Serial.println(currentColor);
    
    
    
    
     // DRAW SELF !!!
      // If new color, draw self 3 times
      for(int j = 0; j < 3; j++)
      {
        for (int i = 0; i < numberOfStrips; i++)
          {
            if(i != 1 && i != 3)  // exclude waving arms
            {
              drawNeoPixelLine(i, currentColor);
              delay(250);
            }
          }
          delay(1500);

          
       }



        // Simulate waving by using neon-like animation, turning on and off neopixels on different strips
          // If new color WAVE
          wave(currentColor);



        // TRANSMIT !!!
            radio.send(TONODEID, theColor, sendlength);

    
            Blink(LED, 100, sendlength);  //blinking onboard LED to show transmitting

            // Transmitting multiple times to help with reception timing
            delay(1000);
            radio.send(TONODEID, theColor, sendlength);
            delay(1000);
            radio.send(TONODEID, theColor, sendlength);




    // MORPH COLOR !!!
    // R G B +/- random amount
    // causes selected color to slightly change with each drawing cycle, to provide variability

      // randomly choose plus or minus
      //bool blnPlusMinus = random(0,1);
            Serial.print("blnPlusMinus: ");
            Serial.println(blnPlusMinus);


    for (int j = 0; j < 3; j++)
    {
      // change the color
      if(blnPlusMinus)
      {
        r = min( r + morphAmount + 10, 255);   // change for each node for variation
        g = min( g + morphAmount, 255);
        b = min( b + morphAmount, 255);
      }
      else
      {
        r = max( r - morphAmount - 10, 0);
        g = max( g - morphAmount, 0);
        b = max( b - morphAmount, 0);
      }

      Serial.print("morphAmount: ");
      Serial.println(morphAmount);

      //uint32_t currentColor = strip[0].Color(r, g, b);
      currentColor = strip[0].Color(r, g, b);


    // DRAW MORPHED SELF
      for (int i = 0; i < numberOfStrips; i++)
        {
          if(i != 1 && i != 3)  // exclude waving arms
          {
            drawNeoPixelLine(i, currentColor);
            delay(250);
          }
        }



      delay(1500);

    }

    // Turn LEDs off
    for (int i = 0; i < numberOfStrips; i++)
      {
        ledsOff(i, 50);
      }
      delay(1000);




  // RECEIVING
  // In this section, we'll check with the RFM69HCW to see
  // if it has received any packets:
  /*
  if (radio.receiveDone()) // Got one!
  {
    // Print out the information:
    Serial.print("received from node ");
    Serial.print(radio.SENDERID, DEC);
    Serial.print(": [");

    // The actual message is contained in the DATA array,
    // and is DATALEN bytes in size:
    for (byte i = 0; i < radio.DATALEN; i++)
      Serial.print((char)radio.DATA[i]);

    Blink(LED, 100, 3);
  }
  */

    //Need random() to be outside so that it looks more random; inside of for() causes repeats
    blnPlusMinus = random(0,2);
    morphAmount = random(10, 50);

  
}  // loop()


// *******************************************************************


// by @GreenMoonArt
void drawNeoPixelLine(int stripNum, uint32_t lineColor)
{
  // this code draws one line at a time
  // using the term "line" to represent a representation of a figure drawing

  //uint32_t lineColor = strip[stripNum].Color(85, 191, 63);

  for (int j=0; j < strip[stripNum].numPixels(); j++) 
  {
    strip[stripNum].setPixelColor(j, lineColor);
    strip[stripNum].show();

    delay(35);
  }
}

void ledsOff(int i, uint8_t wait)
{
  for(uint16_t j=0; j<strip[i].numPixels(); j++) 
  {
    strip[i].setPixelColor(j, strip[i].Color(0, 0, 0));
  }

  strip[i].show();
  delay(wait);
}


// **********  simulate arm waving animation  **********
// main arm is longer than upper and lower waving portions (as if it waves from the elbow)
// by @GreenMoonArt
void wave(uint32_t waveColor)
{
  uint32_t offColor = strip[1].Color(random(0), random(0), random(0));
  int startPixel = 0;

  for(int n=0; n < 5; n++)  // how many times to wave
  {
    for(int i=1; i < 4; i++)  // right arm is [1, 2, 3]
    {
      if(i == 2)
        { startPixel = strip[2].numPixels() / 2;}
      else
        { startPixel = 0; }

      //wave up
      for (int j=startPixel; j < strip[i].numPixels(); j++) 
      {
        strip[i].setPixelColor(j, waveColor);
        strip[i].show();
      }
      delay(200);
      for (int j=startPixel; j < strip[i].numPixels(); j++) 
      {
        strip[i].setPixelColor(j, offColor);
        strip[i].show();
      }
    }

    //wave down
    for(int i=3; i > 0; i--)  // right arm is [1, 2, 3]
    {
      if(i == 2)
        { startPixel = strip[2].numPixels() / 2;}
      else
        { startPixel = 0; }

      //wave up
      for (int j=startPixel; j < strip[i].numPixels(); j++) 
      {
        strip[i].setPixelColor(j, waveColor);
        strip[i].show();
      }
      delay(200);
      for (int j=startPixel; j < strip[i].numPixels(); j++) 
      {
        strip[i].setPixelColor(j, offColor);
        strip[i].show();
      }
    }

    // leave "middle" arm on
    for (int j=0; j < strip[2].numPixels(); j++) 
      {
        strip[2].setPixelColor(j, waveColor);
        strip[2].show();
      }

  }

  
}


// Blink onboard LED
void Blink(byte PIN, int DELAY_MS, int BlinkNum)
// Blink an LED for a given number of ms
{
  for(int i=0; i < BlinkNum; i++)
  {
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN,LOW);
    delay(DELAY_MS);
  }
}




//------------------------------------------------------------
//Completion Routines - get called on completion of a pattern
//------------------------------------------------------------
 
// Strand1 Completion Callback
void Strand1Complete()
{
    //if (digitalRead(9) == LOW)  // Button #2 pressed
    if (true)
    {
        // Alternate color-wipe patterns with Strand2
        //Strand2.Interval = 40;
        Strand1.Color1 = Strand1.Wheel(random(255));
        //Strand1.Interval = 20000;
    }
    else  // Return to normal
    {
      Strand1.Reverse();
    }
}

// Strand2 Completion Callback
void Strand2Complete()
{
    //if (digitalRead(9) == LOW)  // Button #2 pressed
    if (true)
    {
        // Alternate color-wipe patterns with Strand1
        Strand1.Interval = 20;
        Strand2.Color1 = Strand2.Wheel(random(255));
        Strand2.Interval = 20000;
    }
    else  // Return to normal
    {
        Strand2.RainbowCycle(random(0,10));
    }
}

// Strand3 Completion Callback
void Strand3Complete()
{
    // Random color change for next scan
    Strand3.Color1 = Strand3up.Wheel(random(255));
}

void Strand3upComplete()
{
    // Random color change for next scan
    Strand3up.Color1 = Strand3up.Wheel(random(255));
}

void Strand3downComplete()
{
    // Random color change for next scan
    Strand3down.Color1 = Strand3down.Wheel(random(255));
}


// Strand4 Completion Callback
void Strand4Complete()
{
    // Random color change for next scan
    Strand4.Color1 = Strand4.Wheel(random(255));
}

// Strand5 Completion Callback
void Strand5Complete()
{
    // Random color change for next scan
    Strand5.Color1 = Strand5.Wheel(random(255));
}

// Strand6 Completion Callback
void Strand6Complete()
{
    // Random color change for next scan
    Strand6.Color1 = Strand6.Wheel(random(255));
}






