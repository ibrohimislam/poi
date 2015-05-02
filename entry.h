#pragma once

#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <ctime>

#include "const.h"
#include "volume.h"

using namespace std;


class Entry {

public:
  /* Method */
  Entry();
  Entry(ptr_block position, unsigned char offset);
  Entry nextEntry();
  Entry getEntry(const char *path);
  Entry getNewEntry(const char *path);
  Entry getNextEmptyEntry();
  
  void makeEmpty();
  int isEmpty();
  
  string getName();
  unsigned char getAttr();
  short getTime();
  short getDate();
  ptr_block getIndex();
  int getSize();
  
  void setName(const char* name);
  void setAttr(const unsigned char attr);
  void setTime(const short time);
  void setDate(const short date);
  void setIndex(const ptr_block index);
  void setSize(const int size);
  
  time_t getDateTime();
  void setCurrentDateTime();
  void write();
  
  /* Attributes */
  char data[ENTRY_SIZE];
  ptr_block position; //posisi blok
  unsigned char offset; //offset dalam satu blok (0..15)
};
