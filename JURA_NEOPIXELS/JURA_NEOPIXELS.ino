/*
  TODO:
  o Implement missing modes
  o Add some more colors that have no value under 100 (RGB)
  o Implement random mode-switching
  o Refactor Strobo-function
  x Fix and finish (random segment length) new Strobo-function
  x Fix random-color-mode
  x Make speed and color unchangeable in Rainbow and only color unchangeable in Rainbow_Refresh
  x Find solution for loop-pressing after some time
  x Fix Rainbow-Mode
  x Make speed unchangeable in Single_Color
  x Fix >10 centering problem
  x Fix Speeds over 6
  x Implement racing lights with constantly switching segments
  x Implement random color-option
  x Center numbers that are bigger than 9 on OLED-Menu-Counters
  x Fix weird colors after the defined ones
  x Fix unreachable modes (Single Color)
  x Fix wrong count of button clicks for Speed and Color
  x Prevent loop-clicks
*/

#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//PIN-Constants
#define BTN0_PIN 7  //Brightness
#define BTN1_PIN 8  //Speed
#define BTN2_PIN 12 //Color
#define BTN3_PIN 13 //Mode
#define LED_PIN 6

//Button-Constants
#define BUTTON_INTERVAL 250 // Minimal time that has to pass between two button presses

//Color-Constants
const unsigned int COLORS[9][3] =
    {
        {255, 255, 255}, //White
        {255, 0, 0},     //Red
        {0, 255, 0},     //Green
        {0, 0, 255},     //Blue
        {255, 255, 0},   //Yellow
        {0, 255, 255},   //Cyan
        {255, 0, 255},   //Pink
        {255, 102, 0},   //Orange
        {102, 0, 204}    //Purple
};

//Color-Name-Constants
const String COLOR_NAMES[10] = {"White", "Red", "Green", "Blue", "Yellow", "Cyan", "Pink", "Orange", "Purple", "Random"};
const unsigned int COLOR_NAMES_SIZE = sizeof COLOR_NAMES / sizeof COLOR_NAMES[0]; //Devides length of array(first dimension -> 0) / size of datatype

//Mode-Name-Constants
const String MODE_NAMES[30] = {"Single Color", "Racing Pixels[1]", "Racing Pixels[3]", "Racing Pixels[5]", "Racing Pixels Rd.[1]", "Racing Pixels Rd.[3]", "Racing Pixels Rd.[5]", "Carousel [1]", "Carousel [3]", "Carousel [5]", "Strobo", "Strobo Segments[4]", "Strobo Segments[8]", "Strobo Seg. Switch[4]", "Strobo Seg. Switch[8]", "Strobo Segments Rd.", "Rainbow", "Rainbow Refresh", "Comets", "Comets Random", "Fire", "Stars", "Stacking Left", "Stacking Right", "Stacking Both Sides", "Stacking Middle", "Music", "Random"};
const unsigned int MODE_NAMES_SIZE = sizeof MODE_NAMES / sizeof MODE_NAMES[0]; //Devides length of array(first dimension -> 0) / size of datatype

//Speed-Constants
const unsigned int SPEEDS[11] = {3000, 1500, 1000, 700, 500, 300, 100, 50, 30, 15, 1};
const unsigned int SPEEDS_SIZE = sizeof SPEEDS / sizeof SPEEDS[0]; //Devides length of array(first dimension -> 0) / size of datatype

//LED-Strip-Constants
#define LED_COUNT 150 //150 per strip

//NeoPixel-Strip-Object:
Adafruit_NeoPixel pixels(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed

//OLED-Display-Constants
#define SCREEN_WIDTH 128 //OLED display width, in pixels
#define SCREEN_HEIGHT 64 //OLED display height, in pixels
#define OLED_RESET 4     //Reset pin # (or -1 if sharing Arduino reset pin)

//OLED-Display-Object
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Variables to save when a button was pressed, to prevent a "loop-press"
unsigned long btn0Pressed = 0;
unsigned long btn1Pressed = 0;
unsigned long btn2Pressed = 0;
unsigned long btn3Pressed = 0;

//Menu Variables
unsigned int menuBrightness = 50;
unsigned int menuSpeed = 0;
unsigned int menuColor = 0;
unsigned int menuMode = 0;

//Variable to indicate that random-color-mode is active
int randomColor = -1;

//Startup-Code
void setup()
{
  //For Debugging
  Serial.begin(9600);

  pinMode(LED_PIN, OUTPUT);

  //Initialize the pushbutton pins as inputs with integrated pullup resistor:
  pinMode(BTN0_PIN, INPUT_PULLUP);
  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
  pinMode(BTN3_PIN, INPUT_PULLUP);

  //Initialize LED-Strip
  pixels.setBrightness(menuBrightness);
  pixels.begin();
  pixels.clear();
  pixels.show(); //Initialize all pixels to 'off'

  //Initialize OLED-Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3C for 128x64
    for (;;)
      ; // Don't proceed if OLED isn't addressable
  }
  refreshOLED();

  //Startup-Melody
  tone(4, 70, 500);
  delay(500);
  tone(4, 100, 300);
  delay(350);
  tone(4, 100, 200);
  delay(200);
  tone(4, 200, 200);
}

//Main-Loop (Runs as long as the Board has power)
void loop()
{
  pixels.clear();
  buttonCheckDelay(0);
  pixels.setBrightness(menuBrightness);

  //For random color-function
  if (menuColor == 9 || randomColor != -1)
  {
    randomColor = random(0, COLOR_NAMES_SIZE - 1);
    menuColor = randomColor;
  }

  refreshOLED();

  switch (menuMode)
  {
  case 0: //Single_Color
    pixels.fill(pixels.Color(COLORS[menuColor][0], COLORS[menuColor][1], COLORS[menuColor][2]), 0, LED_COUNT);
    pixels.show();
    while (!buttonCheckDelay(100))
    {
    }
    break;
  case 1: //Racing_Pixels (1 Pixel)
    racingPixels(1, false);
    break;
  case 2: //Racing_Pixels (3 Pixel)
    racingPixels(3, false);
    break;
  case 3: //Racing_Pixels (5 Pixel)
    racingPixels(5, false);
    break;
  case 4: //Racing_Pixels - Random color per pixel-segment (1 Pixel)
    racingPixels(1, true);
    break;
  case 5: //Racing_Pixels - Random color per pixel-segment (3 Pixel)
    racingPixels(3, true);
    break;
  case 6: //Racing_Pixels - Random color per pixel-segment (5 Pixel)
    racingPixels(5, true);
    break;
  case 7: //Carousel (1 Pixel)
    carousel(1);
    break;
  case 8: //Carousel (3 Pixel)
    carousel(3);
    break;
  case 9: //Carousel (5 Pixel)
    carousel(5);
    break;
  case 10: //Strobo (Full-Strip)
    strobo(0, false);
    break;
  case 11: //Strobo (4 on / 4 off)
    strobo(4, false);
    break;
  case 12: //Strobo (8 on / 8 off)
    strobo(8, false);
    break;
  case 13: //Strobo switching (4 on / 4 off)
    strobo(4, true);
    break;
  case 14: //Strobo switching (8 on / 8 off)
    strobo(8, true);
    break;
  case 15: //Strobo random segment length
    strobo(100, true);
    break;
  case 16: //Rainbow
    for (unsigned int r = 0; r < LED_COUNT; r++)
    {
      pixels.setPixelColor(r, random(0, 255), random(0, 255), random(0, 255));
    }
    pixels.show();

    while (!buttonCheckDelay(100))
    {
    }
    break;
  case 17: //Rainbow_Refresh
    while (!buttonCheckDelay(SPEEDS[menuSpeed]))
    {
      for (unsigned int r = 0; r < LED_COUNT; r++)
      {
        pixels.setPixelColor(r, random(0, 255), random(0, 255), random(0, 255));
      }
      pixels.show();
    }
    break;
  }

  if (menuColor != randomColor && randomColor != -1)
  {
    randomColor = -1;
    menuColor = 0;
  }
}

void racingPixels(unsigned int pixelAmount, boolean randomColorEach)
{
  while (true)
  {
    //For random-color-function
    if (randomColor != -1)
    {
      randomColor = random(0, COLOR_NAMES_SIZE - 1);
      menuColor = randomColor;
    }

    for (unsigned int p = 0; p < LED_COUNT; p += pixelAmount)
    {
      if (randomColorEach)
      { //Built in random-color-function
        menuColor = random(0, 8);
      }

      //Turn all previous LEDs off
      if (p > pixelAmount - 1)
      {
        for (unsigned int o = pixelAmount; o > 0; o--)
        {
          pixels.setPixelColor(p - o, 0, 0, 0);
        }
      }

      //Turn new LEDs on
      for (unsigned int c = 0; c < pixelAmount; c++)
      {
        pixels.setPixelColor(p + c, COLORS[menuColor][0], COLORS[menuColor][1], COLORS[menuColor][2]);
      }

      pixels.show();
      if (buttonCheckDelay(SPEEDS[menuSpeed]))
      {
        return;
      }
    }

    //Turn last LEDs off
    for (int e = (LED_COUNT - 1) - pixelAmount; e < LED_COUNT; e++)
    {
      pixels.setPixelColor(e, 0, 0, 0);
    }
  }
}

void carousel(unsigned int pixelAmount)
{
  boolean switchColor = true;
  unsigned int moveCounter = 0;

  while (!buttonCheckDelay(SPEEDS[menuSpeed]))
  {
    for (unsigned int m = 0; m < moveCounter; m++)
    {
      pixels.setPixelColor(m, 0, 0, 0);
    }

    for (unsigned int c = moveCounter; c < LED_COUNT; c += pixelAmount)
    {
      if (switchColor)
      {
        //For random-color-function
        if (randomColor != -1)
        {
          randomColor = random(0, COLOR_NAMES_SIZE - 1);
          menuColor = randomColor;
        }
        pixels.fill(pixels.Color(COLORS[menuColor][0], COLORS[menuColor][1], COLORS[menuColor][2]), c - pixelAmount, c);
      }
      else
      {
        pixels.fill(pixels.Color(0, 0, 0), c - pixelAmount, c);
      }

      if ((c + pixelAmount) >= LED_COUNT)
      {
        for (unsigned int e = c; e < LED_COUNT; e++)
        {
          if (switchColor)
          {
            pixels.setPixelColor(e, 0, 0, 0);
          }
          else
          {
            if (randomColor != -1)
            {
              randomColor = random(0, COLOR_NAMES_SIZE - 1);
              menuColor = randomColor;
            }
            pixels.setPixelColor(e, COLORS[menuColor][0], COLORS[menuColor][1], COLORS[menuColor][2]);
          }
        }
      }

      //To color one segment and turn the following off
      switchColor = !switchColor;
    }

    pixels.show();

    moveCounter++;
    if (moveCounter > pixelAmount)
    {
      moveCounter = 0;
    }
  }
}

void strobo(unsigned int pixelAmount, boolean switching)
{
  boolean switchState = false;

  while (!buttonCheckDelay(0))
  {
    //For random-color-function
    if (randomColor != -1)
    {
      randomColor = random(0, COLOR_NAMES_SIZE - 1);
      menuColor = randomColor;
    }

    if (pixelAmount == 0) //Full-strip strobo
    {
      pixels.fill(pixels.Color(COLORS[menuColor][0], COLORS[menuColor][1], COLORS[menuColor][2]), 0, LED_COUNT);
    }
    else if (pixelAmount == 100 && switching) //Random-Segment-Length strobo
    {
      unsigned int randomSegLength = random(1, 5);

      for (unsigned int i = 0; i < LED_COUNT; i += randomSegLength)
      {
        if (randomColor != -1)
        {
          if (switchState)
          {
            randomColor = random(0, COLOR_NAMES_SIZE - 1);
            menuColor = randomColor;

            pixels.fill(pixels.Color(COLORS[menuColor][0], COLORS[menuColor][1], COLORS[menuColor][2]), i - randomSegLength, i);
          }
          else
          {
            pixels.fill(pixels.Color(0, 0, 0), i - randomSegLength, i);
          }
        }
        else
        {
          if (switchState)
          {
            pixels.fill(pixels.Color(COLORS[menuColor][0], COLORS[menuColor][1], COLORS[menuColor][2]), i - randomSegLength, i);
          }
          else
          {
            pixels.fill(pixels.Color(0, 0, 0), i - randomSegLength, i);
          }
        }

        randomSegLength = random(1, 5);
        switchState = !switchState;
      }
    }
    else //Defined-Segment-Length strobo (Sry for this crappy implementation)
    {
      for (unsigned int i = 0; i < LED_COUNT; i += (pixelAmount * 2))
      {
        if (switching && switchState || !switching)
        {
          pixels.fill(pixels.Color(COLORS[menuColor][0], COLORS[menuColor][1], COLORS[menuColor][2]), i - (pixelAmount * 2), i - pixelAmount);
        }
        else
        {
          pixels.fill(pixels.Color(0, 0, 0), i - (pixelAmount * 2), i - pixelAmount);
        }

        if (switching && switchState || !switching)
        {
          pixels.fill(pixels.Color(0, 0, 0), i - pixelAmount, i);
        }
        else
        {
          pixels.fill(pixels.Color(COLORS[menuColor][0], COLORS[menuColor][1], COLORS[menuColor][2]), i - pixelAmount, i);
        }

        if ((i + (pixelAmount * 2)) >= LED_COUNT)
        {
          for (unsigned int e = i; e < LED_COUNT; e++)
          {
            if ((e - i) < pixelAmount)
            {
              if (switching && switchState || !switching)
              {
                pixels.setPixelColor(e, COLORS[menuColor][0], COLORS[menuColor][1], COLORS[menuColor][2]);
              }
              else
              {
                pixels.setPixelColor(e, 0, 0, 0);
              }
            }
            else
            {
              if (switching && switchState || !switching)
              {
                pixels.setPixelColor(e, 0, 0, 0);
              }
              else
              {
                pixels.setPixelColor(e, COLORS[menuColor][0], COLORS[menuColor][1], COLORS[menuColor][2]);
              }
            }
          }
        }
      }
      switchState = !switchState;
    }

    pixels.show();
    if (buttonCheckDelay(SPEEDS[menuSpeed]))
    {
      return;
    }

    pixels.clear();
    pixels.show();
    if (buttonCheckDelay(SPEEDS[menuSpeed]))
    {
      return;
    }
  }
}

boolean buttonCheckDelay(unsigned int delayVal)
{
  boolean btn0State, btn1State, btn2State, btn3State; //Variables to save the state of a button

  for (int d = delayVal; d > 0; d -= 100)
  {
    //Read state of all buttons and save them into the coresponding variables
    btn0State = digitalRead(BTN0_PIN);
    btn1State = digitalRead(BTN1_PIN);
    btn2State = digitalRead(BTN2_PIN);
    btn3State = digitalRead(BTN3_PIN);

    //Reactions to button-pressing
    //BTN0 --> Brightness up
    if ((millis() - btn0Pressed) > BUTTON_INTERVAL)
    {
      if (btn0State == LOW)
      {
        btn0Pressed = millis();
        menuBrightness += 25;
        if (menuBrightness > 255)
        {
          menuBrightness = 0;
        }

        tone(4, 100, 500);
        return true;
      }
    }

    //BTN1 --> Speed up
    if (((millis() - btn1Pressed) > BUTTON_INTERVAL) && menuMode != 0 && menuMode != 17)
    {
      if (btn1State == LOW)
      {
        btn1Pressed = millis();
        if (menuSpeed++ >= (SPEEDS_SIZE - 1))
        {
          menuSpeed = 0;
        }

        tone(4, 200, 500);
        return true;
      }
    }

    //BTN2 --> Change Color
    if (((millis() - btn2Pressed) > BUTTON_INTERVAL) && (menuMode < 4 || menuMode > 6) && menuMode != 17 && menuMode != 18) //!Racing-Pixels-Rd. && !Rainbow && !Rainbowrefresh
    {
      if (btn2State == LOW)
      {
        btn2Pressed = millis();
        if (menuColor++ > COLOR_NAMES_SIZE)
        {
          menuColor = 0;
        }

        tone(4, 300, 500);
        return true;
      }
    }

    //BTN3 --> Change Mode
    if ((millis() - btn3Pressed) > BUTTON_INTERVAL)
    {
      if (btn3State == LOW)
      {
        btn3Pressed = millis();
        if (menuMode++ >= MODE_NAMES_SIZE - 1)
        {
          menuMode = 0;
        }

        tone(4, 400, 500);
        return true;
      }
    }

    if (delayVal < 100)
    {
      delay(delayVal);
    }
    else
    {
      delay(100);
    }
  }
  return false;
}

void refreshOLED()
{
  //Clear the display-buffer
  display.clearDisplay();

  //Set text-size and color for button-description
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  //Print button-description and line above
  display.setCursor(0, 56);
  display.println(F("Light Speed RGBs Mode"));
  display.drawLine(0, 54, 128, 54, SSD1306_WHITE);

  //Print current mode
  display.setCursor(centerText(MODE_NAMES[menuMode], 1), 0);
  display.println(MODE_NAMES[menuMode]);

  //Print current color-name
  display.setTextSize(2);
  if (randomColor != -1 || (menuMode > 3 && menuMode < 7))
  {
    display.setCursor(centerText(COLOR_NAMES[COLOR_NAMES_SIZE], 2), 15);
    display.println(COLOR_NAMES[COLOR_NAMES_SIZE - 1]);
  }
  else
  {
    display.setCursor(centerText(COLOR_NAMES[menuColor], 2), 15);
    display.println(COLOR_NAMES[menuColor]);
  }

  //Print current Brightness
  display.setCursor(10, 38);
  if (menuBrightness / 25 >= 10) //Center all numbers > 9
  {
    display.setCursor(3, 38);
  }
  display.println(menuBrightness / 25);

  //Print current Speed
  display.setCursor(45, 38);
  if (menuSpeed >= 10) //Center all numbers > 9
  {
    display.setCursor(38, 38);
  }
  display.println(menuSpeed);

  //Print current Color
  display.setCursor(77, 38);
  if (randomColor != -1 || (menuMode > 3 && menuMode < 7))
  {
    display.println("0");
  }
  else
  {
    if (menuColor >= 10) //Center all numbers > 9
    {
      display.setCursor(72, 38);
    }
    display.println(menuColor);
  }

  //Print current Mode
  display.setCursor(107, 38);
  if (menuMode >= 10) //Center all numbers > 9
  {
    display.setCursor(102, 38);
  }
  display.println(menuMode);

  //Print line above counters
  display.drawLine(0, 35, 128, 35, SSD1306_WHITE);

  //Print lines between counters
  display.drawLine(32, 35, 32, 64, SSD1306_WHITE);
  display.drawLine(68, 35, 68, 64, SSD1306_WHITE);
  display.drawLine(98, 35, 98, 64, SSD1306_WHITE);

  //Show prepared OLED-content
  display.display();
}

unsigned int centerText(String text, unsigned int size)
{
  unsigned int textLengthPixels;

  if (size == 1)
  {
    textLengthPixels = text.length() * 6;
  }
  else
  {
    textLengthPixels = text.length() * 12;
  }

  return (SCREEN_WIDTH / 2) - (textLengthPixels / 2);
}
