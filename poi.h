#pragma once

// menggunakan versi 26
#define FUSE_USE_VERSION 26

#include <errno.h>
#include <fuse.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "const.h"
#include "volume.h"

using namespace std;


int poi_getattr(const char* path, struct stat* stbuf);

int poi_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);

int poi_mkdir(const char *path, mode_t mode);

int poi_open(const char* path, struct fuse_file_info* fi);

int poi_rmdir(const char *path);

int poi_rename(const char* path, const char* newpath);

int poi_unlink(const char *path);

int poi_mknod(const char *path, mode_t mode, dev_t dev);

int poi_truncate(const char *path, off_t newsize);

int poi_read(const char *path,char *buf,size_t size,off_t offset,struct fuse_file_info *fi);

int poi_write(const char *path, const char *buf, size_t size, off_t offset,struct fuse_file_info *fi);

int poi_link(const char *path, const char *newpath);

int poi_chmod (const char *path, mode_t newmode);