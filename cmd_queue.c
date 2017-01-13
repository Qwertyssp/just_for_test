/*-------------------------------------------------------------------------
    > File Name :	 cmd_queue.c
    > Author :		shang
    > Mail :		shangshipei@gmail.com 
    > Description :	shang 
    > Created Time :	2017年01月13日 星期五 15时06分56秒
    > Rev :		0.1
 ------------------------------------------------------------------------*/
#include "cmd_queue.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


static pthread_mutex_t 	_lock = PTHREAD_MUTEX_INITIALIZER;

#define LOCK(t)	 	pthread_mutex_lock(&(t)->cmdq_lock);
#define UNLOCK(t)	pthread_mutex_unlock(&(t)->cmdq_lock);

static void cmd_queue_cleanup(struct cmd_queue *q);

int 
cmd_queue_init(struct cmd_queue	*queue)
{
	int err = 0;

	pthread_mutex_lock(&_lock);

	TAILQ_INIT(&queue->cmdq_head);

	err = pthread_mutex_init(&queue->cmdq_lock, NULL);
	if (err != 0) {
		printf("%s pthread_mutex_init fail : sys %d \n", __func__, err);
	}

	err = pthread_cond_init(&queue->cmdq_cond, NULL);
	if (err != 0) {
		printf("%s pthread_cond_init fail : sys %d \n", __func__, err);
	}

	pthread_mutex_unlock(&_lock);
	return err;
}

void 
cmd_queue_finish(struct cmd_queue *queue)
{
	int err = 0;


	pthread_mutex_lock(&_lock);
	cmd_queue_cleanup(queue);

	err = pthread_mutex_destroy(&queue->cmdq_lock);
	if (err != 0) {
		printf("%s pthread_mutex_destroy fail : sys %d\n",__func__, err); 
	}

	err = pthread_cond_destroy(&queue->cmdq_cond);
	if (err != 0) {
		printf("%s pthread_cond_destroy fail : sys %d\n",__func__, err); 
	}

	memset(queue, 0, sizeof(*queue));

	pthread_mutex_unlock(&_lock);
}

static void
cmd_queue_cleanup(struct cmd_queue *q)
{
	struct cmd_item *cmd;

	while(!TAILQ_EMPTY(&q->cmdq_head)) {
		cmd = TAILQ_FIRST(&q->cmdq_head);
		cmd_destroy(cmd);
		TAILQ_REMOVE(&q->cmdq_head, cmd, cmdi_link);
	}
}

void cmd_enqueue(struct cmd_queue *q, struct cmd_item *cmd)
{
	int is_empty = 0;

	LOCK(q);

	if (TAILQ_EMPTY(&q->cmdq_head))
		is_empty = 1;

	TAILQ_INSERT_TAIL(&q->cmdq_head, cmd, cmdi_link);

	if (is_empty)
		pthread_cond_signal(&q->cmdq_cond);

	UNLOCK(q);
}


struct cmd_item *
cmd_dequeue(struct cmd_queue *q)
{
	int err;
	struct cmd_item *cmd;
	LOCK(q);

	while (TAILQ_EMPTY(&q->cmdq_head))
		pthread_cond_wait(&q->cmdq_cond, &q->cmdq_lock);

	if (TAILQ_EMPTY(&q->cmdq_head)) {
		cmd = NULL;
	} else {
		cmd = TAILQ_FIRST(&q->cmdq_head);
		TAILQ_REMOVE(&q->cmdq_head, cmd, cmdi_link);
	}

	UNLOCK(q);
	return cmd;
}

int 
cmd_create(int sid, int flags, enum cmd_num cmdno, union cmd_arg arg, struct cmd_item **cmd0)
{
	struct cmd_item *cmd;
	if (flags & CMD_IFLAG_ASYNC) {
		cmd = calloc(1, sizeof(*cmd));
		if (cmd == NULL) {
			return -1;
		}
		*cmd0 = cmd;
	} else {
		if (*cmd0 == NULL) {
			return -2;
		}
		cmd = *cmd0;
		memset(cmd, 0, sizeof(*cmd));
	}

	cmd->cmdi_flags = flags;
	cmd->cmdi_sid = sid;
	cmd->cmdi_error = 0;
	cmd->cmdi_cmdno = cmdno;
	cmd->cmdi_arg = arg;

	return 0;
}


void
cmd_destroy(struct cmd_item *cmd)
{
	if (cmd == NULL) {
		return ;
	}

	if (cmd->cmdi_flags & CMD_IFLAG_FREEARG){
		free(cmd->cmdi_arg.cmda_ptr);
		cmd->cmdi_arg.cmda_ptr = NULL;
	}

	if (cmd->cmdi_flags & CMD_IFLAG_ASYNC) {
		free(cmd);
	}
}
