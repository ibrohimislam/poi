#include <stdexcept>

#include "volume.h"

using namespace std;

/* Global filesystem */
extern Volume filesystem;

/** konstruktor */
Volume::Volume(){
	time(&mount_time);
}
/** destruktor */
Volume::~Volume(){
	handle.close();
}

/** buat file *.poi baru */
void Volume::create(const char *filename){
	
	/* buka file dengan mode input-output, binary dan truncate (untuk membuat file baru) */
	handle.open(filename, fstream::in | fstream::out | fstream::binary | fstream::trunc);
	
	/* Bagian Volume Information */
	initVolumeInformation(filename);
	
	/* Bagian Allocation Table */
	initAllocationTable();
	
	/* Bagian Data Pool */
	initDataPool();
	
	handle.close();
}
/** inisialisasi Volume Information */
void Volume::initVolumeInformation(const char *filename) {
	/* buffer untuk menulis ke file */
	char buffer[BLOCK_SIZE];
	memset(buffer, 0, BLOCK_SIZE);
	
	/* Magic string "poi" */
	memcpy(buffer + 0x00, "poi!", 4);
	
	/* Nama Volume */
	if (strcmp(filename,"") == 0) {
		this->filename = "POI!";
	} else {
		this->filename = string(filename);
	}
	memcpy(buffer + 0x04, filename, strlen(filename));
	
	
	/* Kapasitas filesystem, dalam little endian */
	capacity = N_BLOCK;
	memcpy(buffer + 0x24, (char*)&capacity, 4);
	
	/* Jumlah blok yang belum terpakai, dalam little endian */
	available = N_BLOCK - 1;
	memcpy(buffer + 0x28, (char*)&available, 4);
	
	/* Indeks blok pertama yang bebas, dalam little endian */
	firstEmpty = 1;
	memcpy(buffer + 0x2C, (char*)&firstEmpty, 4);
	
	/* String "!iop" */
	memcpy(buffer + 0x1FC, "!iop", 4);
	
	handle.write(buffer, BLOCK_SIZE);
}
/** inisialisasi Allocation Table */
void Volume::initAllocationTable() {
	short buffer = 0xFFFF;
	
	/* Allocation Table untuk root */
	handle.write((char*)&buffer, sizeof(short));
	
	/* Allocation Table untuk lainnya */
	buffer = 0;
	for (int i = 1; i < N_BLOCK; i++) {
		handle.write((char*)&buffer, sizeof(short));
	}
}
/** inisialisasi Data Pool */
void Volume::initDataPool() {
	char buffer[BLOCK_SIZE];
	memset(buffer, 0, BLOCK_SIZE);
	
	for (int i = 0; i < N_BLOCK; i++) {
		handle.write(buffer, BLOCK_SIZE);
	}
}

/** baca file simple.fs */
void Volume::load(const char *filename){
	/* buka file dengan mode input-output, dan binary */
	handle.open(filename, fstream::in | fstream::out | fstream::binary);
	
	/* cek apakah file ada */
	if (!handle.is_open()){
		handle.close();
		throw runtime_error("File not found");
	}
	
	/* periksa Volume Information */
	readVolumeInformation();
	
	/* baca Allocation Table */
	readAllocationTable();
}
/** membaca Volume Information */
void Volume::readVolumeInformation() {
	char buffer[BLOCK_SIZE];
	handle.seekg(0);
	
	/* Baca keseluruhan Volume Information */
	handle.read(buffer, BLOCK_SIZE);
	
	/* cek magic string */
	if (string(buffer, 4) != "poi!") {
		handle.close();
		throw runtime_error("File is not a valid poi file");
	}
	
	/* baca capacity */
	memcpy((char*)&capacity, buffer + 0x24, 4);
	
	/* baca available */
	memcpy((char*)&available, buffer + 0x28, 4);
	
	/* baca firstEmpty */
	memcpy((char*)&firstEmpty, buffer + 0x2C, 4);
}
/** membaca Allocation Table */
void Volume::readAllocationTable() {
	char buffer[3];
	
	/* pindah posisi ke awal Allocation Table */
	handle.seekg(0x200);
	
	/* baca nilai nextBlock */
	for (int i = 0; i < N_BLOCK; i++) {
		handle.read(buffer, 2);
		memcpy((char*)&nextBlock[i], buffer, 2);
	}
}
/** menuliskan Volume Information */
void Volume::writeVolumeInformation() {
	handle.seekp(0x00);
	
	/* buffer untuk menulis ke file */
	char buffer[BLOCK_SIZE];
	memset(buffer, 0, BLOCK_SIZE);
	
	/* Magic string "poi" */
	memcpy(buffer + 0x00, "poi!", 4);
	
	/* Nama Volume */
	memcpy(buffer + 0x04, filename.c_str(), 32);
	
	/* Kapasitas filesystem, dalam little endian */
	memcpy(buffer + 0x24, (char*)&capacity, 4);
	
	/* Jumlah blok yang belum terpakai, dalam little endian */
	memcpy(buffer + 0x28, (char*)&available, 4);
	
	/* Indeks blok pertama yang bebas, dalam little endian */
	memcpy(buffer + 0x2C, (char*)&firstEmpty, 4);
	
	/* String "SFCC" */
	memcpy(buffer + 0x1FC, "!iop", 4);
	
	handle.write(buffer, BLOCK_SIZE);
}
/** menuliskan Allocation Table pada posisi tertentu */
void Volume::writeAllocationTable(ptr_block position) {
	handle.seekp(BLOCK_SIZE + sizeof(ptr_block) * position);
	handle.write((char*)&nextBlock[position], sizeof(ptr_block));
}
/** mengatur Allocation Table */
void Volume::setNextBlock(ptr_block position, ptr_block next) {
	nextBlock[position] = next;
	writeAllocationTable(position);
}
/** mendapatkan first Empty yang berikutnya */
ptr_block Volume::allocateBlock() {
	ptr_block result = firstEmpty;
	
	setNextBlock(result, END_BLOCK);
	
	while (nextBlock[firstEmpty] != 0x0000) {
		firstEmpty++;
	}
	available--;
	writeVolumeInformation();
	return result;
}
/** membebaskan blok */
void Volume::freeBlock(ptr_block position) {
	if (position == EMPTY_BLOCK) {
		return;
	}
	while (position != END_BLOCK) {
		ptr_block temp = nextBlock[position];
		setNextBlock(position, EMPTY_BLOCK);
		position = temp;
		available--;
	}
	writeVolumeInformation();
}
/** membaca isi block sebesar size kemudian menaruh hasilnya di buf */
int Volume::readBlock(ptr_block position, char *buffer, int size, int offset) {
	/* kalau sudah di END_BLOCK, return */
	if (position == END_BLOCK) {
		return 0;
	}
	/* kalau offset >= BLOCK_SIZE */
	if (offset >= BLOCK_SIZE) {
		return readBlock(nextBlock[position], buffer, size, offset - BLOCK_SIZE);
	}
	
	handle.seekg(BLOCK_SIZE * DATA_POOL_OFFSET + position * BLOCK_SIZE + offset);
	int size_now = size;
	/* cuma bisa baca sampai sebesar block size */
	if (offset + size_now > BLOCK_SIZE) {
		size_now = BLOCK_SIZE - offset;
	}
	handle.read(buffer, size_now);
	
	/* kalau size > block size, lanjutkan di nextBlock */
	if (offset + size > BLOCK_SIZE) {
		return size_now + readBlock(nextBlock[position], buffer + BLOCK_SIZE, offset + size - BLOCK_SIZE);
	}
	
	return size_now;
}

/** menuliskan isi buffer ke filesystem */
int Volume::writeBlock(ptr_block position, const char *buffer, int size, int offset) {
	/* kalau sudah di END_BLOCK, return */
	if (position == END_BLOCK) {
		return 0;
	}
	/* kalau offset >= BLOCK_SIZE */
	if (offset >= BLOCK_SIZE) {
		/* kalau nextBlock tidak ada, alokasikan */
		if (nextBlock[position] == END_BLOCK) {
			setNextBlock(position, allocateBlock());
		}
		return writeBlock(nextBlock[position], buffer, size, offset - BLOCK_SIZE);
	}
	
	handle.seekp(BLOCK_SIZE * DATA_POOL_OFFSET + position * BLOCK_SIZE + offset);
	int size_now = size;
	if (offset + size_now > BLOCK_SIZE) {
		size_now = BLOCK_SIZE - offset;
	}
	handle.write(buffer, size_now);
	
	/* kalau size > block size, lanjutkan di nextBlock */
	if (offset + size > BLOCK_SIZE) {
		/* kalau nextBlock tidak ada, alokasikan */
		if (nextBlock[position] == END_BLOCK) {
			setNextBlock(position, allocateBlock());
		}
		return size_now + writeBlock(nextBlock[position], buffer + BLOCK_SIZE, offset + size - BLOCK_SIZE);
	}
	
	return size_now;
}
