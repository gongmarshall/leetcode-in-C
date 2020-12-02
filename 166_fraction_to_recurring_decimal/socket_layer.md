socket access using file descriptor through VFS layer inside the kernel.

User space

--------------------------------------------------------------------------------------------------------------------------------------------

Kernel

​										VFS

-----------------------------------------------------------------------------------------------------------------------------------------------

Socket Layer

--------------------------------------------------------------------------------------------------------------------------------------------

​						BSD socket

​			Protocol Families(sockets for various families who comply with BSD)

| AF_MAX     |
| ---------- |
| PF_INET    |
| ...        |
| AF_NETLINK |

​			

Further classification of protocol family-type & protocol for PF_INET (sock structure)

​	SOCK_STREAM(IPPROTO_TCP)	SOCK_DGRAM(IPPROTO_UDP) 	SOCK_RAW(IPPROTO_IP)

```c
/*net/socket.c*/
SYSCALL_DEFINE3(socket, int, family, int, type, int, protocol)
    struct socket *sock;
    sock_create(family, type, protocol, &sock);
		__sock_create(current->nsproxy->net_ns, family, type, protocol, res, 0);
			struct socket *sock;
			/*check family and type 0-AF_MAX and 0-SOCK_MAX*/
			sock = sock_alloc();
				/*allocates a new inode for given superblock*/
				inode = new_inode_pseudo(sock_mnt->mnt_sb);
				sock = SOCKET_I(inode);
				/*according to inode to get socket_alloc address, then get socket address*/
						return &container_of(inode, struct socket_alloc, vfs_inode)->socket;
				/*then init inode params*/
			sock->type = type;
			pf=rcu_dereference(net_families[family]);
			err = pf->create(net, sock, protocol, kern);
	sock_map_fd(sock, flags &(O_CLOEXEC|O_NONBLOCK));
**********************************************************************************
    /*对于上面的pf->create(),就PF_INET来说，在ipv4/af_inet.c中*/
    static const struct net_proto_family inet_family_ops = {
        .family = PF_INET,
        .create = inet_create,
        .owner = THIS_MODULE,
    };
inet_create(net, sock, protocol, 0)
    struct inet_protosw *answer;
    /*check protocol 0-IPPROTO_MAX*/
    sock->state = SS_UNCONNECTED;
	/*according to sock->type to get answer,the answer is inet_protossw struct. for ipv4, it initialized in af_inet*/
	sock->ops = answer->ops;
	answer_prot = anwer->prot;
	answer_no_check = answer->no_check;
	answer_flags = answer->flags
    /*for INET_STREAM, alloc tcp_sock*/
    sk = sk_alloc(net, PF_INET, GFP_KERNEL, answer_prot);
	inet= inet_sk(sk);
		return (struct inet_sock *)sk;
	sock_init_data(sock, sk); //init sk
		skb_queue_head_init(&sk->sk_receive_queue);
		skb_queue_head_init(&sk->sk_write_queue);
		skb_queue_head_init(&sk->skerror_queue);
		sk->sk_send_head = NULL;
		init_timer(&sk->sk_timer);
		sk->sk_allocation = GFP_KERNE;
		sk->sk_rcvbuf = sysctl_rmem_default;
		sk->sk_sndbuf = sysctl_wmem_default;
		sk->sk_state = 	TCP_CLOSE;
		...
	sk->sk_prot->init(sk)
   =tcp_v4_init_sock(sk)
            tcp_init_sock(sk);
			__skb_queue_head_init(&tp->out_of_order_queue);
			tcp_init_xmit_timers(sk);
				inet_csk_init_xmit_timer(sk, &tcp_write_timer, &tcp_delack_timer, &tcp_keepalive_timer);
			tcp_prequeue_init(tp);
***********************************************************************************
    /ipv4/af_inet.c/
    static struct inet_protosw inetsw_array[] = 
    {
        {
            .type = SOCK_STREAM,
            .protocol = IPPROTO_TCP,
            .prot = &tcp_prot,
            .ops = &inet_stream_ops,
            .no_check = 0,
            .flags = INET_PROTOSW_PERMANENT|INET_PROTOSW_ICSK,
        },
        {
            .typef = SOCK_DGRAM, 
            .protocol = IPPROTO_UDP,
            .prot = &udp_prot,
            .ops = &inet_dgram_ops,
            .no_check = UDP_CSUM_DEFAULT,
            .flags = INET_PROTOSW_PERMANENT,
            
        }
        {
            ...
        }
    }
**********************************************************************************
    /*ipv4/tcp_ipv4.c*/
    struct proto tcp_prot = {
        .name = "TCP",
        .close = tcp_close,
        .connect = tcp_v4_connect,
        ...
        .obj_size = sizeof(struct tcp_sock),
        ...
    }
```

End of tcp_v4_init_sock().

end of inet_create()

Unitil now, we have seen that various fields of structures sockets, sock, and tcp_opt are iniitialized in inet_create().

![](/home/marshall/Pictures/flow for socket system call.png)

对于socket()函数返回的是一个fd文件描述符，而对于这个文件描述符，可以通过inode可以获取到socket结构体，对于PF_INET/SOCK_STREAM socket->ops = inet_stream_ops: inet_bind(), inet_listen(), inet_accept(), inet_connect(), inet_sendmsg().

同样socket结构体中会有个sock结构体，对于PF_INET/SOCK_STREAM sk->prot = tcp_prot:

tcp_accept(), tcp_v4_connect(), tcp_sendmsg(), tcp_recvmsg(), tcp_setsockopts()...

BSD socket中有5种状态：

​	SS_FREE (sock is not yet allocated)

​	SS_UNCONNECTED (sock is allocated but is not yet connected)

​	SS_CONNECTING (sock is in the process of connecting)

​	SS_CONNECTED (already connected to sock)

​	SS_DISCONNECTING (in the process of disconnecting)

# 2. NETLINK SOCKETS

Netlink is a bidirectional communication method for transferring the data between kernel modules and user space processes. This functionality is provided using the standard socket APIS for user space processes and an internal kernel API for kernel modules.

The supported netlink families are as follows:

* NETLINK_ROUTE: It is used for queueing disciplines, to update the IPV4 routing table.

* NETLINK_SKIP:Reserved for ENskip.

* NETLINK_USERSOCK: Reserved for user mode socket protocols.

* NETLINK_FIREWALL: Receives packets sent by the IPv4 firewall code.

* NETLINK_TCPDIAG: TCP socket monitoring.

* NETLINK_NFLOG: Netfilter/iptables ULOG.

* NETLINK_ARPD: to update the arp table.

* NETLINK_ROUTE6: To update the IPV6 routing table.

  ```c
  static const struct net_proto_family netlink_family_ops =
  {
      .family = PF_NETLINK,
      .create = netlink_create,
      .owner = THIS_MODULE,
  }
  static struct proto netlink_proto =
  {
      .name = "NETLINK",
      .owner = THIS_MODULE,
      .obj_size = sizeof(struct netlink_sock),
  }
  /*net/netlink/af_netlink.c*/
  __init netlink_proto_init
  	proto_register(&netlink_proto, 0);
  	nl_table = kcalloc(MAX_LINKS, sizeof(*nl_table), GFP_KERNEL);
  ```

  

  The main purpose of the sock_register() function is to advertise the protocol handler's address family and have it linked into the socket module. (net_families[NPROTO])

* The socket() is a system call which is then resolved in the kernel. It calls the sys_socketcall(), which in turn calls sys_socket(); sys_socket() calls the sock_create() and based on the family in this case it is netlink; and sock_create() calls the netlink_create. This funciton creates the socket and initializes the operations of protocol performed with sokcet. It initializes the sock->ops to be &netlink_ops, where netlink_ops is a list of funciton pointers for various operation to be performed on netlink sockets.

  * structure data

    ```c
    struct netlink_table {
    	struct nl_portid_hash hash;
        struct hlist_head mc_list;
        struct listeners __rcu *listeners;
        unsigned int flags;
        unsigned int groups;
        struct mutex *cb_mutex;
        struct mudule *module;
        void (*bind)(int group);
        bool (*compare)(struct net *net, struct sock *sock);
        int registered;
    }
    struct rtnl_link {
        rtnl_doit_func doit;
        rtnl_dumpit_func dumpit;
        rtnl_calcit_func calcit;
    };
    
    ```

    nl_table is an array of pointers to netlink_table structures(socket linked list). Its size is set to MAX_LINKS(32). Each element of nl_table array represents a NETLINK protocol family--for example, NETLINK_ROUTE, NETLINK_FIREWALL, and so on.

