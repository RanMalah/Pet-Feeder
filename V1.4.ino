// ***********************************************************
// **  7/1/2021         The Pet Feeder                      **
// **                  ^^^^^^^^^^^^^^^^                     **
// **                   by Ran Malach                       **
// **                                                       **
// **   Control a DC motor to drive an AUGER Doser.         **
// **   Two buttons L(eft) & R(ight).                       **
// **   Limit Feeds to twice a day + a bonus feed.          **
// **   State machine implementation.                       **
// ***********************************************************

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "Clock.h"

#define LCD_ROW_LENGTH 16
#define DEBOUNCE 200
#define CLOCK_DISPLAY_LENGTH 13
#define DELAY_FOR_BACKLIGHT_IN_SECONDS 25
#define NUM_OPTIONS_MAIN_MENU 4
#define MAX_FEED_IN_SECONDS 20

#define AUGER  13
#define L_BUTTON  8
#define R_BUTTON  7
#define LED1 10
#define LED2 11
#define LED3 12

enum fsm {IDLE, MENU, SET_TIME, SET_FEED_FACTOR, HIST};
fsm state = IDLE;

// Setting the length of feeds
uint8_t feedingFactor = 1;

// This array holds a week of feeding history.
// Every feed will be written here.
char feedingHistory[7][LCD_ROW_LENGTH * 2] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

// For the two buttons:
//    - state is digitally read, normally HIGH
//    - previous is the last clock button state, 
//      its purpose is for ignoring the button while its pressed and wait for release
//    - on is the buttom line... 
bool Lon = LOW, Ron = LOW, Lstate = HIGH, Rstate = HIGH;
bool Lprevious = HIGH, Rprevious = HIGH;

// For simplyfing the menu
bool firstRun = true;

// For scrolling menus
uint8_t optionSelect = 0;

// Counting the period of pressing buttons.
// Its making the buttons pressing more stable & it will help to set the clock.
unsigned long Lt = 0, Rt = 0;

// t is for counting the auger operating duration
// dt is for counting a second == 1000 * millis()
unsigned long t = 0, dt = 0; 

// Counting the seconds of backlight for shutdown..
unsigned int backLightOnSecCnt = 0;

// String for the display in IDLE state
char clockStr[CLOCK_DISPLAY_LENGTH];

// Checkout my Clock.h implemantation
Clock clk;

LiquidCrystal_I2C lcd(0x27, LCD_ROW_LENGTH, 2);  // set the LCD address to 0x27 for a 16 chars and 2 line display


void setup() 
{

 //Serial.begin(9600);
  pinMode(AUGER, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  pinMode(L_BUTTON, INPUT);
  pinMode(R_BUTTON, INPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(1,0);
  lcd.print("The Pet Feeder");
  lcd.setCursor(2,1);
  lcd.print("By Ran Malach");
  delay(2000);
  lcd.clear();
}

void loop() 
{
  if(millis() - dt > 999)
  // Count a second.
      {
        dt = millis();
        // Pass a sec in clock
        clk.tic();
        if(!clk.getFeedCnt())
        // the feed counter zeroes at midnight, so...
        // turn off the lights, g'night...
        {
          digitalWrite(LED1, LOW);
          digitalWrite(LED2, LOW);
          digitalWrite(LED3, LOW);
        }
        if(state == IDLE)
        // Show the clock each sec in IDLE state
        {
          lcd.setCursor(2, 1);
          clk.clock2str(clockStr);
          lcd.print(clockStr);
        }
      }
  if(clk.getElapsedSecs() - backLightOnSecCnt > DELAY_FOR_BACKLIGHT_IN_SECONDS)
  // Turn off the backlight after several seconds of no activity
  // Each press on one of the two buttons resets the backLightOnSecCnt variable
  {
    backLightOnSecCnt = clk.getElapsedSecs();
    lcd.clear();
    lcd.noBacklight(); 
    optionSelect = 0;
    state = IDLE;
  }    
  // ------------------------------------------------------
  // Read the button's state (check the func. down below) |
                        buttonsRead();         //         |
  // ------------------------------------------------------

  
  // ************************   FSM   ********************************
  switch(state)
  {
    case IDLE:  // ********** fsm - IDLE   *******************************
      if(millis() - t > 1000 * (unsigned long)(feedingFactor / (clk.getFeedCnt() != 3 ? 1 : 2)))
          // Stop feeding.
          // The ternary expression is for the Bonus feed.     [   ^       ^       ^      ^  ]
          // If that Hungry little one wants an extra meal, give it half a dose.
          {
            digitalWrite(AUGER, LOW);
          }
          else
          // Disable pressing 'feed' while feeding
          {
            Lon = LOW;
          }
          
      if(Lon)
      // Left button is pressed
        { 
          if(clk.getFeedCnt() < 3)
          // Check if the feeding counter enables feeding
            {  
              // Feed
              digitalWrite(AUGER, HIGH);
              // Turn on the appropriate led
              digitalWrite(LED1 + clk.getFeedCnt(), HIGH);
              // Advance feed Counter
              clk.setFeedCnt();
              // Write this feed to the feeding HISTORY's array
              clk.clock2strHist(feedingHistory[clk.getDay()]);
              lcd.print("Feed");
              t = millis();
              lcd.clear();
           }
        }
      if(Ron)
      // Right button is pressed
      {
        state = MENU;
        menuDisp();
      }
    break;  // end case IDLE
    
    case MENU:  // ********** fsm -   MENU   *******************************
      if(Lon)
      {
        switch(optionSelect)
        {
          case 0:
            state = HIST;
            firstRun = true;
          break;
          case 1:
            state = SET_TIME;
            optionSelect = 0;
            firstRun = true;
            lcd.clear();
          break;
          case 2:
            state = SET_FEED_FACTOR;
            feedDisp();      
            optionSelect = 0;           
          break;
          case 3:
            state = IDLE;
            lcd.clear();
            optionSelect = 0;
          break;
          
          default:
          break;
        }
        
      }
      if(Ron)
      {
        optionSelect++;
        if(optionSelect == NUM_OPTIONS_MAIN_MENU)
          optionSelect = 0;
        menuDisp();         
      }
    break; // end case MENU

    case SET_FEED_FACTOR:  // ********** fsm -   SET_FEED_FACTOR   ************************
      if(Lon)
      {
        state = IDLE;
        lcd.clear();
      }
      if(Ron)
      {
        feedingFactor++;
        if(feedingFactor > MAX_FEED_IN_SECONDS)
          feedingFactor = 1;
        feedDisp();
      }
    break;

    case SET_TIME: // ************* fsm -   SET_TIME   *************************
      setClock();
    break;
    
    case HIST: // ******************* fsm -   HIST   *********************
        if(firstRun)
        {
            lcd.clear();
            // Use optionSelect as the day to show..
            optionSelect = clk.getDay();
            lcd.print(feedingHistory[optionSelect]);
            lcd.setCursor(0, 1);
            // Well, the feedingHistory array is of size : [7][two times row length]
            lcd.print(&feedingHistory[optionSelect][LCD_ROW_LENGTH]);
            firstRun = false;
          }
        if(Ron)
        {
          optionSelect++;
          if(optionSelect == 7)
            optionSelect = 0;
          lcd.clear();
          lcd.print(feedingHistory[optionSelect]);
          lcd.setCursor(0, 1);
          lcd.print(&feedingHistory[optionSelect][LCD_ROW_LENGTH]);
        }
        if(Lon)
        {
          optionSelect = 0;
          state = IDLE;  
          lcd.clear();
        }
       
    break;
    default: // *************   DEFAULT   *************************
    break;
    
  } //************************** end fsm - switch state  *******************************
}

void buttonsRead()
{
  Lstate = digitalRead(L_BUTTON);
  Rstate = digitalRead(R_BUTTON);
  
  if(!Lstate && Lprevious && millis() - Lt > DEBOUNCE)
  {
    Lon = HIGH;
    Lt = millis();
    backLightOnSecCnt = clk.getElapsedSecs();
    lcd.backlight();
  }
  else
    Lon = LOW;

  if(!Rstate && (state == SET_TIME ? 1 : Rprevious) && millis() - Rt > DEBOUNCE)
  {
    Ron = HIGH;
    Rt = millis();
    backLightOnSecCnt = clk.getElapsedSecs();
    lcd.backlight();
  }
  else
    Ron = LOW;

  Lprevious = Lstate;
  Rprevious = Rstate;
}

void menuDisp()
{
  uint8_t op[] = {11, 0, 6, 11};
  lcd.clear();
  lcd.print("MENU:");
  lcd.setCursor(12, 0);
  lcd.print("hist");
  lcd.setCursor(0, 1);
  lcd.print(" clock feed back");
  // Use that ternary to switch between rows
  lcd.setCursor(op[optionSelect], optionSelect == 0 ? 0 : 1);
  lcd.print('*');
}

void feedDisp()
{
  lcd.clear();
  lcd.print("Feeding Duration");
  lcd.setCursor(7, 1);
  lcd.print(feedingFactor);
}

void setClock()
{
 char dayStr[4];
  if(Lon)
  {
    optionSelect++;
    firstRun = true;
    lcd.clear();
  }
    switch(optionSelect)
    {
      case 0:
              if(Ron)
              {
                clk.setMin(clk.getMin() + 1);
                lcd.clear();
                firstRun = true;
              }
              if(firstRun)
              {
              lcd.print("Minutes: ");
              lcd.print(clk.getMin() / 10);
              lcd.print(clk.getMin() % 10);
              firstRun = false;
              }
      break;
      case 1:
              if(Ron)
              {
                clk.setHour(clk.getHour() + 1);
                lcd.clear();
                firstRun = true;
              }
              if(firstRun)
              {
              lcd.print("Hours: ");
              lcd.print(clk.getHour() / 10);
              lcd.print(clk.getHour() % 10);
              firstRun = false;
              }
      break;
      case 2:
              if(Ron)
              {
                clk.setDay((weekDays)((clk.getDay() + 1) % 7));
                lcd.clear();
                firstRun = true;
              }
              if(firstRun)
              {
              lcd.print("Day: ");
              clk.getDayStr(dayStr);
              lcd.print(dayStr);
              firstRun = false;
              }
      break;
      default:
              optionSelect = 0;
              state = IDLE;
      break;
    }  
    
}
