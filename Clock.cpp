
#include "Clock.h"

char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

  
	Clock::Clock()
	{
		elapsedSecs = 0;
    feedCnt = 0;
		day = sun;
		sec = 0;
		min = 0;
		hour = 0;
	}
	Clock::Clock(const Clock& other)
	{
    feedCnt = other.feedCnt;
    elapsedSecs = other.elapsedSecs;
		sec = other.sec;
		min = other.min;
		hour = other.hour;
		day = other.day;
	}
	void Clock::setDay(const weekDays d)
	{
		day = d;
	}
	void Clock::tic()
  // This func should be called every second.
  // It counts and updates the clock, day, min, hour
  // count elapsed seconds as well.
	{
		elapsedSecs++;
    
		if(sec == 59)
		{
			sec = 0;
			if(min == 59)
			{
				min = 0;
				if(hour == 23)
				{
					hour = 0;
					day == sat ? day = sun : day = (weekDays)(day + 1);
          // Its a new day, a new dawn so... reset the feeding counter
          feedCnt = 0;
				}
				else hour++;
			}
			else
				min++;
		}
		else 
			sec++;
	}

	void Clock::clock2str(char* str)
  // For the display in the IDLE state.
	{
		
		str[2] = ':';
		str[5] = ':';
		str[8] = ' ';
		str[0] = (char)(hour / 10) + '0';
		str[1] = (char)(hour % 10) + '0';
		str[3] = (char)(min / 10) + '0';
		str[4] = (char)(min % 10) + '0';
		str[6] = (char)(sec / 10) + '0';
		str[7] = (char)(sec % 10) + '0';
		for(int i = 0; i < 3; i++)
			str[9 + i] = days[day][i];
		str[12] = 0;
	}
  
	void Clock::clock2strHist(char* str)
	{
   // For the display in the HIST(ory) state. 
   // 
		switch(feedCnt)
   {
    case 1:
              // Erase all feeds from a week ago
              for(int i = 3; i < LCD_ROW_LENGTH * 2; i++)
              str[i] = 0;
              
              // Write first feed
              str[0 + LCD_ROW_LENGTH] = '1';
              str[1 + LCD_ROW_LENGTH] = ')';
              str[2 + LCD_ROW_LENGTH] = (char)(hour / 10) + '0';
              str[3 + LCD_ROW_LENGTH] = (char)(hour % 10) + '0';
              str[4 + LCD_ROW_LENGTH] = ':';
              str[5 + LCD_ROW_LENGTH] = (char)(min / 10) + '0';
              str[6 + LCD_ROW_LENGTH] = (char)(min % 10) + '0';
              str[7 + LCD_ROW_LENGTH] = ' ';
              break;
    case 2:
              // Write second feed
              str[8 + LCD_ROW_LENGTH] = '2';
              str[9 + LCD_ROW_LENGTH] = ')';
              str[10 + LCD_ROW_LENGTH] = (char)(hour / 10) + '0';
              str[11 + LCD_ROW_LENGTH] = (char)(hour % 10) + '0';
              str[12 + LCD_ROW_LENGTH] = ':';
              str[13 + LCD_ROW_LENGTH] = (char)(min / 10) + '0';
              str[14 + LCD_ROW_LENGTH] = (char)(min % 10) + '0';          
    break;
    case 3:
              // Write bonus- half feed
              str[3] = ' ';
              str[4] = ' ';
              str[5] = ' ';
              str[6] = '1';
              str[7] = '/';
              str[8] = '2';
              str[9] = ')';
              str[10] = (char)(hour / 10) + '0';
              str[11] = (char)(hour % 10) + '0';
              str[12] = ':';
              str[13] = (char)(min / 10) + '0';
              str[14] = (char)(min % 10) + '0';           
    break;
    
    default:
    break;
   }
	}
	unsigned int Clock::getElapsedSecs() const
	{	return elapsedSecs;	}
	void Clock::setSec(){sec = 0;}
	void Clock::setMin(const uint8_t t){min = t > 59 ? 0 : t;}
	void Clock::setHour(const uint8_t t){hour = t > 23 ? 0 : t;}
	uint8_t Clock::getMin() const {return min;}
	uint8_t Clock::getHour() const {return hour;}
	weekDays Clock::getDay() const {return day;}
  void Clock::setFeedCnt(){ feedCnt++; }
  uint8_t Clock::getFeedCnt() const {return feedCnt;}
  void Clock::getDayStr(char* t)
  {
    t[3] = 0;
    for(int i = 0; i < 3; i++)
      t[i] = days[day][i];  
  }
