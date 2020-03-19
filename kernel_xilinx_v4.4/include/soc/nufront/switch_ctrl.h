#ifndef __SWITCHCTRL_H__
#define __SWITCHCTRL_H__

#define My_netlink 26
#define U_PID 1
#define K_MSG 2

int switch_ctrl_init(void);
void switch_ctrl_exit(void);

void switch_pkt_receive(struct sk_buff* __skb) ;

#endif
