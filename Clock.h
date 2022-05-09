#ifndef CLOCK_H
#define CLOCK_H

#include "stdint.h"

#define CLOCK_DISPLAY_LENGTH 13
#define LCD_ROW_LENGTH 16


enum weekDays {sun, mon, tue, wed, thu, fri, sat};


class Clock
{
	unsigned int elapsedSecs;
	uint8_t feedCnt;
	uint8_t sec, min, hour;
	weekDays day;
	
public:
	Clock();
	void setDay(const weekDays);
	void setSec();
	void setMin(const uint8_t);
	void setHour(const uint8_t);
	uint8_t getMin() const;
	uint8_t getHour() const;
	weekDays getDay() const;
	void tic();
	void clock2str(char*);
	void clock2strHist(char*);
	unsigned int getElapsedSecs() const;
  void setFeedCnt();
  uint8_t getFeedCnt() const;
  void getDayStr(char*);
};



#endif //CLOCK_H
