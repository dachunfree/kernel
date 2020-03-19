#ifndef _CAP_AUTH_SMD_H
#define _CAP_AUTH_SMD_H

#define MAX_MSG_LEN         128

typedef struct _node_type
{
    int flag;
    int msg_len;
    char buff[MAX_MSG_LEN];
}QNodeType, *QNodeTypePtr;

typedef struct _common_msg_queue
{
    struct _common_msg_queue *next;
    struct _common_msg_queue *prev;

    QNodeType common_msg;
}TCommonMsgQueue, *TCommonMsgQueuePtr;


void init_queue(TCommonMsgQueuePtr queue);
int is_queue_empty(TCommonMsgQueuePtr queue);
void en_queue(TCommonMsgQueuePtr msg, TCommonMsgQueuePtr queue);
void de_queue(TCommonMsgQueuePtr msg);
void free_queue(TCommonMsgQueuePtr msg);

// the maximum length of INPUT-BUFFER is 128
int cap_auth_smd_get_msg_buf(char *buf);
int cap_auth_smd_set_msg_buf(const char *buf, int count);

#endif	/*_CAP_AUTH_SMD_H*/
