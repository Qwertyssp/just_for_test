#ifndef _CMD_QUEUE_H_
#define _CMD_QUEUE_H_

#include "queue.h"
#include "cmd_queue.h"
#include <pthread.h>
#include <limits.h>
#include <stdint.h>

#define CMD_IFLAG_ASYNC			0x01
#define CMD_IFLAG_FREEARG		0x02

union cmd_arg {
	int32_t 	cmda_int;
	uint32_t 	cmda_uint;
	void 		*cmda_ptr;
	const void	*cmda_cptr;
	int64_t		cmda_int64;
};

enum cmd_num {
	CMDNO_MIN	= -1,
	/* just for test */
	CMDNO_TEST1	= 0,
	CMDNO_TEST2	= 1,
	CMDNO_MAX	= 2
};

struct cmd_item {
	int 			cmdi_flags;
	int 			cmdi_sid;
	int 			cmdi_error;
	int 			cmdi_cmdno;
	union	cmd_arg		cmdi_arg;
	TAILQ_ENTRY(cmd_item)	cmdi_link;
};


TAILQ_HEAD(cmd_head, cmd_tiem);

struct cmd_queue {
	struct cmd_head		cmdq_head;
	pthread_mutex_t		cmdq_lock;
	pthread_cond_t		cmdq_cond;
};

int cmd_queue_init(struct cmd_queue *queue);

void cmd_queue_finish(struct cmd_queue *queue);

void cmd_enqueue(struct cmd_queue *queue, 
		struct cmd_item *cmd);

struct cmd_item * cmd_dequeue(struct cmd_queue *queue);

struct cmd_item *cmd_dequeue_timed(struct cmd_queue *queue);

int cmd_create(int sid, int flags, enum cmd_num cmdno, union cmd_arg arg, struct cmd_item **cmd0);

void cmd_destroy(struct cmd_item *cmd);

#endif /*_CMD_QUEUE_H_ */
