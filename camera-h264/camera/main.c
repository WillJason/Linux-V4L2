#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <malloc.h>
#include "video_capture.h"

struct camera *cam;
pthread_t mythread;

void capture_encode_thread(void) {
	int count = 1;
	for (;;) {
		printf("\n\n-->this is the %dth frame\n", count);
		if (count++ >= 100) // 采集100帧的数据
				{
			printf("------need to exit from thread------- \n");
			break;
		}

		fd_set fds;
		struct timeval tv;
		int r;

		FD_ZERO(&fds);
		FD_SET(cam->fd, &fds);

		/* Timeout. */
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		r = select(cam->fd + 1, &fds, NULL, NULL, &tv);

		if (-1 == r) {
			if (EINTR == errno)
				continue;

			errno_exit("select");
		}

		if (0 == r) {
			fprintf(stderr, "select timeout\n");
			exit(EXIT_FAILURE);
		}

		if (read_and_encode_frame(cam) != 1) {
			fprintf(stderr, "read_fram fail in thread\n");
			break;
		}
	}
}

int main(int argc, char **argv) {
	cam = (struct camera *) malloc(sizeof(struct camera));
	if (!cam) {
		printf("malloc camera failure!\n");
		exit(1);
	}
	cam->device_name = "/dev/video0";
	cam->buffers = NULL;
	cam->width = 640;
	cam->height = 480;
	cam->display_depth = 5; /* RGB24 */

	v4l2_init(cam);

	if (0 != pthread_create(&mythread, NULL, (void *) capture_encode_thread, NULL)) {
		fprintf(stderr, "thread create fail\n");
	}
	pthread_join(mythread, NULL);
	printf("-----------end program------------");
	v4l2_close(cam);

	return 0;
}
