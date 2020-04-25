#ifndef _LINUX_TIMERQUEUE_H
#define _LINUX_TIMERQUEUE_H

#include <linux/rbtree.h>
#include <linux/ktime.h>


struct timerqueue_node {
	struct rb_node node;	//������Ľڵ�
	ktime_t expires;        //�ýڵ�����hrtimer�ĵ���ʱ�䣬��hrtimer�ṹ�е�_softexpires���в�ͬ
};
/*next:���ڱ����������ȵ��ڵĶ�ʱ���ڵ㣬ʵ���Ͼ������������·��Ľڵ㣬����next�ֶΣ��������¼�����ʱ��
ϵͳ���ر��������������ֻҪȡ��next�ֶζ�Ӧ�Ľڵ���д�����*/
struct timerqueue_head {
	struct rb_root head;            //������ĸ��ڵ�
	struct timerqueue_node *next;	//�ú���������絽�ڵĽڵ㣬Ҳ���������µĽڵ�
};


extern bool timerqueue_add(struct timerqueue_head *head,
			   struct timerqueue_node *node);
extern bool timerqueue_del(struct timerqueue_head *head,
			   struct timerqueue_node *node);
extern struct timerqueue_node *timerqueue_iterate_next(
						struct timerqueue_node *node);

/**
 * timerqueue_getnext - Returns the timer with the earliest expiration time
 *
 * @head: head of timerqueue
 *
 * Returns a pointer to the timer node that has the
 * earliest expiration time.
 */
static inline
struct timerqueue_node *timerqueue_getnext(struct timerqueue_head *head)
{
	return head->next;
}

static inline void timerqueue_init(struct timerqueue_node *node)
{
	RB_CLEAR_NODE(&node->node);
}

static inline void timerqueue_init_head(struct timerqueue_head *head)
{
	head->head = RB_ROOT;
	head->next = NULL;
}
#endif /* _LINUX_TIMERQUEUE_H */
