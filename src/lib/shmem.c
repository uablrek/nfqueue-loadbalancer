/*
   SPDX-License-Identifier: MIT License
   Copyright (c) 2021 Nordix Foundation
*/

#include "shmem.h"
#include "die.h"
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>

int createSharedData(char const* name, void* data, size_t len)
{
	int fd = shm_open(name, O_RDWR|O_CREAT, 0600);
	if (fd < 0) return fd;
	int c = write(fd, data, len);
	if (c != len) return c;
	close(fd);
	return 0;
}
void createSharedDataOrDie(char const* name, void* data, size_t len)
{
	if (createSharedData(name, data, len) != 0) {
		die("createSharedData: %s\n", strerror(errno));
	}
}
void* mapSharedData(char const* name, size_t len, int mode)
{
	int fd = shm_open(name, mode, (mode == O_RDONLY)?0400:0600);
	if (fd < 0) return NULL;
	void* m = mmap(
		NULL, len,
		(mode == O_RDONLY)?PROT_READ:PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (m == MAP_FAILED) return NULL;
	return m;
}
void* mapSharedDataOrDie(char const* name, size_t len, int mode)
{
	void* m = mapSharedData(name, len, mode);
	if (m == NULL)
		die("FAILED mapSharedData: %s\n", name);
	return m;
}


