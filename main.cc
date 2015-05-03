#include <iostream>

#include "poi.h"
#include "volume.h"

using namespace std;

struct fuse_operations poi_op;

Volume filesystem;

void init_fuse() {
	poi_op.getattr	= poi_getattr;
	poi_op.readdir	= poi_readdir;
	poi_op.mkdir	= poi_mkdir;
	poi_op.mknod	= poi_mknod;
	poi_op.read	= poi_read;
	poi_op.rmdir 	= poi_rmdir;
	poi_op.unlink	= poi_unlink;
	poi_op.rename	= poi_rename;
	poi_op.write	= poi_write;
	poi_op.truncate= poi_truncate;
	poi_op.open 	= poi_open;
	poi_op.link	= poi_link;
	poi_op.chmod	= poi_chmod;
}

int main(int argc, char** argv){
	if (argc < 3) {
		printf("Usage: ./poi <mount folder> <filesystem.poi> [--new]\n");
		return 0;
	}
	
	// jika terdapat argumen --new, buat baru
	if (argc > 3 && string(argv[3]) == "--new") {
		filesystem.create(argv[2]);
	}
	
	filesystem.load(argv[2]);
	
	// inisialisasi fuse
	int fuse_argc = 2; 
	char* fuse_argv[2] = {argv[0], argv[1]};
	
	init_fuse();
	
	// serahkan ke fuse
	return fuse_main(fuse_argc, fuse_argv, &poi_op, NULL);
}
