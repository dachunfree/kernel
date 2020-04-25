#ifndef _LINUX_TIMERQUEUE_H
#define _LINUX_TIMERQUEUE_H

#include <linux/rbtree.h>
#include <linux/ktime.h>


struct timerqueue_node {
	struct rb_node node;	//红黑树的节点
	ktime_t expires;        //该节点代表队hrtimer的到期时间，与hrtimer结构中的_softexpires稍有不同
};
/*next:用于保存树中最先到期的定时器节点，实际上就是树的最左下方的节点，有了next字段，当到期事件到来时，
系统不必遍历整个红黑树，只要取出next字段对应的节点进行处理即可*/
struct timerqueue_head {
	struct rb_root head;            //红黑树的根节点
	struct timerqueue_node *next;	//该红黑树中最早到期的节点，也就是最左下的节点
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
