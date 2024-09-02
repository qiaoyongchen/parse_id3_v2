#include <stdio.h>
#include "id3.h"
#include <fcntl.h>

int main(void) {
	char *k = FRAME_ID_KEY(AENC);
	char *v = FRAME_ID_VALUE(AENC);
	printf("%s - > %s\n", k, v);

	int fd = open("sn.mp3", O_RDONLY);
	if (fd == -1) {
		perror("Error opening file");
		return 1;
	}

	printf("open mp3 ok.\n");

	id3v2 *id3 = id3v2_create();

	printf("create id3 ok.\n");

	bool r = parse_id3v2(id3, fd);
	if (!r) {
		printf("parse failed\n");
	}

	id3v2_destory(id3);
	close(fd);

	return 0;
}
