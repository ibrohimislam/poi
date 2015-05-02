#pragma once

#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <ctime>

#include "const.h"
#include "entry.h"

using namespace std;

class Volume {
public:
  /* Method */
  
  /* konstruktor & destruktor */
  Volume();
  ~Volume();
  
  /* buat file *.poi */
  void create(const char *filename);
  void initVolumeInformation(const char *filename);
  void initAllocationTable();
  void initDataPool();
  
  /* baca file *.poi */
  void load(const char *filename);
  void readVolumeInformation();
  void readAllocationTable();
  
  void writeVolumeInformation();
  void writeAllocationTable(ptr_block position);
  
  /* bagian alokasi block */
  void setNextBlock(ptr_block position, ptr_block next);
  ptr_block allocateBlock();
  void freeBlock(ptr_block position);
  
  /* bagian baca/tulis block */
  int readBlock(ptr_block position, char *buffer, int size, int offset = 0);
  int writeBlock(ptr_block position, const char *buffer, int size, int offset = 0);

  /* Attributes */
  fstream handle;     // file .poi
  ptr_block nextBlock[N_BLOCK]; //pointer ke blok berikutnya
  
  string filename;    // nama volume
  int capacity;     // kapasitas filesystem dalam blok
  int available;      // jumlah slot yang masih kosong
  int firstEmpty;     // slot pertama yang masih kosong
  time_t mount_time;    // waktu mounting, diisi di konstruktor
};