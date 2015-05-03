#include <stdexcept>

#include "entry.h"

/* Global filesystem */
extern Volume filesystem;

using namespace std;

/** Konstruktor: buat Entry kosong */
Entry::Entry() {
	position = 0;
	offset = 0;
	memset(data, 0, ENTRY_SIZE);
}
/** Konstruktor parameter */
Entry::Entry(ptr_block position, unsigned char offset) {
	this->position = position;
	this->offset = offset;
	
	/* baca dari data pool */
	filesystem.handle.seekg(BLOCK_SIZE * DATA_POOL_OFFSET + position * BLOCK_SIZE + offset * ENTRY_SIZE);
	filesystem.handle.read(data, ENTRY_SIZE);
}

/** Mendapatkan Entry berikutnya */
Entry Entry::nextEntry() {
	if (offset < 15) {
		return Entry(position, offset + 1);
	}
	else {
		return Entry(filesystem.nextBlock[position], 0);
	}
}

/** Mendapatkan Entry dari path */
Entry Entry::getEntry(const char *path) {
	/* mendapatkan direktori teratas */
	unsigned int endstr = 1;
	while (path[endstr] != '/' && endstr < strlen(path)) {
		endstr++;
	}
	string topDirectory = string(path + 1, endstr - 1);
	
	/* mencari entri dengan nama topDirectory */
	while (getName() != topDirectory && position != END_BLOCK) {
		*this = nextEntry();
	}
	
	/* kalau tidak ketemu, return Entry kosong */
	if (isEmpty()) {
		return Entry();
	}
	/* kalau ketemu, */
	else {
		if (endstr == strlen(path)) {
			return *this;
		}
		else {
			/* cek apakah direktori atau bukan */
			if (getAttr() & 0x8) {
				ptr_block index;
				memcpy((char*)&index, data + 0x1A, 2);
				Entry next(index, 0);
				return next.getEntry(path + endstr);
			}
			else {
				return Entry();
			}
		}
	}
}

/** Mendapatkan Entry dari path */
Entry Entry::getNewEntry(const char *path) {
	/* mendapatkan direktori teratas */
	unsigned int endstr = 1;
	while (path[endstr] != '/' && endstr < strlen(path)) {
		endstr++;
	}
	string topDirectory = string(path + 1, endstr - 1);
	
	/* mencari entri dengan nama topDirectory */
	Entry entry(position, offset);
	while (getName() != topDirectory && position != END_BLOCK) {
		*this = nextEntry();
	}
	
	/* kalau tidak ketemu, buat entry baru */
	if (isEmpty()) {
		while (!entry.isEmpty()) {
			if (entry.nextEntry().position == END_BLOCK) {
				entry = Entry(filesystem.allocateBlock(), 0);
			}
			else {
				entry = entry.nextEntry();
			}
		}
		/* beri atribut pada entry */
		entry.setName(topDirectory.c_str());
		entry.setAttr(0xF);
		entry.setIndex(filesystem.allocateBlock());
		entry.setSize(BLOCK_SIZE);
		entry.setTime(0);
		entry.setDate(0);
		entry.write();
		
		*this = entry;
	}
	
	if (endstr == strlen(path)) {
		return *this;
	}
	else {
		/* cek apakah direktori atau bukan */
		if (getAttr() & 0x8) {
			ptr_block index;
			memcpy((char*)&index, data + 0x1A, 2);
			Entry next(index, 0);
			return next.getNewEntry(path + endstr);
		}
		else {
			return Entry();
		}
	}
}

/** Mendapatkan Entry dari path */
Entry Entry::createLink(const char *path, const ptr_block) {
	/* mendapatkan direktori teratas */
	unsigned int endstr = 1;
	while (path[endstr] != '/' && endstr < strlen(path)) {
		endstr++;
	}
	string topDirectory = string(path + 1, endstr - 1);
	
	/* mencari entri dengan nama topDirectory */
	Entry entry(position, offset);
	while (getName() != topDirectory && position != END_BLOCK) {
		*this = nextEntry();
	}
	
	/* kalau tidak ketemu, buat entry baru */
	if (isEmpty()) {
		while (!entry.isEmpty()) {
			if (entry.nextEntry().position == END_BLOCK) {
				entry = Entry(filesystem.allocateBlock(), 0);
			}
			else {
				entry = entry.nextEntry();
			}
		}
		/* beri atribut pada entry */
		entry.setName(topDirectory.c_str());
		entry.setAttr(0xFF);
		entry.setIndex(ptr_block);
		entry.setSize(0);
		entry.setTime(0);
		entry.setDate(0);
		entry.write();
		
		*this = entry;
	}
	
	if (endstr == strlen(path)) {
		return *this;
	}
	else {
		/* cek apakah direktori atau bukan */
		if (getAttr() & 0x8) {
			ptr_block index;
			memcpy((char*)&index, data + 0x1A, 2);
			Entry next(index, 0);
			return next.getNewEntry(path + endstr);
		}
		else {
			return Entry();
		}
	}
}

/** Mengembalikan entry kosong selanjutnya. Jika blok penuh, akan dibuatkan entri baru */
Entry Entry::getNextEmptyEntry() {
	Entry entry(*this);
	
	while (!entry.isEmpty()) {
		entry = entry.nextEntry();
	}
	if (entry.position == END_BLOCK) {
		/* berarti blok saat ini sudah penuh, buat blok baru */
		ptr_block newPosition = filesystem.allocateBlock();
		ptr_block lastPos = position;
		while (filesystem.nextBlock[lastPos] != END_BLOCK) {
			lastPos = filesystem.nextBlock[lastPos];
		}
		filesystem.setNextBlock(lastPos, newPosition);
		entry.position = newPosition;
		entry.offset = 0;
	}
	
	return entry;
}

/** mengosongkan entry */
void Entry::makeEmpty() {
	/* menghapus byte pertama data */
	*(data) = 0;
	write();
}

/** Memeriksa apakah Entry kosong atau tidak */
int Entry::isEmpty() {
	return *(data) == 0;
}

/** Getter-Setter atribut-atribut Entry */
string Entry::getName() {
	return string(data);
}

unsigned char Entry::getAttr() {
	return *(data + 0x15);
}

short Entry::getTime() {
	short result;
	memcpy((char*)&result, data + 0x16, 2);
	return result;
}

short Entry::getDate() {
	short result;
	memcpy((char*)&result, data + 0x18, 2);
	return result;
}

ptr_block Entry::getIndex() {
	ptr_block result;
	memcpy((char*)&result, data + 0x1A, 2);
	return result;
}

int Entry::getSize() {
	int result;
	memcpy((char*)&result, data + 0x1C, 4);
	return result;
}

void Entry::setName(const char* name) {
	strcpy(data, name);
}

void Entry::setAttr(const unsigned char attr) {
	data[0x15] = attr;
}

void Entry::setTime(const short time) {
	memcpy(data + 0x16, (char*)&time, 2);
}

void Entry::setDate(const short date) {
	memcpy(data + 0x18, (char*)&date, 2);
}

void Entry::setIndex(const ptr_block index) {
	memcpy(data + 0x1A, (char*)&index, 2);
}

void Entry::setSize(const int size) {
	memcpy(data + 0x1C, (char*)&size, 4);
}

/** Bagian Date Time */
time_t Entry::getDateTime() {
	unsigned int datetime;
	memcpy((char*)&datetime, data + 0x16, 4);
	
	time_t rawtime;
	time(&rawtime);
	struct tm *result = localtime(&rawtime);
	
	result->tm_sec = datetime & 0x1F;
	result->tm_min = (datetime >> 5u) & 0x3F;
	result->tm_hour = (datetime >> 11u) & 0x1F;
	result->tm_mday = (datetime >> 16u) & 0x1F;
	result->tm_mon = (datetime >> 21u) & 0xF;
	result->tm_year = ((datetime >> 25u) & 0x7F) + 10;
	
	return mktime(result);
}

void Entry::setCurrentDateTime() {
	time_t now_t;
	time(&now_t);
	struct tm *now = localtime(&now_t);
	
	int sec = now->tm_sec;
	int min = now->tm_min;
	int hour = now->tm_hour;
	int day = now->tm_mday;
	int mon = now->tm_mon;
	int year = now->tm_year;
	
	int _time = (sec >> 1) | (min << 5) | (hour << 11);
	int _date = (day) | (mon << 5) | ((year - 10) << 9);
	
	memcpy(data + 0x16, (char*)&_time, 2);
	memcpy(data + 0x18, (char*)&_date, 2);
}

/** Menuliskan entry ke filesystem */
void Entry::write() {
	if (position != END_BLOCK) {
		filesystem.handle.seekp(BLOCK_SIZE * DATA_POOL_OFFSET + position * BLOCK_SIZE + offset * ENTRY_SIZE);
		filesystem.handle.write(data, ENTRY_SIZE);
	}
}
