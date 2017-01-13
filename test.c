/*-------------------------------------------------------------------------
    > File Name :	 test.c
    > Author :		shang
    > Mail :		shangshipei@gmail.com 
    > Description :	shang 
    > Created Time :	2017年01月13日 星期五 16时28分18秒
    > Rev :		0.1
 ------------------------------------------------------------------------*/

#include "cmd_queue.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

struct cmd_queue QUEUE;

pthread_t id[3];

void *producor_thread1(void *arg)
{
	struct cmd_item *cmd;
	int i = 0;
	union cmd_arg cmd_arg;
	for (i  = 0; i < 10; i++) {
		printf("producor1 cread cmd %d\n", i);
		cmd_arg.cmda_int = i;
		cmd_create(1, CMD_IFLAG_ASYNC, CMDNO_TEST1, cmd_arg,  &cmd);
		cmd_enqueue(&QUEUE, cmd);
		sleep(1);
	}

}

void *producor_thread2(void *arg)
{
	struct cmd_item *cmd;
	int i = 0;
	union cmd_arg cmd_arg;
	for (i  = 10; i < 20; i++) {
		printf("producor2 cread cmd %d\n", i);
		cmd_arg.cmda_int = i;
		cmd_create(1, CMD_IFLAG_ASYNC, CMDNO_TEST1, cmd_arg,  &cmd);
		cmd_enqueue(&QUEUE, cmd);
	}

}

void *consumer_thread(void *arg)
{
	struct cmd_item *cmd;
	int end = 0;
	int i = 0;
	while (!end) {
		cmd = cmd_dequeue(&QUEUE);
		if (cmd != NULL) {
			printf("consumer recv cmd , cmd is : %d\n", cmd->cmdi_arg.cmda_int);
			cmd_destroy(cmd);
		}
	}
	printf("end of task\n");
}

int main(void)
{
	int err; 
	int i ;
	cmd_queue_init(&QUEUE);

	pthread_create(&id[0], NULL, producor_thread1, NULL); 
	pthread_create(&id[2], NULL, producor_thread2, NULL); 
	pthread_create(&id[1], NULL, consumer_thread, NULL);

	for (i = 0 ; i < 3; i++)
		pthread_join(id[i], NULL);

	cmd_queue_finish(&QUEUE);
}
