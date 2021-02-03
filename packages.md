[TOC]



# 1. net-wall

## 1.1 config_ap_mode_rule

这个函数的功能就是实现：

​	iptables -t nat -A PREROUTING -i br0 -d ![br0_addr] -p udp --dport 53 -j REDIRECT --to-port 53

### 1.1.1 reset_firewall

### 1.1.2 create the REDIRECT rule



![图1.1.2.1](D:\files\graphviz\hash_example.png)



首先分配一个ipt_entry的地址空间，将ipt_entry的target_offset和next_offset初始化为这个结构体的末尾。

接着用append_match(e, "udp", sizeof(ipt_udp)来在之前分配的ipt_entry结构体后再分配一个ipt_match结构体和一个ipt_udp结构体。而ipt_entry的target_offset和next_offset的值指向重现分配的整个结构体末尾。最后再设置udp结构体的值。

再完成一系列的配置后得到的ipt_entry如下图所示：

![](D:\files\graphviz\ap_mode_config.png)





然后调用iptc_append_entry("PREROUTING", e, &handle)将上图的ipt_entry结构体附加到nat的“PREROUTING”chain中.

接着是build_lan_http_redirect_rule，即远程管理功能将WAN口的端口如8443转发到WAN口的443端口或80端口。

如下图所示

![](D:\files\graphviz\create_lan_tcp_redirect_rule.png)



而对于lan ftp redirect rule只是将from和to的端口号换了其它都与上图一样。

net_dnat_rule会添加到dnatp2p_list中，所以遍历这个链表中的rule，将rule->entry附加到handle上。

最后再iptc_commit(&handle)。

## 1.2 init upnp rules

首先从/tmp/upnp_pmlist中提取出protocol ,inport ,extport ,dstip, 然后根据提取出来的信息进行规则添加。

### 1.2.1 generate dnat rules

#### firewall pass-through

![](D:\files\graphviz\dnat_firewall_pass_through.png)

这部分只关注目的IP地址和协议然后target为Accept。

#### DNAT

![](D:\files\graphviz\create_dnat_rule.png)

这里我关注的部分是端口号和协议，即我们只匹配对应的协议（tcp/udp）的外部端口号，然后将其转发为目的IP的内部端口。

### 1.2.2 generate dnat p2p rules

![](D:\files\graphviz\create_dnatp2p_rule.png)

对于dnatp2p与dnat基本上是一样的，只是nf_nat_range的flags有点不同。而对于传入的extports[0]和exports[1]不同也会使得匹配的不是某一个端口而是一个范围的端口来进行转发。而对于外部端口和内部端口不同则会使得外部端口转发到指定的内部端口。

## 1.3 port forwarding rules

端口转发使用的函数和upnp的是一样的都是将外部的端口范围转发到内部IP的端口范围。而对于TCP的内外部端口号都为1723时是特定为Cisco的GRE tunnels服务的。

## 1.4 init firewall

首先从datalib中获取wan_proto然后再进行validate ifaces, init default rule, init pmss rule, init notsyn rule, pppoe 和 pptp等初始化。

### 1.4.1 validate ifaces

获取WAN口和LAN口的信息

```c
struct ifconf {
	int ifc_len; /*size of buffer*/
	union {
		char *ifcu_buf;
		struct ifreq *ifcu_req;
	}ifc_ifcu;
}
#define ifc_buf ifc_ifcu.ifcu_buf
#define ifc_req ifc_ifcu.ifcu_req

struct ifreq {
	#define IFHWADDRLEN 6
	union {
		char ifrn_name[IFNAMSIZ]; /*if name, e.g. "en0"*/
	}ifr_ifrn;
	union {
		struct sockaddr ifru_addr;
		struct sockaddr ifru_dstaddr;
		struct sockaddr ifru_broadaddr;
		struct sockaddr ifru_netmask;
		struct sockaddr ifru_hwaddr;
		short ifru_flags;
		int ifru_ivalue;
		int ifru_mtu;
		struct ifmap ifru_map;
		char ifru_slave[IFNAMSIZ]; /*Just fits the size*/
		char ifru_newname[IFNAMSIZ];
		void * ifru_data;
		sturct if_settings ifru_settings;
	}ifr_ifru;
}

#define ifr_name ifr_ifrn.ifrn_name /*interface name*/
#define ifr_hwaddr ifr_ifrm.ifrn_hwaddr /*MAC address*/
#define ifr_addr ifr_ifru.ifru_addr /*address*/
#define ifr_dstaddr ifr_ifru.ifru_dstaddr /* other end of p-p lnk*/
#define ifr_broadaddr ifr_ifru.ifru_broadaddr /* broadcast address */
#define ifr_netmask ifr_ifru.ifru_netmask /* interface net mask */
#define ifr_flags ifr_ifru.ifru_flags /* flags */
#define ifr_metric ifr_ifru.ifru_ivalue /* metric */
#define ifr_mtu ifr_ifru.ifru_mtu /* mtu */
#define ifr_map ifr_ifru.ifru_data /* device map */
#define ifr_slave ifr_ifru.ifru_slave /* slave device */
#define ifr_data ifr_ifru.ifru_data /* for use by interface */
#define ifr_ifindex ifr_ifru.ifru_ivalue /* interface index */
#define ifr_bandwidth ifr_ifru.ifru_ivalue /* link bandwidth */
#define ifr_qlen ifr_ifru.ifru_ivalue /* Queue length */
#define ifr_newname ifr_ifru.ifru_newname /* newname */
#defien ifr_settings ifr_ifru.ifru_settings /* Device/proto settings */
```

fd = socket(AF_INET, SOCK_DGRAM, 0);

ioctrl(fd, SIOCGIADDR, &ifr);会将获取接口地址，并用ifr指向其地址。也可以根据ioctrl中的flag来获取广播地址或子网掩码等。

最后配置/proc/sys/net/ipv4/ip_forward(ip_dynaddr, icmp_echo_ignore_broadcasts, tcp_ecn...)和/proc/sys/net/ipv4/conf/(interface)/accept_source_route(accept_redirects, log_martians...)

### 1.4.2 初始化默认规则

对于默认规则是将匹配IP_CT_ESTABLISHED和IP_CT_RELATED 的状态掩码（见ip_conntrack_info）并ACCEPT 。

对于标准规则只是分配一个ipt_entry然后再其后append一个target。

### 1.4.3 To be continued

##################################################

## 1.5 初始化netfilter（To be continued）

#######################################################

## 1.6 init trigger rules

该功能在内部输出的某个端口在预先设置的内部端口范围内时触发某个指定端口来向某个server进行访问。

要正确设置 port trigger，您的电脑必须具备可触发路由器开启指定端口的应用程序。 以下图例为 port triggering 连接至 IRC 服务器。
![img](http://kmpic.asus.com/images/2014/07/29/ef5dca68-650f-4eb0-a8e1-2dbb76d57997.jpg)


![img](http://kmpic.asus.com/images/2014/07/29/fec202f6-4ca0-4933-84d5-8d14c5d9638a.jpg)

 

当连接到 IRC 服务器时，客户端（图标中的PC 2）使用端口范围 6660-7000 向外连接。

IRC 服务器验证用户名称，接着使用端口 113 与客户端电脑建立新连接。

**"Non-Working Port Trigger"图标显示了失败的 IRC 连接**。

\- 在此情境中，路由器因为无法决定哪一台电脑正在请求访问 IRC，因此放弃连接。

**「\**Working Port Trigger。\**」图标显示成功使用端口范围 6660-7000 向外连接。**，**使用端口 113 传入连接**。

\- 在此情境中，路由器的端口触发设定指定端口 113 来接收传入资料。 譬如，电脑 2 使用端口 6767 向外连接时，会暂时使用端口 113 接收所有传入连接。

 net-wall中的实现方式如下：

![](D:\files\graphviz\create_trigger_rule.png)

对于ipt_match后udp也可以是tcp,通过目的源地址ip和目的端口来匹配然后再target中进行触发端口设置。

而在kmod_trigger中也有对trigger内核驱动进行设置。

```c


create_trigger_del_flag_proc

​	proc_create("trigger_del_flag",0, NULL, &trigger_del_flag_fops)

​		static const struct file_operations trigger_del_flag_fops = {
			.open =trigger_del_flag_proc_open,
    		.read = seq_read,
    		.write = trigger_del_flag_write,
    		.llseek = seq_lseek,
    		.release = single_release,
};
trigger_del_flag_proc_open
    single_open
    	...
    这个主要是打开缓存文件
  return ipt_register_target(&ipt_trigger_reg);
	ipt_trigger_reg = {
        	.name = "TRIGGER",
        	.target = ipt_trigger_target,
        	.checkentry = ipt_trigger_check,
        	...
    }
	ipt_trigger_target
         (ipt_trigger_info)* info= xt_action_param *par->targinfo
		由于我们在应用程序中设置的type为IPT_TRIGGER_OUT(info->type),所以调用
       	trigger_out
        	将info中的信息配置到ipt_trigger结构体中并添加到一个全局链表中然后启动定时器。
    这样即完成了一个自己扩展的target。    
        
        	
        
        
    	
```

## 1.7 init block site rules

将value(baidu youdao ...)储存到sites[]中。

![](D:\files\graphviz\create_blk_site_rule.png)

如上图所示，防火墙首先会匹配多个端口和网站关键字，然后排除TrustIP，最后在target中将这些包reject。当然这些match为扩展match，所以应该在内核中注册这些match。

同样对于reject target后的数据是ipt_netgear_reject_info,也需要register target。

Netfilter IPv4 Hooks

* NF_INET_PRE_ROUTING
  * Incoming packets pass this hook in ip_rcv() before routing
* NF_INET_LOCAL_IN
  * All incoming packets addressed to the local host pass this hook in ip_local_deliver()
* NF_INET_FORWARD
  * All incoming packets not addressed to the local host pass this hook in ip_forward().
* NF_INET_LOCAL_OUT
  * All outgoing packets created by this local computer pass thsi hook in ip_build_and_send_pkt()
* NF_INET_POST_ROUTING
  * All outgoing packets(forwarded or locally created) will pass this hook in ip_finish_output()

## 1.8 iptables

* iptables的匹配（matches）

  -p --protocol 检测特定协议，必须是/etc/protocols里面定义

  -s --src, --source 匹配源地址 可单个主机，也可地址加子网掩码匹配整个网络

  -d --dst --destination 匹配目的地址

  -i --in-interface 以包进入所使用的网络接口来匹配包，只能用于INPUT, FORWARD和PREROUTING三个链

  -o, --out-interface 以包离开本地所使用的网络接口来匹配包

  -f --fragment 用来匹配一个被分片的包的第二片或及以后的部分。

  --sport --source-port 源端口来匹配包，不指定此项，则暗示所有端口

  --source-port 22:80（80：22） 22到80的所有端口，可以省略第一个号，默认为0，端口前的感叹号表示取反如--source-port !22:80表示菜单22到80端口之外的所有端口

  --dport --destination-port 匹配目的端口

  --tcp-flags 匹配指定的TCP标记 SYN, FIN, ACK SYN RST, URG, PSH 第一个参数表示要匹配的标记，第二个参数指定再第一个列表中出现的且必须被设为1的标记，其它标记必须为0

  ​	iptables -p tcp --tcp-flags SYN,FIN,ACK SYN表示匹配那些SYN标记被设置而FIN和ACK标记没有设置的包，注意各标记之间只有一个逗号而没有空格。

  --syn 匹配那些SYN标记被设置而ACK和RST标记没有被设置的包

  --tco-option

  根据选项匹配包

  * UDP匹配器

  * ICMP匹配器

    --icmp-type 根据ICMP类型匹配包

  * 显示匹配
    * 地址匹配选项

      --src-type

      iptables -A INPUT -m addrtype --src-type UNICAST

      这个选项主要用来匹配报文的源地址类型 BROADCAST, MULTICAST

      --dst-type

    * comment match

      这个匹配器主要用来再内核和规则集里增加注释，这样就能够让人更加容易知道规则的作用并已于调试。并不是一个真正的匹配器，需要-m comment加载

      --comment

      iptables -A INPUT -m comment --comment “A comment”

      --comment 选项用来添加实际的注释内容，每个注释最大的长度为256字节

      

    * contrack匹配器

      --ctstate

      iptables -A INPUT -p tcp -m conntrack --ctstate RELATED

      这个match用来根据连接跟踪的状态匹配报文状态，它和以前的状态匹配功能是一样的，可以使用的状态如下：

      INVALID, ESTABLESHD, NEW, RELATED, SNAT, DNAT

      一次可以匹配多个状态，但这些状态要用逗号隔开：-m conntrack --ctstate ESTABLISHED, RELATED.

      --ctproto

      iptables -A INPUT -p tcp -m conntrack --ctproto TCP

      这个match匹配协议，它和--protocol用法一样

      --ctorigsrc

      iptables -A INPUT -p tcp -m conntrack --ctorigsrc 192.168.0.0/24

      这个match匹配正向连接的源地址，用法和源地址匹配一样

      --ctorigdst

      iptables -A INPUT -p tcp  -m conntrackf --ctorigdst 192.168.0.0/24

      --ctreplsrc

      iptables -A INPUT -p tcp -m conntrack --ctreplsrc 192.168.0.0/24

      这个match匹配反向连接的源地址

      --ctrepldst

      --ctstatus

      iptables -A INPUT -p tcp -m conntrack --ctstatus RELATED

      这个匹配连接的状态，可用值如下：

      ​	NONE: 该连接没有状态

      ​	EXPECTED:这个连接是被expextation创建的期望连接

      ​	SEEN-REPLY:这个连接已经得到方向报文，但是还没有保障。

      ​	ASSURED:这个连接已经有保障了，除非它的超时或者一种关闭，这个连接才会被删除。

      --ctexpire

      iptables -A INPUT -p tcp -m conntrack --ctexpire 100:150

      这个match用来匹配连接的连接跟踪系统里面的超时实际，单位s

    * DSCP匹配器

      --dscp

      iptables -A INPUT -p tcp -m dscp --dscp 32

      这个选项指定匹配的数值，我们可以输入10进制或者16进制。

      --dscp--class

      iptables -A INPUT -p tcp -m dscp --dscp-class BE

      用来基于报文的diffServ类型来匹配报文，这个值可以是RFC里面定义的BE, EF AFxx,CSxx

    * ECN匹配器

      ecn match用来基于TCP的ECN字段匹配报文。

      iptables -A INPUT -p tcp -m ecn --ecn-tcp-cwr

      --ecn-tcp-ece

      --ecn-ip-ect

    * Hashlimit匹配器

      它为每个目的地址，源地址，目的端口以及源端口都建立hash表项。例如你可以设置每一个IP地址每秒钟最多接收1000个报文，或者你可以设置每种业务报文每秒最多200个。

      每一条规则创建一个单独的hashtable,每个hashtable都有最大尺寸以及最大数量的桶。hash表里是一个或多个源地址，目的地址，源/目的端口的组合。

      --hashlimit

      iptables -A INPUT -p tcp --dst 192.168.0.3 -m hashlimit --hashlimit 1000/sec --hashlimit-mode dstip, dstport --hashlimit-name hosts

      --hashlimit 指定每个桶的限度，在这个例子中是1000，这个数值可以是/sec ,/minute, /hour或者/day.默认是/sec

      --hashlimit-mode

      --hashlimit-mode选项指定了那些值用来作为hash的key.这个例子里面，我们用dstip,所以192.168.0.0/16网络里面的每台主机都会被限制1000/sec，我们可以在源/目的地址，源/目的端口里面选择一个或多个

      --hashlimit-name

      这个选项指定了具体hash的名称，可以在/proc/net/ipt_hashlimit里面看到

      --hashlimit-burst

      iptables -A INPUT -p tcp --dst 192.168.0.3 -m hashlimit --hashlimit 1000 --hashlimit-mode dstip,dstport --hashlimit-name hosts --hashlimit-burst 2000

      这个选项和--limit-burst一样设置桶的最大尺寸，每个桶都会有一个突发大小，令牌桶的工作原理参考limit match

      --hashlimit-htable-size

      这个选项设置最多可用的桶的数目

      --hashlimit-htable-max

      设定hash表项的最大值，包含所有连接，当然包含那些暂时没有激活的连接

      --hashlimit-htable-gcinterval

      垃圾回收多久运行，太小占系统资源，太大会有大量无用连接占用hash表项。

      --hashlimit-htableg-expire

      这个值设定一个空闲的hash表项会超时，假如一个桶长于这个时间没有使用，它就会超时，接着下一次垃圾回收就会把它从hash表项中删除

    * helper匹配器

      这个match是基于连接的helper模块名称来匹配报文的。我们看下FTP的会话，首先打开控制绘画，接着在控制会话里面协商数据会话的参数，ip_conntrack_ftp helper模块就会发现这个协商数据，接着创建一个关联的连接到控制连接上，这个一个报文进来我们就能够知道它管理到哪一个协议。

      --helper

      iptables -A INPUT -p tcp -m helper --helper ftp-21

      --heper选项的参数是一个字符串，它告诉match哪一个helper被匹配，我们可以指定在哪个端口上面这个期望被创建

    * IP范围匹配器

      IP范围匹配用来匹配一整段的IP,和--source以及destination匹配工作原理一样，只是IP范围更加灵活，它可以匹配整段的地址。

      --src-range

      iptalbes -A INPUT -p tcp -m iprange --src-range 192.168.1.13-192.168.2.19

      匹配从192.168.1.13到192.168.2.19之内的所有地址，也可以！取反

      --dst-range

    * 长度匹配器

      --length

      iptalbes -A INPUT -p tcp -m length --length 1400:1500

    * 限速匹配器

      这个匹配必须由-m limit指定才能使用

      --limit

      iptables -A INPUT -m limit --limit 3/hour

      一小时最多匹配3次

      --limit-burst

    * MAC地址匹配器

      iptables -A INPUT -m mac --mac-source 00:00:00:00:00:01

      基于包的MAC源地址匹配包

    * Mark匹配器

      --mark

    * 多端口匹配器

      --source-port

      iptables -A INPUT -p tcp -m multiport --source-port 22,53,80,110

      最多能够匹配15个不连续的源端口，它使用的前提是通过-p tcp或者-p udp指定协议，其实它就是增强版的--source-port功能

      --destination-port

      --port

      iptables -A INPUT -p tcp -m multiport --port 22,53,80,110

      同端口多端口匹配，意思是它匹配的是那种源端口和目的端口是同一个端口的包，如80到80，110到110

    * 报文类型匹配器

      --pkt-type

      iptables -A OUTPUT -m pkttype --pky-type unicast

      --pkt-type告诉报文类型匹配器什么样的报文将被匹配，它的值可能是unicast, broadcast or multicast可取反

    * 状态匹配器

      iptables -A INPUT -m state --state RELATED, ESTABLISHED

      指定要匹配包的状态，INVALID, ESTABLISHED, NEW和RELATED。

      

  * iptables的动作（target）和跳转（jumps）

    DNAT target

    DNAT target 用来做目的地址转换，换句话讲就是重写报文的目的地址。DNAT只在NAT表里面的PREROUTING和OUTPUT链上有效。

    --to-destination

    iptables -t nat -A PREROUTING -p tcp -d 15.45.23.67 --dport 80 -j DNAT --to-destination 192.168.1.1-192.168.1.10

    指定要写入IP头的地址，这也是包要被转发到的地方。上面的例子就是要把发往地址15.45.23.67的包都转发到一段LAN使用的私有地址中，即192.168.1.1到192.168.1.10.如前所述，在这种情况下，每个流都会被随机分配一个要转发到的地址，但同一个流总是使用同一个地址。我们也可以只指定一个IP地址作为参数，这样所有的包都被转发到同一台机子。我们还可以在地址后指定一个或一个范围端口。比如：--to-destination 192.168.1.1:80或--to-destination 192.168.1.1:80-100. SNAT的语法和这个target的一样，只是目的不同罢了。要注意只有先用--protocol指定了TCP或UDP协议，才能使用端口。

    eg:我们想通过internet发布我们的web服务，但是我们只有一个IP地址，并且HTTP服务器放置在我们的内网中，这条防火墙就有了唯一的IP地址，我们称之为$INET_IP，而HTTP服务器有一个内网地址，我们称之为$HTTP_IP。最后防火墙郑家一个内网地址$LAN_IP.那么我们在NAT表的PREROUTING链里面要做的第一件事情如下：

    iptables -t nat -A PREROUTING --dst $INET_IP -p tcp --dport 80 -j DNAT --to-destination $HTTP_IP

    所有从Internet上面访问本机80端口的报文都会被转发到内部的HTTP服务器上面。假如你通过Internet做上面的测试，一切工作非常完美。假如你通过同一内网其它主机访问这台web服务器呢？那么它就工作不正常了。这其实是路由的问题。我们把外网访问我们服务器的那台机器的IP地址记为$EXT_BOX.

    1. 包从地址为$EXT_BOX的机器出发，去往地址为$INET_IP的机子
    2. 报文到达防火墙
    3. 防火墙这个报文做地址转换，而且这个报文还要通过其它很多链。
    4. 报文离开防火墙并被转发到$HTTP_IP
    5. 报文到达HTTP服务器，并且HTTP服务器对这个报文做出应答，应答包需要再次通过防火墙。当然，这要求防火墙作为HTTP到达$EXT_BOX的网关。一般情况下，防火墙就是HTTP服务器的缺省网关。
    6. 防火墙复原返回报文，这样这个报文看起来就像是防火墙自己发出的。
    7. 返回报文和往常一样返回$EXT_BOX

    现在看看假如和HTTP服务器一个网络的主机访问会出现什么问题。这儿我们定义访问的主机地址为$LAN_BOX, 其它不变。

    1. 报文离开￥LAN_BOX到到$INET_IP
    2. 报文到达防火墙
    3. 报文做DNAT以及所需的操作，但是这个报文没有做SNAT,所以源地址仍然是一样的。
    4. 报文离开防火墙并且达到HTTP服务器。
    5. HTTP服务器尝试回应这个包，查找路由表得知是同一个网络的主机，所有就直接给这台主机发送回应报文。
    6. 回复包到达客户机，但它会困惑，因为这个包不是来自它访问的那台机子。这样，它就会把这个包扔掉去等待“真正”的回复包

    如果对那些进入防火墙而且是去往地址为$HTTP_IP、端口是80的包做SNAT操作，那么那些包好像是从$LAN_IP来的了，也就是这些包的源地址被改为$LAN_IP了。这样，HTTP服务器就会把回复包发给防火墙，而防火墙会再对包做un-DNAT操作，并把包发送给客户机

    iptables -t nat -A POSTROUTING -p tcp --dst $HTTP_IP --dport 80 -j SNAT --to-source $LAN_IP

    对于防火墙自己访问HTTP服务器，还是不行，我们要再nat表的OUTPUT链中添加下面的规则：

    iptables -t nat -A OUTPUT --dst $INET_IP -p tcp --dport 80 -j DNAT --to-destination $HTTP_IP

    和HTTP服务器不在同一个网的机子能正常访问

    和它在一个内网的机子也可以正常访问

    防火墙本身也能访问

    * DROP target

      顾名思义，如果包符合条件，这个target就会把它丢弃。

      REJECT target 在丢弃包的同时还返回一个错误信息。

    * DSCP target

      这个target修改报文里面的DSCP mark,DSCP target能够设吹TCP报文的DSCP只为任意你想要的。DSCP是一种把服务分类的方法，并且基于这些分类方法在通过路由器的时候给予不同的优先级。通过这个方法，你可以对交换性强的TCP会话一个交互性强的连接。对于一些优先级低的连接，例如SMTP,你可以通过一些慢速网络传递。

      --set-dscp

      iptables -t mangle -A FORWARD -p tcp --dport 80 -j DSCP -set-dscp 1

    * MASQUERADE target 

      这个target和SNAT target的作用是一样的，区别就是它不需要指定 --to-source. MASQUERADE 是专门涉及用于那些动态获取IP地址的连接的，比如拨号上网、DHCP连接等。如果有固定IP还是用SNAT target

      伪装一个连接意味着，我们自动获取网络接口的IP地址，而不使用--to-source. 当接口停用时，MASQUERADE不会记住任何连接，这在我们kill掉接口时是有很大的好处。如果我们使用的是SNAT target，连接跟踪的数据是被保留下来的，而且时间要好几天，着可是要占用很多连接跟踪的内存的。和SNAT一样只能用于nat表的POSTROUTING链

      --to-ports

      iptables -t nat -A POSTROUTING -p TCP -j MASQUERADE --to-ports 1024-31000

      在指定TCP或UDP的前提下，设置外出包能使用的端口。

    * NETMAP target

      NETMAP是SNAT和DNAT的新实现，在转化过程里面做的是1：1的映射，即一个私有地址映射到一个公网地址，它们的主机地址部分不做更改。

      iptables -t mangle -A PREROUTING -s 192.168.1.0/24 -j NETMAP --to 10.5.6.0/24

      192.168.1.x会直接变成10.5.6.x。

    * REDIRECT target

      在防火墙所在的机子内部转发包或到另一个端口。比如，我们可以把所有去往端口HTTP的包REDIRECT到HTTP proxy，当然着都发生在我们自己主机内部。它只能用在nat表的PREROUTING、OUTPUT链和被它们调用的自定义链里。

      --to-ports

      iptables -t nat -A PREROUTING -p tcp --dport 80 -j REDIRECT --to-ports 8080

      在指定TCP或UDP协议的前提下，定义目的端口，方法如下：

      1. 不使用这个选项，目的端口不会被改变
      2. 指定一个端口，如--to-ports 8080
      3. 指定端口范围，如--to-ports 8080-8090

    * REJECT target

      --reject-with

      iptables -A FORWARD -p TCP --dorpt 22 -j REJECT --reject-with tcp-reset

      告诉REJECT target 应向发送者返回什么样的信息。一旦满足了设定的条件，就要发送相应的信息，然后再像DROP一样无情的抛弃。

      可用的信息类型：

      	1. icmp-net-unreachable
       	2. icmp-host-unreachable
       	3. icmp-port-unreachable
       	4. icmp-proto-unreachable
       	5. icmp-net-prohibited
       	6. icmp-host-prohibited.

    * RETURN target

      它使包返回上一层，顺序是：子链->父链->缺省的策略。就是若在子链中遇到了RETURN，则返回父链的下一条规则继续进行条件的比较。若是在父链中遇到RETURN，就要被缺省策略（一般是ACCEPT或DROP）。

    * SNAT target

      这个target是用来源网络地址转换的，就是重写包的源IP地址。当我们有几个机子共享一个Internet连接时，就能用到它了。先在内核里打开ip转发功能，然后再写一个SNAT规则，就可以把所有从本地网络出去的包的源地址改为Internet连接的地址了。

      --to-source

      iptables -t nat -A POSTROUTING -p tcp -o eth- -j SNAT --to-source 194.236.50.155-194.236.50.160:1024-32000

      指定源地址和端口，有以下几种方式：

      1. 单独的地址。
      2. 一段连续的地址，用连字符分隔，如194.236.50-194.236.50.160，这样可以实现负载平衡。每个流被随机分配一个IP，但对于同一个流使用的时同一个IP
      3. 在指定-p tcp或udp的前提下，可以指定源端口的范围如194.236.50.155：1024-32000.

  

# 2. VPN

	## 2.1 生成证书

## 2.2 配置防火墙

​	配置的关键是允许tun tap连入，对从OpenVPN客户端来公司局域网的流量做NAT

# 3. VLAN

## 3.1 By bridge group--enable VLAN ID(XR500)

​	![image-20200820142331892](C:\Users\yuanshuai.gong\AppData\Roaming\Typora\typora-user-images\image-20200820142331892.png)

对于桥接时的:

​	如果使能vlan,则如上图的STB所示：使能vlan的端口同时支持lan和vlan即有vlan tag时，其IP应是WAN端IP，否则其IP应为LAN端IP.

​	而如果没有使能vlan, 其IP应为WAN端IP即正常的桥接模式。

其方式是通过swconfig来实现,而其它orbi系列则是通过qca-ssdk-shell完成。

我的理解是swconfig配置的vlan是在switch上的也就是第二层，而用vconfig来配置的vlan是在第三层。也就是桥接时只需要在switch上设置vlan即可。

start_stage0 start

​	op_set_induced_configs  if these configs are set, then :

​		i_wlg_br="brwan"

​		iwla_br="brwan"

​		i_wlg_guest_br="brwan"

​		i_wlg_guest_br="brwan"

​	op_create_brs_and_vifs

​		op_del_all_brs_vifs

​		iptv_create_brs_and_vifs

​			**ifconfig eth1 hw ether lanmac**

​			**ifconfig eth0 hw ether wanmac**

​			**ip link set dev eth1 name ethlan**

​			**ip link set dev eth0 name ethwan**

​			**brctl addif br0 ehtlan**

​			**ifconfig br0 hw ether lanmac**

​			**brctl addif brwan ethwan**

​			sw_configvlan "iptv" iptv_mask

​				sw_configvlan_iptv iptv_mask

​					ports1=“6”， ports2="0 5", ports3="5t"(依次为LAN, WAN , VLAN)

​					

​					

​				



​			

​			

## 3.2 By VLAN group

files path: /lib/cfgmgr/cfgmgr.sh ./opmode.sh

opmode

start_stage0 start

 1. op_set_induced_configs

     1. vlan # $1:enable, $2: name, $3:vid, $4: pri, $5: wports, $6: wlports

        Internet: i_wlg_br="brwan"

        other_name:  i_wlg_br="br20" (vid=20)

 2. op_create_brs_and_vifs

     1. op_del_all_brs_vifs

        ifconfig athx.. ethx.. down

        brctl delif br\[0][wan] athx ethx

        vconfig rem eth*(!=eth0, eth1)

        ifconfig br0 brwan down

        brctl delbr br0 brwan

    2. vlan_create_brs_and_vifs

       1. op_create_br0_brwan

          1. br_create br0 br_create brwan

             **brctl addbr br0|brwan**

             **brctl setfd br0|brwan 0**

             **brctl stp br0|brwan 0**

       2. **ifconfig bro|brwan hw ether mac**

       3. **brctl addif br0 eth1**

       4. **ifconfig eth0 br0 brwan up**

       5. sw_configvlan "vlan" "start"

          1. sw_configvlan_vlan start

             1. sw_print_ssdk_cmds_start

                > ***/usr/sbin/ssdk_sh vlan entry flush >/tmp/ssdk.sh***

       6. config get vlan_tag_i

          Internet: i_vid = $3, i_pri=$4f

          lan_vid=$(create_lan_vid_dif_wan $3)

          ​	lan_vid=`expr 4095 - $1`	

          vlan_create_br_and_vif $3 $4 $lan_vid

          ​	vconfig add eth1 lan_vid && ifconfig eth1.lan_vid up 

          ​	vconfig add eht0 wan_vid && ifconfig eth0.wan_vid up

          ​	brctl addif br(wan_vid) eth1.lan_vid

          ​	brctl addif br(wan_vid) eh0.wan_vid

           	//set priority and ifconfig br(wan_vid) hw ether mac

          ​	brctl stp br(wan_vid) on

          ​	ifconfig br(wan_vid) up

          sw_configvlan "vlan" "add" "vlan" "vid" "0" "pri"

          	1. ports="0t 5t"
           	2. sw_print_ssdk_cmds_add_vlan_for_wan "vid" >> ssdk_cmds_file
                	1. ***ssdk_sh vlan entry create vid***
                	2. ***ssdk_sh vlan entry append vid vid 0,5 0,5 null null no no***

          3. sw_print_ssdk_cmds_set_ports_pri "0t 5t" "pri" >> ssdk_cmds_file

             1.  do nothing

                

          sw_configvlan "vlan" "add" "lan" "$lan_vid" "wports" "pri"

           	1. ports = "0t 4" ports1="0t 4t"
                	2. sw_print_ssdk_cmds_add_vlan_for_lan "ports" "vid" >>ssdk_cmds_file
                        	1. ***ssdk_sh vlan entry create 3096(vid)*** 
                        	2. ***ssdk_sh vlan member add 3096(vid)  0 tagged***
                        	3. ***vlan member add 3096(vid) 4 untagged***
                   	3. sw_print_ssdk_cmds_set_ports_default_cvid "0t 4" "vid" >>ssdk_cmds_file
                            	1. ***ssdk_sh portvlan defaultcvid set 4 3096***
                            	4. sw_print_ssdk_cmds_set_ports_pri "0t 4" "pri" >>ssdk_cmds_file
                                	1. ***ssdk_sh qos ptDefaultCpri set 4 1***

          sw_configvlan "vlan" "add" "lan" "$lanvid" $(($used_wports^0x7)) "0"

           1. vid = lanvid, mask =(used_wports^0x7)&0x1(`*there is 0xf in XR500*`) pri=0, ports1="0t", ports="0t"

           2. ports="0t 4" ports1="0t 4t"

           3. sw_tmpconf_adjust_vlan 0 lanvid "0t 4"

           4. sw_print_cmds_add_vlan_forlan "0t 4" "lanvid" >>$ssdk_cmds_file

               1. ***ssdk_sh vlan entry create 1***
2. ***ssdk_sh vlan member add 1 0 tagged***
       
* ssdk_sh分析
       
  > cmd_des_t gcmd_des[] = {
  >   >
  > ​	{
  >   >
  >      > ​		"port", g_port_des,
  >      >
  >      > ​	},
  >
  > ```c
  > > ​	{
  > ```
  
     > ​		"vlan", g_vlan_des,
     >
     > ​	}
     >
     > ​	{
     >
     > ​		"portVlan", g_portvlan_des,
     >
     > ​	},
     >
     > ​	{
     >
     > ​		"qos", g_qos_des,
     >
     > ​	}
     >
     > "ip",  "nat", "trunk"  ...
     >
     > }
     >
     > sub_cmd_des_t g_vlan_des[]=
     >
     > {
     >
     > ​	{"entry", "set", SW_API_VLAN_ADD, NULL},
     >
     > ​	{"entry", "create", SW_API_VLAN_ADD, NULL}
     >
     > ​	{"entry", "del", SW_API_VLAN_DEL, NULL},
     >
     > ​	{"entry", "update", SW_API_VLAN_UPDATE, NULL},
     >
     > ​	{"entry", "append", SW_API_VLAN_APPEND, NULL},
     >
     > ​	{"entry", "flush", SW_API_VLAN_FLUSH, NULL},
     >
     > ​	{"fid", "set", SW_API_VLAN_FID_SET, NULL},
     >
     > ​	{"member", "set", "SW_API_VLAN_MEMBER_ADD", NULL},
     >
     > ​	{"member", "add", SW_API_VLAN_MEMBER_ADD, NULL},
     >
     > ​	{member, "del", SW_API_VLAN_MEMBER_DEL, NULL}
     >
     > }
     >
     > cmd_init()
     >
     > ​	malloc ioctl_buf and ioctl_argp
     >
     > ​	cmd_socket_init
     >
     > ​		ssdk_init
     >
     > ​	
     >
     > ​	
     >
     > 当输入参数小于1时会进入函数的死循环，以至于进入另一个shell "dev0@qca>"
     >
     > 当输入参数大于1时：
     >
     > ​	cmd_args(char*cmd_str, int argc, const char *argv[])
     >
     > ​		talk_mode = 0;
     >
     > ​		if arg[1]=="run"
     >
     > ​			sprintf(cmd_str, "%s %s %s", argv[1], argv[2], argv[3])
     >
     > ​			cmd_run_batch(cmd_str)
     >
     > ​		else
     >
     > ​			将argv传入的参数都存储在cmd_str中并以空格隔开
     >
     > ​			cmd_run_one(cmd_str)
     >
     > ​				cmd_parse(cmd_str, 0, 0);
     >
     > ​					把参数存储到*tmp_str[cmd_nr]中
     >
     > ​					cmd_lookup(tmp_str, cmd_index, cmd_index_sub)
     >
     > ​					以ssdk_sh vlan entry create 1为例
     >
     > ​					将vlan entry create匹配到的序号一次保存为*cmd_index, *cmd_index_sub, cmd_deepth=3
     >
     > ​				arg_val[0]=SW_API_VLAN_xx之类的api
     >
     > ​				arg_val[1]=ioctl_buf;//一个全局数组
     >
     > ​				cmd_parse_api(tmp_str, arg_val)
     >
     > ​				
     >
     > ​					
     >
     > ​			
     >
     > ​			
     >
     > ​		
     >
     > ​		
  
    ```
    
    ```
  
  
  ​     
  ​     sw_configvlan "vlan" "add" "br" "10"  "0" "0"
  ​     
  ​     	1. ports="0t 5t"
  ​      	2. sw_print_ssdk_cmds_add_vlan_for_wan vid
  ​           	1. ​	***ssdk_sh vlan entry create 10*** 
  ​           	2. ***ssdk_sh vlan entry append 10 10 0,5 0,5 null null no no***
  ​     
  ​     sw_print_ssdk_cmds_set_cpu_port_vid
  ​     
  
       ​	***ssdk_sh misc cpuvid set enable***

XR500--VLAN

​	op_set_induced_configs

​	op_create_brs_and_vifs

​		vlan_create_brs_and_vifs

​			ifconfig eth1 hw ether lanmac

​			ifconfig eth0 hw ether wanmac

​			ip link set dev eth1 name ethlan

​			vconfig add eth1 lanvid && ifconfig eth1.lanvid down

​			ip link set dev eth1.lanvid name ethlan

​			ifconfig brwan up

​			brctl addif br0 ethlan

​			ifconfig br0 hw ether lanmac

​			sw_configvlan "vlan" "start"

​			Internet:

​				vlan_create_internet_vif vid pri

​					vconfig add eth0 vid && ifconfig eth0.vid down

​					ip link set dev eth0.vid name ethwan

​					vlan_set_vif_pri ethwan pri

​					brctl addif brwan ethwan

​				sw_configvlan "vlan" "add" "br" "vid" "0" "pri"

​			Name(other):

​				vlan_create_br_and_vif vid pri

​					vconfig add ethlan vid && ifconfig ethlan.vid up

​					vconfig add eth0 vid && ifconfig eth0.vid up

​					erctl addif

​				sw_configvlan "vlan" "add" "vlan" "$3" "$5" "$4"

​				sw_configvlan "vlan" "add" "br" "vid" "0" "pri"

​			sw_configvlan "vlan" "add" "lan" "$lanvid" used_wports "0"

​			sw_configvlan "vlan" "add"

​					********other mode ****



/net/8021q

## 3.3. swconfig

***swconfig dev switch0 load swconf***

swlib_connect(cdev) //connect to switch0 on XR500

swlib_scan

swconfig_load_uci

主要通过netlink来与内核交互





# 4. DHCP(5days 11.27)

对于DHCP的基本过程如下：

client:																						server:

​		->discover

​																				offer<-

​		->request

​																				ack

* DHCPD
  * udhcpd /tmp/udhcpd.conf

当参数输入的参数小于2时就自动读取/etc/udhcpd.conf否则读取输入的文件如/tmp/udhcpd.con并添加到server_config结构体中。

> pidfile /var/run/udhcpd.pid
> start 192.168.1.2
> end 192.168.1.254
> interface br0
> remaining yes
> auto_time 5
> lease_file /tmp/udhcpd.leases
> option subnet 255.255.255.0
> option router 192.168.1.1
> option dns 192.168.1.1
> option lease 86400

配置server_config

设置SIGNUSR1和SIGTERM的信号处理函数，当这两个信号发生时，将该信号发送到signal_pipe[1]

listen_socket(INADDR_ANY, SERVER_PORT, server_config.interface) //监听br0的67端口

​	socket()

​	addr.sin_family addr.sin_port addr_sin_addr.s_addr

​	setopt(SO_REUSEDADDR)

​	setopt(SO_BROADCAST)

​	setopt(SO_BINDTODEVICE, interface)

​	bind(fd, addr)

​	flags = fcntl(fd, F_GETFL, 0)

​	fcntl(fd, F_SETFL, flags|O_NONBLOCK)

create raw socket with PF_PACKET protocol family to obtain layer2 source mac address

raw_server_socket = raw_socket(server_config.ifindex, 67)

​	struct sock_filter filter_instr[] = {

​		/* load 9th byte (protocol) */

​		/* jump to L1 if it is IPPROTO_UDP, else to L4 */

​		/* L1: load halfword from offset 6 (flags and frag offset) */

​		/* jump to L4 if any bits in frag offset field are set, else to L2 */

​		/* L2: skip IP header (load index reg with header len) */

​		/* load udp destination port from halfword[header_len + 2] */

​		/* jump to L3 if udp dport is CLIENT_PORT, else to L4 */

​		/* L3: accept packet */

​		/* L4:discard packet */

}

​	sock.sll_family = AF_PACKET;

​	sock.sll_protocol = htons(ETH_P_IP)

​	sock.sll_ifindex = ifindex;

   bind(fd, sock)

​	setsockopt(fd, SOL_SOCKET, SO_ATTACH_FILTER, &filter_prog, sizeof)



> DHCPDISCOVER:
>
> ​	sendOffer(&packet)
>
> DHCPREQUEST:
>
> ​	get_option( &packet, DHCP_REQUESTED_IP)
>
> ​	get_option(&packet, DHCP_SERVER_ID)
>
> ​	sendACK()
>
> DHCPDELINE:
>
> ​	

* udhcpc

  udhcpc -b -i wwan0 -h /tmp/dhcp_name.conf -r 0.0.0.0 -N 0.0.0.0

  ```c
  client_config_t client_config = {
  	/* Default options.*/
  	abort_if_no_lease: 0,
  	foreground: 0,
  	quit_after_lease: 0,
  	background_if_no_lease: 0,
  	interface: "eth0",
  	pidfile: NULL,
  	script: NULL, //DEFAULT_SCRIPT/2/AP/AP2
  	clientid: NULL,
  	domain_name: NULL,
  	hostname: NULL,
  	ifindex: 0,
  	arp: "\0\0\0\0\0\0",
  	vendor: NULL,
  	user_class:NULL,
  	authentication: NULL,
  	apmode: 0,
  };
  
  requet_ip = 0.0.0.0, old_request_ip = 0.0.0.0
  1. 设置client_config.script
  2. 从client_config.interface获取client_config.ifindex和client_config.arp即网卡MAC地址
  3. 设置client_config.clientid, 0:0x3d, 1:7, 2:1 3+: client_config.arp
  4. 信号处理
  5. run_script: default.wwan deconfig envp: interface, PATH, HOME, ip, siaddr, boot_file, sname
  6. fd=raw_socket_client(wwan, 68),set fd_set select
      
   	//每隔10s发送一次discover ap模式发总共5次，其它发总共3次
      首先判断WAN cable是否插上，再发送包
      获取random xid
      发送discover
      
      FD_ISDET(fd, &rfds)
      get_raw_packet(&packet, fd);
  	if packet type = offer
          state = REQUESTING
      send request packet
      
      get_raw_packet
      REQUESTIONG:
  		DHCPACK:
  			lease 等于server设定值，或者一个小时 t1=lease/2 t2=(lease*0x7)>>3=0.875*lease timeout = now + t1
  			检测IP conflict
              run_script(, BOUND)
              state = BOUND
      BOUND:
  		lease is starting to run out, time to renewing state
  		state = RENEWING
      RENEWING:
  		(t2-t1) <=(lease/14400 +1)时rebinding state= REBINDING
          否则每隔 
              t1 = (t2 - t1)/2 + t1
              timeout = t1 + start;
              0.1875*lease 发送一次renew包
  	REBINDING：
              if （lease - t2） <=(lease/14400 +1)
              	丢弃lease, state = INIT_SELECTING;
  			else:
  				每隔 t2=(lease - t2)/2 +t2
                      timeout = t2 +t1
  
  ```

  > ***default.script.wwan***
  >
  > deconfig:
  >
  > ​	ifconfig wwan0 0.0.0.0
  >
  > ​	config set wan2_dhcp_ipaddr=0.0.0 exit 0
  >
  > bound|renew:
  >
  > ​	get old ip subnet router from datalib
  >
  > ​	ifconfig interface ip broadcast netmask config set wan_dhcp_ipaddr,  subnet
  >
  > ​	dns setting
  >
  > ​	/sbin/ipconflict
  >
  > ​	
  >
  > ​	WAN_IPUP ip
  >
  > ​	ip changed then firewall restart, cmdigmp restart, 
  >
  > ​	restart /cmdroute (static route) and ripd
  >
  > ​	check qos bandwidth
  >
  > ​	if need, restart circle
  >
  > 
  >
  > 

# 5. DNS(3days )

​	首先建立信号处理函数，捕获SIGUSR1, SIGUSR2, SIGHUP, SIGTERM, SIGALRM, SIGCHLD

pid == 0 

SIGTERM： exit

pid != getpid()

​	SIGALRM: exit //alarm is used to kill TCP children after a fixed time.

pid == getpid() master process

​	SIGHUP: EVENT_RELOAD

​	SIGALRM: EVENT_ALARM

​	SIGTERM: EVENT_TERM

​	SIGUSR1: in_hijack=1

​	SIGUSR2: in_hijack=0

send_





# 6. UPNP(3days 11.18)

# 7. AWS-IOT(1days11.22)

# 8. DATA USAGE(1days 11.21)

# 9. NTP(2days11.20)

# 10. 网络协议栈（17th_7_8 18th_9_10_11 19th_12_13 20th_14_15 21st_16_17 22nd_18_19 23rd_20_21_22 24th_23_24_25 25tf_26_27 26th_28_29 27th_30_31 28th_32_33 29th_24_35 30th-31st_36_sum ）

## 10.1 关键数据结构（sk_buff&net_device）

| struct sk_buff *next;                      | next buffer in list                                          |
| ------------------------------------------ | ------------------------------------------------------------ |
| struct sk_buff *prev;                      | previous buffer in list                                      |
| ktime_t tstamp;                            | time we arrived                                              |
| struct sock *sk;                           | socket we are owned by                                       |
| struct net_device *dev;                    | device we arrive on/are leaving by                           |
| char cb[48] __aligned(8);                  | control buffer. Free for use by every layer. Put private vars here |
| unsigned long _skb_refdst;                 | destination entry(with norefcount bit)                       |
| struct sec_path *sp;                       | the security path, used for xfrm                             |
| unsigned int len, data_len;                | Length of actual data;  Data length                          |
| __u16 mac_len, hdr_len;                    | Length of link layer header; Writable header length of cloned skb |
| __wsum csum;                               | checksum (must include start/offset pair)                    |
| __u16 csum_start;                          | Offset from skb->head where checksumming should start        |
| __u16 csum_offset;                         | Offset from csum_start where checksum should be stored       |
| __u32 priority;                            | Packet queueing priority                                     |
| __u8 local_df:1,                           | allow local fragmentation                                    |
| __u8 cloned:1,                             | Head may be cloned (check refcnt to be sure)                 |
| __u8 ip_summed:2,                          | Driver fed us an IP checksum                                 |
| __u8 nohdr:1,                              | Payload reference only, must not modify header               |
| nfctinfo:3,                                | Relationship of this skb to the connection                   |
| pkt_type:3,                                | Packet class                                                 |
| fclone:2,                                  | skbuff clone status                                          |
| ipvs_property:1,                           | skbuff is owned by ipvs                                      |
| peeked:1,                                  | this packet has been seen already, so stats have been done for it, don't do them again |
| nf_trace:1;                                | netfilter packet trace flag                                  |
| __be16 protocol                            | packet protocol from  driver                                 |
| void (*destructor)(struct sk_buff *skb);   | Destruct function                                            |
| struct nf_conntrack *nfct;                 | Associated connection, if any                                |
| struct nf_bridge_info *nf_bridge;          | Saved data about a bridged frame - see br_netfilter.c        |
| int skb_iif;                               | ifindex of device we arrived on                              |
| __u32 rxhash;                              | the packet hash computed on receive                          |
| __u32 flow_cookie;                         |                                                              |
| __be16 vlan_proto;                         | vlan encapsulation protocol                                  |
| __u16 vlan_tci;                            | vlan tag control information                                 |
| __u16 tc_index; /*traffic control index*/  | Traffic control index                                        |
| __u16 tc_verd; /*traffic control verdict*/ | Traffic control verdict                                      |
| __u16 queue_mapping;                       | Queue mapping for multiqueue devices                         |
| _u8 pfmemalloc:1;                          |                                                              |
| __u8 ooo_okay:1;                           | allow the mapping of a socket to a queue to be changed       |
| __u8 14_rxhash:1;                          | indicate rxhash is a canonical 4-tuple hash over transport ports |
| __u8 wifi_acked_valid:1;                   | wifi acked was set                                           |
| __u8 wifi_acked:1;                         | whether frame was acked on wifi or not                       |
| __u8 no_fcs:1;                             | Request NIC to treat last 4 bytes as Ethernet FCS            |
| __u8 head_frag:1;                          |                                                              |
| __u8 encapsulation:1;                      |                                                              |
| __u8 fast_forwarded:1;                     |                                                              |
| unsigned int napi_id;                      | id of the NAPI struct this skb came from                     |
| dma_cookie_t dma_cookie;                   | a cookie to one of several possible DMA operations           |
| __u32 secmark;                             | security marking                                             |
| __u32 mark;                                | Generic packet mark                                          |
| __u32 dropcount;                           | total number of sk_receive_queue overflows                   |
| __u32 reserved_tailroom;                   |                                                              |
| __be16 inner_protocol;                     | Protocol(encapsulation)                                      |
| __u16 inner_transport_header;              | Inner transport layer header (encapsulation)                 |
| __u16 inner_network_header;                | Network layer header(encapsulation)                          |
| __u16 inner_mac_header;                    | Link layer header (encapsulation)                            |
| __u16 transport_header;                    | Transport layer header                                       |
| __u16 network_header;                      | Network layer header                                         |
| __u16 mac_header;                          | Link layer header                                            |
| sk_buff_data_t tail;                       | Tail pointer                                                 |
| sk_buff_data_t end;                        | End pointer                                                  |
| unsigned char *head,                       | head of buffer                                               |
| *data;                                     | Data head pointer                                            |
| unsigned int truesize;                     | Buffer size                                                  |
| atomic_t users;                            | User count - see {datagram, tcp}.c                           |
| void *free_addr[];                         |                                                              |
| void *alloc_addr[];                        |                                                              |
| u32 sum;                                   |                                                              |
|                                            |                                                              |

* 管理函数

  skb_put 从tail（数据尾部）插入数据 len+, tail指针加（下移）

  skb_push从data(数据头部)插入数据 len+, data指针减（上移）

  skb_pull从data(数据头部)移出数据 len-, data指针加（下移）

  skb_reserve 调整头部空间 This is only allowed for an empty buffer，在头部预留一些空间，通常允许插入一个报头，或强迫数据对齐某个边界。

  ​	即当data和tail指向同一地址（数据为空），data和tail指针加（下移）

  alloc_skb 调用kmem_cache_alloc_node获取skb_buff结构，再调用kmalloc_reserve获得数据缓存区。

  dev_alloc_skb是设备驱动程序使用的缓冲区分配函数，应该在中断中执行，其中也是调用alloc_skb。

  kfree_skb递减skb->users, 当该计数器为1时，这个基本函数才会释放缓冲区

  缓冲区尾端有个名为skb_shared_info的数据结构，用以保持此数据块的附加信息。

  skb_clone只拷贝sk_buff结构，然后引用计数，以免过早释放共享的数据块。skb_shared_info中的dataref会递增。

  * 列表管理函数

    skb_queue_head_init

    ​	用一个元素为空的队列对sk_buff_head.

    skb_queue_head, skb_queue_tail

    ​	把一个缓冲区分别添加到队列的头或尾

    skb_dequeue, skb_dequeue_tail

    ​	把一个元素分别从队列的头或尾去掉

    skb_queue_purge

    ​	把队列变为空队列

    skb_queue_walk

    ​	依次循环运行队列里中的每个元素

    都是原子操作

* net_device结构

| char name[IFNAMSIZ];                                         | name of the interface                                        |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| struct hlist_node name_hlist;                                | device name hash chain,                                      |
| char *ifalias;                                               | snmp alias                                                   |
| unsigned long mem_end;                                       | shared mem end                                               |
| unsigned long mem_start;                                     | shared mem start                                             |
| unsigned long base_addr;                                     | device I/O address                                           |
| int irq;                                                     | device IRQ number                                            |
| unsigned long state;                                         |                                                              |
| struct list_head dev_list;                                   |                                                              |
| struct list_head napi_list;                                  |                                                              |
| struct list_head unreg_list;                                 |                                                              |
| struct list_head close_list;                                 |                                                              |
| struct adj_list;                                             | directly linked devices, like slaves for bonding             |
| struct all_adj_list;                                         | all linked devices, *includeing* neighbours                  |
| netdev_features_t features;                                  | currently active device features                             |
| netdev_features_t hw_features;                               | user-changeable features                                     |
| netdev_features_t wanted_features;                           | user-requested features                                      |
| netdev_features_t vlan_features;                             | mask of features inheritable by VLAN devices                 |
| netdev_features_t hw_enc_features;                           | what encapsulation offloads                                  |
| netdev_features_t mpls_features;                             | mask of features in heritable by MPLS                        |
| int ifindex;                                                 | Interface index. --                                          |
| int iflink;                                                  | --Unique device identifier                                   |
| struct net_device_stats stats;                               |                                                              |
| atomic_long_t rx_dropped;                                    | dropped packets by core network(don't use this in drivers)   |
| const struct iw_handler_def *wireless_handlers;              | list of functions to handle wireless extensions              |
| struct iw_public_data *wireless data;                        | instance data managed by the core of wireless extensions     |
| const struct net_device_ops *netdev_ops;                     |                                                              |
| const struct ethtool_ops *ethtool_ops;                       |                                                              |
| const struct forwarding_accel_ops *fwd_ops                   |                                                              |
| void (*eth_mangle_rx)(struct net_device *dev, struct sk_buff *skb) |                                                              |
| struct sk_buff *(*eth_mangle_tx)(...)                        |                                                              |
| const struct header_ops *header_ops;                         | hardware header description                                  |
| unsigned int flags;                                          | interface flags                                              |
| unsigned int priv_flags;                                     | like flags but invisible to userspace                        |
| unsigned short gflags;                                       |                                                              |
| unsigned short padded;                                       | how much padding added by alloc_netdev()                     |
| unsigned char operstate                                      | RFC2863 operstate                                            |
| unsigned char link_mode;                                     | mapping policy to operstate                                  |
| unsigned char if_port;                                       | Selectable AUI, TP                                           |
| unsigned char dma;                                           | DMA channel                                                  |
| unsigned int mtu;                                            | interface MTU value;                                         |
| unsigned short type;                                         | interface hardware type                                      |
| unsigned short hard_header_len;                              | hardware hdr length                                          |
| unsigned short needed_headroom;                              |                                                              |
| unsigned short needed_tailroom;                              |                                                              |
| unsigned char perm_addr[MAX_ADDR_LEN];                       | permanent hw address                                         |
| unsigned char addr_assign_type;                              | hw address assignment type                                   |
| unsigned char addr_len;                                      | hardware address length                                      |
| unsigned short neigh_priv_len;                               |                                                              |
| unsigned short dev_id                                        | used to differentiate devices,share the same link layer address |
| spinlock_t addr_list_lock;                                   |                                                              |
| struct netdev_hw_addr_list uc;                               | unicast mac addresses                                        |
| struct netdev_hw_addr_list mc                                | multicast mac addresss                                       |
| struct netdev_hw_addr_list dev_addrs;                        | list of device hw addresses                                  |
| struct kset *queues_kset;                                    |                                                              |
| bool uc_promisc;                                             |                                                              |
| unsigned int promiscuity;                                    |                                                              |
| unsigned int allmulti;                                       |                                                              |
| struct vlan_info __rcu *vlan_info;                           | VLAN info                                                    |
| struct dsa_switch_tree *dsa_ptr;                             | dsa specific data                                            |
| struct tipc_bearer __rcu *tipc_ptr;                          | TIPC specific data                                           |
| void *atalk_ptr;                                             | AppleTalk link                                               |
| struct in_device __rcu *ip_ptr;                              | IPv4 specific data                                           |
| struct dn_dev __rcu *dn_ptr;                                 | DECnet specific data                                         |
| struct inet6_dev __rcu *ip6_ptr;                             | IPv6 specific data                                           |
| void *ax25_ptr;                                              | AX.25 specific data                                          |
| struct wireless_dev *ieee80211_ptr;                          | IEEE 802.11 specific data, assign before registering         |
| void *phy_ptr;                                               | PHY device specific data                                     |
| unsigned long last_rx;                                       | Time of last Rx, not should set in drivers, unless really needed |
| unsigned char *dev_addr;                                     | hw address,(before bcast because most packets are unicast)   |
| netdev_rx_queue *_rx;                                        |                                                              |
| unsigned int num_rx_queues;                                  | number of RX queues allocated at register_netdev() time      |
| unsigned int real_num_rx_queues;                             | number of RX queues currently acitve in device               |
| rx_handler_func_t __rcu *rx_handler;                         |                                                              |
| void __rcu *rx_handler_data;                                 |                                                              |
| struct netdev_queue __rcu *ingress_queue;                    |                                                              |
| unsigned char broadcast[MAX_ADDR_LEN];                       | hw bcast add                                                 |
| struct netdev_quue *_tx __cacheline_aligned_in_smp;          |                                                              |
| unsigned int num_tx_queues;                                  | number of TX queues allocated at alloc_netdev_mq() time      |
| unsigned int real_num_tx_queues;                             | Number of TX queues currently acitve in device               |
| struct Qdisc *qdisc;                                         | root qdisc from userspace point of view                      |
| unsigned long tx_queue_len;                                  | max frames per queue allowed                                 |
| spinlock_t tx_global_lock;                                   |                                                              |
| struct xps_dev_maps __rcu *xps_maps                          |                                                              |
| struct cpu_rmap *rx_cpu_rmap;                                | CPU reverse-mapping for RX completion interrupts, indexed by RX queue number. Assigned by driver. This must only be set if the ndo_rx_flow_steer opration is defined. |
| unsigned long trans_start;                                   | time(in jiffies) of last Tx                                  |
| int watchdog_timeo;                                          | used by dev_watchdog()                                       |
| struct timer_list watchdog_timer;                            |                                                              |
| int __percpu *pcpu_refcnt;                                   | number of references to this device                          |
| struct list_head todo_list;                                  | delayed register/unregister                                  |
| struct hlist_head index_hlist;                               | device index hash chain                                      |
| struct list_head link_watch_list;                            |                                                              |
| enum reg_state:8;                                            | register/unregister state machine                            |
| bool dismantle;                                              | device is going do be freed                                  |
| enum rtnl_link_state:16;                                     |                                                              |
| void (*destructor)(struct net_device *dev);                  | called from unregister, can be used to call free_netdev      |
| struct netpoll_info __rcu *npinfo;                           |                                                              |
| struct net *nd_net;                                          | network namespace this network device is inside              |
| void *ml_priv;                                               | mid-layer private                                            |

| struct pcpu_lstats __percpu *lstats;                | loopback stats                                               |
| --------------------------------------------------- | ------------------------------------------------------------ |
| struct pcpu_sw_netstats __percpu *tstats;           |                                                              |
| struct pcpu_dstats __percpu *dstats;                | dummy stats                                                  |
| struct pcpu_vstats __percpu *vstats;                | veth stats                                                   |
| struct net_bridge_port *br_port;                    | bridge stuff                                                 |
| struct garp_port __rcu *garp_port;                  | GARP                                                         |
| struct mrp_port __rcu *mrp_port;                    | MRP                                                          |
| struct device dev;                                  | class/net/name entry                                         |
| const struct attribute_group *sysfs_group[4];       | space for optional device, statistics, and wireless sysfs groups |
| const struct attribute_group *sysfs_rx_queue_group; | space for optional per-rx queue attributes                   |
| const struct rtnl_link_ops *rtnl_link_ops;          | rtnetlink link ops                                           |
| #define GSO_MAX_SIZE 65536                          |                                                              |
| unsigned int gso_max_size;                          |                                                              |
| #define GSO_MAX_SEGS 65535                          |                                                              |
| u16 gso_max_segs;                                   |                                                              |
| cnst struct dcbnl_rtnl_ops *dcbnl_ops;              | data center bridging netlink ops                             |
| u8 num_tc;                                          |                                                              |
| struct netdev_tc_txq tc_to_txq[TX_MAX_QUEUE];       |                                                              |
| u8 prio_tc_map[TC_BITMASK+1];                       |                                                              |
| unsigned int fcoe_ddp_xid;                          | max exchange id for FCoE LRO by ddp                          |
| struct netprio_map __rcu *priomap;                  |                                                              |
| struct phy_device *phydev;                          | phy device may attach itself for hardware timestamping       |
| struct lock_class_key *qdisc_tx_busylock;           |                                                              |
| int group;                                          | group the device belongs to                                  |
| struct pm_qos_request pm_qos_req;                   |                                                              |
|                                                     |                                                              |
|                                                     |                                                              |
|                                                     |                                                              |
|                                                     |                                                              |
|                                                     |                                                              |

标识符

int ifindex 独一无二的ID,当设备以dev_new_index注册时分派给每个设备

在原本的net_device中的一些函数指针，都封装到net_device_ops结构体中。

如：

ndo_open,ndo_stop, ndo_start_xmit, ndo_tx_xmit, ndo_tx_timeout, ndo_set_rx_mode,  ndo_do_ioctl, ndo_chage_mtu, ndo_set_features, ndo_validate_addr, ndo_set_mac_address. ndo_poll_controller.

## 10.2 用户空间和内核接口

* /proc/sys/下的目录ctl_table来定义，其中的.child = child_dir_ctl_table.

* iotcl

  ifconfig命令使用ioctl与内核通信，ifconfig会打开一个套接字，用从系统管理员那里接收的信息（struct ifreq ）初始化一个本地数据结构，然后以ioctl调用传递给内核。

  ​	

  ```c
  struct ifreq data;
  fd = socket(PF_INIT, SOCK_DGRAM, 0);
  <...对“data”初始化...>
  err = ioctl(fd, SIOCSIFMTU, &data);
  ```

## 10.3 系统初始化

### 10.3.1 通知链

通知链就是一份简单的函数列表，当给定事件发生时予以执行。每个函数都让另一个子系统知道，调用此函数的子系统内所发生的一个事件或者子系统所侦测到的一个事件。

因此，就每条通知链而言，都有被动端和主动端，也就是所谓的发布-订阅模型。

* 定义链

  通知链列表元素的类型是notifier_block,其定义如下：

  struct notifier_block

  {

  ​	int (*notifier_call)(struct notifier_block *self, unsigned long, void *);

  struct notifier_block *next;

  int priority;

  }

...

### 10.3.2 网络设备初始化

lbr1020中使用的网卡型号为edma_axi

* 设备注册和初始化

  由设备驱动程序和通用总线层（PCI或USB）合作完成，把每个设备的这类功能配置成IRQ和I/O地址，使其能与内核交互。

  NIC必须被分派一个IRQ，虚拟设备不需要分派。

  驱动程序将其设备的一个内存区域映射到系统内存，使得驱动程序的读写操作可以通过系统内存地址直接进行。I/O端口和内存分别使用request_region和release_region注册和释放。

* 硬件中断

  每个中断事件都会运行一个函数被称为中断处理例程，当设备驱动程序注册一个NIC时，会请求并分派一个IRQ。

  int request_irq(unsigned int irq, void (**handler)(int, void*, struct pt_regs*), unsigned long irqflags, const char* *devname, void *dev_id)

  request_irq 返回0为成功

  unsigned int irq :请求的中断号

  (*handler) 中断处理函数

  flags 掩码：

  ​	SA_INTERRUPT:当置位时，这表示一个快速中断处理。快速处理在当前处理器上禁止中断来执行。

  ​	SA_SHIRQ: 这个位表示中断可以在设备间共享。

  ​	SA_SAMPLE_RANDOM: 

  dev_name 设备名在/proc/interrupts来显示

  void *dev_id 用作共享中断线的指针，它是个独立的识别ID。当不共享时设为NULL。

  * 前和后半部

    前半部：request_irq注册的那个。

    后半部：由前半部调度来延后执行的函数

  * Tasklet实现

    tasklet必须使用DECLARE_TASKLET(name, function, data);

    ​	name:是tasklet的名字，function是调用来执行tasklet（它带有一个unsigned long 参数并且返回void）的函数，以及data是一个unsigned long 值来传递给tasklet函数

  ```c
  void short_do_tasklet(unsigned long);
  DECLARE_TASKLET(short_tasklet, short_do_tasklet, 0);
  irqreturn_t short_tl_interrupt(int irq, void *dev_id, struct pt_regs *regs)
  {
  	/*do something*/
  	tasklet_schedule(&short_tasklet); //调用short_tasklet
  	return IRQ_HANDLED;
  }
  void short_do_tasklet(unsigned long unused)
  {
  	do_many_things;
  }
  ```

  * 工作队列

    ```c
    static struct work_struct short_wq;
    INIT_WORK(&short_wq, (void(*)(void *))short_do_tasklet, NULL);
    ireqreturn_t short_wq_interrupt(int irq, void *dev_id, struct pt_reqs *regs)
    {
    	...
    	schedule_work(&short_wq);
    	return IRQ_HANDLED;
    }
    ```

  


## 10.4 初始化选项

* 模块初始化（module_param系列的宏）

  module_param(multicast_filter_limit, int 0444);

  会在sys对应驱动名中看到/sys/module/xxx/parameters/...

* 引导期间内核选项（__setup系列的宏）

* 设备处理层初始化：net_dev_init

  net_dev_init()

  * Initialise the packet receive queues. 初始化两个网络软件中断所使用的对应各个CPU的数据结构被初始化。
  * dev_proc_init和devmcast_init添加文件到/proc
  * netdev_kobject_init向/sys/class/net目录注册
  * dst_init协议无关的目的缓存初始化，通知链
  * 把一个回调处理例程注册到发出有关CPU热拔插事件的事件的通知信息的通知链。使用的回调函数为dev_cpu_callback。

* 组件初始化的内核基础架构

  * ​	注册关键字

    __setup(string, function_handler)

    当输入的引导期间字符串中包括string时，就执行function_handler.string必须以=字符作结束，以使parse_args的解析能轻松点。任何跟在=后的字符串都传给function_handler。

    net/core/dev.c

    __setup("netdev=", netdev_boot_setup);

    一个处理函数可以对应多个关键字

    net/ethernet/eth.c

    __setup("ether=", netdev_boot_setup);

    __setup()

    ​	__setup_param(str, fn, fn, 0)

    ​		__setup_param会把所有obs_kernel_params实例放入一个专用的内存区域内。即\_\_setup_start和\_\_setup_end两个指针之间。

  * 使用引导选项配置网络设备

    当ether=和netdev=关键字配匹配到，会把netdev_boot_setup处理函数的结果储存到netdevice.h中的netdev_boot_setup数据结构中。

    netdev_boot_setup:就是从字符串中抽取输入参数，填入一个ifmap结构，然后用netdev_boot_setup_add把ifmap结构添加到dev_boot_setup数组中。

    ```c
    /*this structure holds at boot time configured netdevice settings. They are then used in the devices probing.*/
    struct netdev_boot_setup{
    	char name[IFNASIZ];
    	struct ifmap map;
    }
    struct ifmap {
        unsigned long mem_start;
        unsigned long mem_end;
        unsigned short base_addr;
        unsigned char irq;
        unsigned char dma;
        unsigned char port;
        /*3 bytes spare*/
    }
    
    ```

  * 引导期间初始化函数

    * 当所有内核组件都已初始化之时，必须在引导期间执行
    * 一旦被执行后，就不再需要这些函数

    * \_\_init和\_\_exit宏

      \_\_init宏把输入函数放到.text.init内存节区：

      #define \_\_init \_\_attribute\_\_ ((\_\_section\_\_ (".text.init")))

      用于关闭模块的函数都放在.text.exit节区中

## 10.5 设备的注册和初始化

* 分配net_device结构

  alloc_netdev_mqs(int sizeof_priv, const char *name, unsigned int txqs, unsigned int rxqs)

  @sizeof_priv size of private data to allocate space for 

  @name device name format string

  @setup callback to initialize device

  @txqs: the number of TX subqueues to allocate

  @rxqs: the number of RX subqueues to allocate

  > 在dm9000中的dm9000_probe中为：
  >
  > ndev = alloc_etherdev(sizeof(struct board_info));
  >
  > ​	alloc_etherdev_mq(sizeof_priv, 1)
  >
  > ​		alloc_etherdev_mqs(sizeof_priv, count, count)
  >
  > ​			alloc_netdev_mqs(sizeof_priv, "eth%d", ether_setup, txqs, rxqs)

  ether_setup会初始化net_device的header_ops, type, hard_header_len, mtu, addr_len, tx_queue_len, flags, priv_flags, broadcast

  probe会初始化base_addr, irq, features, netdev_ops(包含很多处理函数如：open,stop, start_xmit等), watchdog_timeo, ethtool_ops, dev_addr

  net_device数据结构插入在一个全局列表和两张hash（name和index）表中

* 设备的注册

  用dev_new_index分派一个独一无二的识别码给设备。

  list_netdevice添加dev到dev_base,再插入到dev_name_hash, dev_index_hash.

  设置dev->state中的__LINK_STATE_PRESENT标识，让设备能为系统可用（可见可用）

  设备的队列规则通过dev_init_scheduler做初始化，由流量控制用于实现QoS。队列规则定义出口封包如何排入出口队列，以及如何退出出口队列、定义开始丢掉封包前有多少封包可以排入队列中。

  call_netdevice_notifiers通知所有对此感兴趣的子系统。

  ioctl->inet_ioctl->dev_ioctl->dev_ethtool->ethtool_xxx

  ethtool_xxx辅助函数把数据从用户空间移往内核空间然后调用其中一个ethtool_ops函数。

## 10.6 传输和接收

![image-20201020104059583](D:\data\app\typora\image-20201020104059583.png)

### 10.6.1 中断和网络驱动程序

```c
dm9000_open
	request_irq(dev->irq, dm9000_interrupt, irqflags, dev->name, dev);
		dm9000_interrupt
		->holders of db->lock must always block IRQS
		->save previous register address
		->disable all interrupts
		->got dm9000 interrupt status
		->received the coming packet
			dm9000_rx(dev);
		->transmit Interrupt check
			dm9000_tx_done(dev, db)
```



### 10.6.2 帧的接收

* NAPI简介

  NAPI混合了中断事件和轮询，在高流量负载下其性能会比旧方法要好，因为可以大幅减少CPU的负载。

  如果接收到帧时，内核还没完成处理前几个帧的工作，驱动程序就没有必要产生其他中断事件：让内核一直处理设备输入队列中的数据会比较简单（该设备的中断功能关闭）， 然后当该队列为空时再重新开启中断功能。

```c
dm9000_rx(dev)
	netdev_alloc_skb(dev, Rxlen + 4);
	skb_reserve(skb, 2);
	pdptr = (u8 *)skb_put(skb, RxLen - 4); //len+4, tail+4
	(db->inblk)(db->io_data, rdptr, RxLen);
	skb->protocol = eth_type_trans(skb, dev);
	netif_rx(skb);
		netif_rx_internal(skb);
		netpoll_rx(skb);
		/*save receive time*/
		net_timestamp_check(netdev_tstamp_prequeue, skb);
		cpu= smp_processor_id();//取出本地CPU ID
		/*enqueue_to_backlog is called to queue an skb to a per CPU backlog queue(may be a remote CPU queue)*/
		enqueue_to_backlog(skb, cpu, &rflow_qtail);
			_skb_queue_tail(&sd->input_pkt_queue, skb);/*把数据添加到每个CPU的输入队列中（softnet_data->input_pkt_queue）*/
			__napi_schedule(sd, &sd->backlog);
			list_add_tail(&napi->poll_list, &sd->poll_list);
			->sd->backlog.poll = process_backlog; //dev.c net_dev_init
				->_netif_receive_skb(skb);
					_netif_receive_skb_core
                        deliver_skb(skb, pt_prev, orig_dev);
							atomic_inc(&skb->users);
							pt_rev->func(skb, skb->dev, pt_prev, orig_dev);
/*func() init */
/*af_inet.c*/
static struct packet_type ip_packet_type _read_mostly = {
    	.type = cpu_to_be16(ETH_P_IP),
    	.func = ip_rcv,
};
/*arp.c */
static struct packet_type arp_packet_tyep _read_mostly = {
    	.type = cpu_to_be16(ETH_P_ARP),
    	.func = arp_rcv,
}

/*ip_input.c -- ip_rcv*/
return NF_HOOK(NFPROTO_IPV4, NF_INET_PRE_ROUTING, skb, dev, NULL, ip_rcv_finish);/*this will be used by firewall*/

	
```



### 10.6.3 帧的传输

ip_output.c

ip_route_output_flow -> ip_queue_xmit

​											ip_local_out

​											NF_INET_LOCAL_OUTPUT

​											ip_output

​											NF_INET_POST_ROUTING

​											ip_finish_output

​											ip_finish_output2

​											dst_neigh_output

​											neigh_hh_output

​											dev_queue_xmit

​												__dev_queue_xmit

​												__dev_xmit_skb

​												(qdisc的控制，即QoS流量控制)

​													_qdisc_run

​														qdisc_restart

​															sch_direct_xmit

​																dev_hard_start_xmit //也包含了vlan的一些处理

​																	ops->ndo_start_xmit

​												这个在驱动函数中定义

## 10.7 网桥

### 10.7.1 生成树协议

STP通过让各个网桥之间交换称为网桥协议数据单元（bridge protocol data unit, BPDU）的特殊帧来达到其目的。用BPDU交换的信息可让网桥：

​		a. 为每个网桥端口指定一个明确的状态，例如转发或阻塞，以定义该端口可否接收数据流量。

​		b. 通过对端口状态的设定，从环路拓扑中选择或丢弃适当的链路，以这种方式最终生成无环路拓扑。

* 根网桥

  * 根网桥是唯一能产生BPDU的网桥。其他网桥只有在其接收到BPUD时才会传输BPUD（也就是他们修改所接收到的信息，更新其中一些字段）
  * 当拓扑发生改变时，根网桥确保网络中的每台网桥都知道此事

* 指定网桥

  每个LAN都可以有一个指定网桥，它成为该LAN中所有主机和网桥到达根节点的网桥。指定网桥的选择是根据LAN上哪台网桥通往根网桥的路径开销最低而定。

* 生成树端口

  * 端口状态

    * 关闭（disable）

      该端口已关闭，它不再接收或传输任何流量

    * 阻塞（blocking）

      该端口已开启，但STP将其阻塞了。该端口不能转发任何数据流量。

    * 监听（listening）

      该端口已开启，但是不能用于转发任何数据流量

    * 学习（learning）

      该端口已开启，但是不能用于转发任何数据流量，然而，该网桥的地址学习进程是激活的

    * 转发（forwarding）

      该端口已开启，地址学习是激活的，并且可以转发数据流量

  * 端口角色

    * 根

      除根网桥外，每个网桥上通往根网桥路径开销最低的端口就会被选为根端口。

    * 指定

      每个LAN上，通往根网桥的路径开销最小的端口就会被选为指定端口。指定端口所属的网桥就称为该LAN的指定网桥。一台网桥上的端口连接不同的LAN时可以有一个以上的指定端口。

* 网桥ID和端口ID

  * 网桥ID（bridge ID）

    每个网桥都会分配一个ID,称为网桥ID，此ID是一个8字节的数，分为两部分。最低的六个字节是某个桥接端口的Ethernet MAC地址，而最高的两个字节是一个可以配置的优先级，称为网桥的优先级。

    网桥ID中多了一个新元素，称为系统ID扩展（12位），可以使4096个不同网桥共享同一MAC地址。

  * 端口ID( port ID)

    ID的一部分代表一个唯一的识别码，称为端口号，其值在网桥本地有意义。现在的端口号为12位。

* 网桥协议数据单元（BPDU）

  协议ID|协议版本号|类型|TC/TCA|根网桥ID|根路径开销|网桥ID|端口ID|消息生存期|Max age|hello|forward delay

  优先级向量=根网桥ID+根路径开销+网桥ID+端口ID

  * 何时传输配置BPDU

    * 根网桥会运行一个定时器（hello定时器），该定时器到期后就触发配置BPDU的传输。同一个BPDU从根网桥的每一个指定端口传输出去。只有根网桥能产生新的BPDU，但是当一个网桥加电启动时，它会以为自己就是根网桥（因为没有其他优先级可以比较）。所以，该网桥会将其所有端口都设定为指定角色，启动Hello定时器，然后开始产生BPDU。
    * 非根网桥只有在应答其根端口收到的BPDU时才会产生BPDU；也就是说，非根网桥会传递BPDU。由非根网桥传输的BPDU，其所携带的信息和它们所接收的BPDU除下列字段外都是相同的，非根网桥更新这些字段：
      * 传输者的网桥ID和端口ID会被该网桥自己的信息替换。
      * 该网桥会把开销更新为其所接受的开销加上其本地网桥中接收BPDU的端口（根端口）的开销之和。
      * 消息生存期会更新

    无论是否是根网桥，在下列情况下也会传输一个配置BPDU:

    * 当一台网桥接收到一个BPDU，如果其优先级向量低于该网桥在其端口上所用的，那么，网桥就会以其自己（较高级）的信息应答。
    * 当一台网桥接收到一个TCN BPDU时，就会立刻发出一个设有特殊标识的配置BPDU作为应答。这样有利于快速传播拓扑中的变化。

  * 拓扑变化

    检测到拓扑变化的网桥A2会启动TCN定时器，然后从（新）根端口发出一个TCM BPDU.当上层网桥D2收到TCN BPDU时，会传回设有TCA标识的配置BPDU作为应答，然后启动TCN定时器，再传出一个TCM BPDU。当最初的那个网桥A2收到来自上层D2的应答时，就会停止其TCN定时器。D2接收到上层C1的应答后，也是如此。

    当根网桥C1收到TCN BPDU时，会启动Topology Change定时器（持续转发延迟加最大生存期），然后在该定时器未决的期间，在所有发送出去的BPDU上设定TC标识。

    TC标识会往下传播到整个树上，因为所有网桥都会转发接受自根网桥的标识。当一台网桥看见一个入口BPDU上设有此标识时，就会启动Short Aging定时器（如果还没启动的话）。一旦根网桥的Topology Change定时器到期了，根网桥就会停止在其BPDU中设置TC标识。其他网桥接收到一个清除掉TC标识的BPDU时，就会停止使用Short Aging定时器，而开始启动默认的老化时间（aging）。当正在使用Short Aging定时器的网桥收到一个有TC标识的配置BPDU时，它不需要做任何事。

* Linux的实现

  * 重要的数据结构

    * mac_addr	MAC地址

    * bridge_id     网桥ID

    * net_bridge_fdb_entry    转发数据库的记录项。网桥所学到的每个MAC地址都会有这样一个记录。

    * net_bridge_port    网桥端口

    * net_bridge    应用到单个网桥的信息。该结构会附加到net_device数据结构之上。和大多数虚拟设备一样，其内部包括只有虚拟设备程序（桥接代码）才能理解的私有信息。

    * br_config_bpdu

      入口配置BPDU的一些关键字段会复制到该数据结构中，此结构会将这些字段传递给处理配置BPDU的函数，而不是由原BPDU来传递。

  * 桥接程序的初始化

    在br.c中的br_init （br_deinit与其相反）

    * 注册stp协议 stp_proto_register(&br_stp_proto)
    * 转发数据库初始化，就是在内存中建立一块slab cache,以存放net_bridge_fdb_entry结构（br_fdb_init）.
    * 注册pernet子系统 （register_pernet_subsys）
    * 初始化网桥的netfilter (br_netfilter_init)
    * 注册网络设备通知链 register_netdevice_notifier
    * netlink初始化 br_netlink_init
    * 初始化函数指针br_ioctl_hook为处理ioctl命令的函数。

  * 建立网桥设备和网桥端口（br_if.c）

    每个网桥设备最多可以有BR_MAX_PORTS(1024)个端口

    网桥设备的建立和删除分别是通过br_add_bridge函数和br_del_bridge函数进行

    给网桥设备添加端口或删除端口是通过br_add_if函数和br_del_if函数进行

  * 建立一个新网桥设备

    br_if.c -- br_add_bridge(*net, *name)

    ​	alloc_netdev(sizeof(struct net_bridge), name, br_dev_setup)

    ​		br_dev_setup

    ​			*br = netdev_priv(dev); //get network device private data

    ​			设置参数包括dev, br

  * 删除网桥

    br_del_bridge

    ​	br_dev_delete

    ```
    /*Delete port(interface) from bridge is done in two steps*/
    /*First step, marks device as down. That deletes all the timers and stops new packets from flowing through.*/
    /*Final cleanup doesn't occur until after all CPU's finished processing packets.*/
    del_nbp(struct net_bridge_port *p)
    	sysfs_remove_link; //remove symlink in object's directory.
    	dev_set_promiscuity(dev, -1);//promiscuity count decrease 1
    	br_stp_disable_port(p); 
    		del_timer(&p->message_age_timer);
    		del_timer(&p->forward_delay_timer);
    		del_timer(&p->hold_timer);
    		br_fdb_delete_by_port;//删除每个端口在转发数据库中所有的数据项。
    			br_multicast_disable_port(p);
    				del_timer(&port->multicast_router_timer);
    				del_timer(&port->ip4_query.timer);
    	
    ```

	## 10.8 IPv4

	### 10.8.1 IPv4的数据结构

```c
struct iphdr{
	__u8 version:4, //协议版本
		 ihl:4;     //报头长度
	__u8 tos; //type of service,服务类型
	__be16 tot_len; //total length
	__be16 id; //封包识别符
	__be16 frag_off; //fragment offset分段偏移量
	__u8 ttl; //time to live
	__u8 protocol; //L4层的协议
	__sum16 check; //报头校验和
	__be32 saddr;
	__be32 daddr;
    /*The options start here*/
}
/*af_inet.c*/
inet_init()
    proto_register(&tcp_prot, 1);
	proto_register(&udp_prot, 1);
	proto_register(&raw_prot, 1);
	proto_register(&ping_prot, 1);
	socket_register(&inet_family_ops);
		struct net_proto_family inet_family_ops = {
            .family = PF_INET,
            .create = inet_create,
            .owner = THIS_MODULE,
        };
	/*add all the base protocols*/
	inet_add_protocol(&icmp_protocol, IPPROTO_ICMP);
	inet_add_protocol(&udp_protocol, IPPROTO_UDP);
	inet_add_protocol(&tcp_protocol, IPPROTO_tcp);
		struct net_protocol tcp_protocol = {
            	.early_demux = tcp_v4_early_demux,
            	.handler	 = tcp_v4_rcv,
            	...
        };
	inet_add_protocol(&igmp_protocol, IPPROTO_igmp);
	
	/*register the socket-side information for inet_create*/
	inetsw[0]-inetsw[SOCK_MAX]
    inetsw_array-inetsw_array[INETSW_ARRY_LEN]
        inet_register_protosw
    arp_init();
	ip_init();
	tcp_v4_init();
	/*setup tcp slab cache for open requests*/
	tcp_init();
	/*setup udp memory threshold*/
	udp_init();
	ping_init();
	...
    dev_add_pack(&ip_packet_type);
		struct packet_type ip_packet ={
            .type = cpu_to_be16(ETH_P_IP),
            .func = ip_rcv,
        }
  ip_rcv  
      /*pskb_my_pull 后会重新获取IP头部指针*/
      iph = ip_hdr(skb)
      /*一些健康检测*/
      /*进行防火墙处理，若不丢包，则调用ip_rcv_finish*/
      return NF_HOOK(PF_INET, NF_IP_PRE_ROUTING, skb, dev, NULL, ip_rcv_finish);
ip_rcv_finish
    ip_route_input_noref
    /*更新QoS的统计信息*/
    if(unlikely(skb_dst(skb)->tclassid))
        ...
     /*当IP报头的长度大于20字节时（5*32bit），有一些选项要处理*/
    if(iph->ih1>5 && ip_rcv_option(skb))
        
    /*input packet from network to transport*/
    return dst_input(skb)
    	dst_input
        	return skb_dst(skb)->input(skb);
	/*上面的input函数可能是ip_local_deliver或ip_forward,这取决于封包的目的地址*/
        
```

ip_rcv -> ip_rcv_finish -> ip_route_noref 进行路由查询的相关操作。

### 10.9.2 转发和本地传递

​	在route.c中__mkroute_input()中

​		rth->dst.input = ip_forward;

​		rth->dst.output = ip_output;

这个是在路由系统中设定，接着会在上一节的skb_dst(skb)->input(skb);中被调用。

转发由下列步骤组成：

1. 处理IP选项。这可能会涉及记录本地IP地址以及时间戳（如果IP报头中的选项需要这两项信息）。
2. 确定封包可以被转发（根据IP报头字段）。
3. 递减IP报头的TTL字段，然后，如果TTL字段变为零，就丢弃该封包。
4. 根据路径相关MTU，必要时处理分段工作。
5. 把封包传送至外出设备。

ip_forward中主要是一些规则检测包括IPsec策略检测，ttl的递减，最后调用NF_HOOK(NFPROTO_IPV4, NF_INET_FORWARD, skb, skb->dev, rt->dst.dev, ip_forward_finish);

​	ip_forward_finish

​		dst_output(skb)

dst_output会使用虚拟函数（如果目的地址是单播的，会初始化为ip_output;如果是多播，则会初始化为ip_mc_output）

ip_output

​	NF_HOOK_COND(NFPROTO_IPV4, NF_INET_POST_ROUTING, skb, NULL, dev, ip_finish_output, !(IPCB(skb)->flags & IPSKB_REROUTED))	

​	ip_finish_output

​		检测skb->len与mtu决定是否分段

​		ip_finish_output2(skb)

​			dst_neigh_output

​				neigh_hh_output

​					dev_queue_xmi

### 10.9.3 IPv4传输

在dst_out前封包准备阶段，内核的任务：

1. 查询下一个跳点

   IP层必须知道外出设备以及用作下一个跳点的下一个路由器。路径是通过函数ip_route_output_flow发现的。

2. 初始化IP报头

   几个字段会在此阶段填入（如封包ID）。如果封包是要转发的那个，稍早之前就已对报头做了一些工作（如更新TTL、校验和以及选项字段）。但是，若要传输，还有更多 事情要完成。

3. 处理选项

4. 分段

5. 校验和

6. netfilter检查

7. 更新统计数据

* 本地流量相关的套接字数据结构

  Linux中用socket结构的实例表示BSD套接字。此结构包含一个指向sock数据结构的指针，而网络层信息都存储在sock结构内。只要传输是本地产生（只有少数例外），每个sk_buff缓冲区都会和其sock实例相关联，而且以skb->sk链接起来。

  如果缓冲区已被设定正确路由信息（skb->dst）,就没有必要查询路由表。ip_queue_xmit会检查套接字结构中是否已缓存了一条路径。如果有的话，就会确定该路径依然有效：

  > rt=(struct rtable *)__sk_dst_check(sk, 0)

  如果套接字还没有一条缓存的路径可供封包使用，或者如果IP层迄今一直在用的路径在同一时间也失效了（ 路由协议的更新），则ip_queue_xmit就必须利用ip_route_output_flow寻找一条新路径，然后将结果存储到sk数据结构中。目的地址由daddr变量表示。首先，此变量设置成封包的最终目的地（inet->daddr）；如果存在就把daddr变量设成源路径中的下个跳点（inet->faddr）.

  如果ip_route_output_flow失败了，封包会被丢弃。如果找到了路径，就会以__sk_dst_set存储在sk数据结构中，使其下次可以直接被使用，而路由表就不必再次被查询。如果因为某种原因该路径再次失效，就得再次调用ip_queue_xmit来通过ip_route_output_flow 再次找出新路径。

  *  ip_append_data函数
    * 把来自于L4层的输入数据组成一些缓冲区，而这些缓冲区的尺寸又使其易于处理IP分段工作。此外，把那些数据片段放进那些缓冲区时，要还安排成让L3和L2层稍后能够轻易新增较低层协议头。
    * 优化内存分配，把来自上层的信息以及出口设备的能力考虑进来。
    * 处理L4校验和

  ip_ufo_append_data alloc a skb and reserve space hardware layer, initialize network layer and protocol layer

* L4协议的注册

  位于IPv4之上的L4协议是由net_protocol数据结构定义

  ```c
  net_protocol tcp_protocol = {
  	.early_demux = tcp_v4_early_demux,
  	.handler = tcp_v4_rcv,
  	...
  };
  net_protocol udp_protocol = {
  	.early_demux = udp_v4_early_demux,
  	.handler = udp_rcv,
  	...
  };
  net_protocol icmp_protocol;
  
  /*使用inet_add_protocol来将protocols添加到inet_protos*/
  inet_add_protocol(&tcp_protocol, IPPROTO_TCP);
  /*inet_protos的只读存取*/
  ip_local_deliver_finish
      /*指向L4头部*/
      __skb_pull(skb, skb_network_header_len(skb));
  	/*提取L4层的协议*/
  	protocol = ip_hdr(skb)->protocol;
  	/*根据protocol名从inet_protos哈希表中获取net_protocol结构体*/
  	ipprot =  rcu_dereference(inet_protos[protocol]);
  	/*再调用L4层的rcv函数,在此函数前后都会有xfrm4_policy_check函数的检测，此函数为IPsec的检测函数*/
  	ret = ipprot->handler(skb);
  ```

## 10.10 邻居子系统

当传送一个封包时，执行下面的步骤：

1. 本地主机的路由子系统选择L3目的地址（下一个跃点）
2. 根据路由表，如果下一个跃点在同一个网络中（也就是说下一个跃点是邻居），邻居层就会把目的L3地址解析为跃点的L2地址。这个关联被放入缓存以便将来再次使用。这样的话，如果某个应用程序短期内发送多个封包给另一个应用程序，那么只在发送第一个封包的时候使用一次邻居协议。
3. 最后，一个函数，比如dev_queue_xmit负责完成发送，将封包传送给流量控制（QoS）层。邻居层实际也能调用其他函数（大部分用dev_queue_xmit进行封装）。

# 11. TCP&IP ARCHITECTURE(2weeks 11.15)

## 11.1 socket layer

## 11.2 netlink sockets

## 11.3 IP QUALITY of Service In Linux(IP QoS)

> iproute/tc/tc.c main
>
> ​	do_cmd()
>
> ​		qdisc
>
> ​			do_qdisc
>
> ​				if qdisc add
>
> ​					tc_qdisc_modify -- allocates the req struct and initializes.
>
> ​						rtnl_talk -- allocates the msghdr struct and calls the sendmsg with & msg
>
> ​							sendmsg(rtnl->fd, &msg, 0); 								
>
> ​								inivokes sendmsg() sys_call and enters kernel mode
>
> - ------------------------------------------------------------------------------------------------------------------------------
>
> - kernel 
>
>   sys_sendmsg()
>
>   ​	__sys_sendmsg
>
>   ​		___sys_sendmsg
>
>   ​			sock_sendmsg
>
>   ​				__sock_sendmsg
>
>   ​					sock->ops->sendmsg
>
>   ops = netlink_ops  sendmsg = netlink_sendmsg
>
>   ​			memcpy_fromiovec(skb_put(skb, len), msg->msg_iov, len) //copy the netlink packet to sk_buff.
>
>   ​			netlink_unicast(sk, skb, dst_portid, msg->msg_flags&MSG_DONTWAIT); //or netlink_broadcast
>
>   ​				netlink_unicast_kernel
>
>   ​					nlk->netlink_rcv(skb)
>
>   ***************************************************-----------------------------------------------***************************************************************
>
>   在/net/core/rtnetlink.c中
>
>   rtnetlink_net_init
>
>   ​	struct netlink_kernel_cfg cfg = {
>
>   ​	.group = RTNLGRP_MAX,
>
>   ​	.input = rtnetlink_rcv,
>
>   ​	...
>
>   };
>
>   sk = netlink_kernel_create(net, NETLINK_ROUTE, &cfg);
>
>   ​		nlk_sk(sk)->netlink_rcv = cfg->input;
>
>   *******************__________________________________************************************_______________________________*********************************************************
>
>   对于NETLINK_ROUTE类型的套接字来说就是rtnetlink_rcv.
>
>   rtnetlink_rcv(skb)
>
>   ​	netlink_rcv_skb(skb, &rtnetlink_rcv_msg)
>
>   ​		rtnetlink_rcv_msg
>
>   ​			doit = rtnl_get_doit(family, type);
>
>   ​			return doit(skb, nlh);
>
>   
>
>   在net/core/rtnetlink.c/rtnetlink_init中
>
>   rtnl_register(PF_UNSPEC, RTM_NEW_NEWLINK, rtnl_newlink, NULL, NULL);//后面跟着多个注册函数
>
>   ​	__rnt_register(protocol, msgtype, doit, dumpit, calcit)
>
>   ​		if(doit)
>
>   ​			tab[msgindex].doit = doit
>
>   所以如果是RTM_NEW_NEWLINK类型的 return doit(skb, nlh)将会调用到rtnl_newlink(skb, nlh)
>
>   net/sched/sch_api.c
>
>   也有rtnl_register(PF_UNSPEC, RTM_NEWQDISC, tc_modify_qdisc, NULL, NULL);
>
>   所有如果是RTM_NEWQDISC类型将会调用到tc_modify_qdisc(skb, nlh)
>
>   tc_modify_qdisc(skb, nlh)
>
>   ​	clid = tcm->tcm_parent;
>   
>   ​	/*判断clid是否为TC_H_ROOT,然后通过qdisc_lookup和qdisc_leaf找出parent qdisc和band qdisc。*/
>   
>   ​	qdisc_create
>   
>   ​		qdisc_alloc
>   
>   ​			p=kzalloc_node
>   
>   ​			sch = (struct Qdisc *)p
>   
>   ​			skb_queue_head_init(&sch->q)
>   
>   ​			skb->ops = ops;
>   
>   ​			sch->enqueue = ops->enqueue;
>   
>   ​			sch->dequeue = ops->dequeue;
>   
>   ​			sch->dev_queue = dev_queue;
>   
>   ​		sch->parent = parent;
>   
>   ​		sch->handle = qdisc_alloc_handle(dev);
>   
>   ​		/*for different TCA_OPTION, ops is different, such as sbq_qisc_ops, pfifo_qdisc_ops*/
>   
>   ​		ops->init(sch, tca[TCA_OPTIONS]); 
>   
>   ​			cbq_init //initializing the cbq queueing discipline. It sets up the classid of class, priority, siblings link, creates a default qdisc for the queueing discipline by calling the funciton qdisc_create_dflt().
>   
>   ​	/*graft qdisc "new" to class "classid" of qdisc "parent" or to device "dev"*/
>   
>   ​	qdisc_graft



cbq, pfifo,bfifo,  atm, blackhole, choke, codel, drr, dsmark, esfq, fq, fq_codel, gred, hfsc, prio, tbf, sfb...

> example: tc class add dev eth0 parent 1:1 classid 1:2 cbq bandwidth 10Mbit rate 8Mbit maxburst 20 allot 1514 prio 2 avpkt 1000 cell 8 weight 800Kbit split 1:0 bounded

# 12. quectel-CM

quectel-CM -s default -a 0 -p 1234 -4 -f /tmp/qcm_logfile

quectel-CM这个进程再与内核通信（driver/usb/serial/usb_wwan.c driver/net/usb/qmi_wwan.c）

因为是默认设置，所已就算用户的apn等信息是错的也是可以上网的。

## 12.1 数据结构

```c
typedef struct __PROFILE {
	char *qmichannel;
	char *usbnet_adapter;
	char *qmapnet_adapter;
	char *driver_name;
	int qmap_mode;
	int qmap_version;
	const char *apn; //
	const char *user; //
	const char *password; //
	const char *pincode; //
	int auth; //
	int pdp;
	int curIpFamily;
	int rawIP;
	int muxid;
	IPV4_T ipv4; //
	IPV6_T ipv6;
	int enable_ipv4;
	int enable_ipv6;
	int apntype;
	const struct qmi_device_ops *qmi_ops;
	} PROFILE_T;
	
typedef struct __IPV4 {
	uint32_t Address;
	uint32_t Gateway;
	uint32_t SubnetMask;
	uint32_t DnsPrimary;
	uint32_t DnsSecondary;
	uint32_t Mtu;
}IPV4_T;

typedef struct __IPV6 {
	UCHAR Address[16];
	UCHAR Gateway[16];
	UCHAR SubnetMask[16];
	UCHAR DnsPrimary[16];
	UCHAR DnsSecondary[16];
	UCHAR PrefixLengthIPAddr;
	UCHAR PrefixLengthGateway;
	ULONG Mtu;
	
}IPV6_T;
	
}

typedef struct _QCQMI_HDR
{
    UCHAR IFType;
    USHORT Length;
    UCHAR CtlFlags; //reserved
    UCHAR QMIType;
    UCHAR ClientId;
}QCQMI_HDR, *PQCQMI_HDR;

typedef struct _QMICTL_MSG
{
    //Message Header
    QCQMICTL_MSG_HDR QMICTLMsgHdr;
    QCQMICTL_MSG_MSG_HDR_RESP QMICTLMsgHdrRsp;
    
    //QMICTL Message
    QMICTL_SET_INSTANCE_ID_REQ_MSG SetInstanceIdReq;
    QMICTL_SET_INSTANCE_ID_RESP_MSG SetInstanceIdRsp;
    QMICTL_GET_VERSION_REQ_MSG GetVersionReq;
    QMICTL_GET_VERSION_RESP_MSG GetVersionRsp;
    QMICTL_GET_CLIENT_ID_REQ_MSG GetClientIdReq;
    QMICTL_GET_CLIENT_IDpRESQ_MSG GetClientIdRsq;
    .._RELEASE..
    ...REVOKE...
    ...INVALID...
    QMICTL_SET_DATA_FORMAT_REQ_MSG ...
    .......................RESP... ...
    QMICTL_SYNC_REQ_MSG
    ............RESP...
    ............IND....
    
}

typedef struct _QMUX_MSG
{
    QCQMUX_HDR QMUXHdr;
    union {
        //Message Header
        QCQMUX_MSG_HDR QMUXMsgHdr;
        QCQMUX_MSG_HDR_RESP QMUXMsgHdrResp;
        //QMIWDS Message
       	QMIWDS_GET_RUNTIME_SETTINGS_REQ_MSG GetRuntimeSettingsReq;
        QMIWDS_GET_RUNTIME..........RESP ...;
        ...SET_CLIENT_IP_FAMILY_PREF_REQ_MSG ...;
        .............................RESP...;
        QMIWDS_START_NETWORK_INTERFACE_REQ_MSG ...;
        ...............................RESP... ...;
        
    }
}

typedef struct _QCQMIMSG {
    QCQMI_HDR QMIHdr;
    union {
        QMICTL_MSG CTLMsg;
        QMUX_MSG MUXMsg;
    };
};
/*root@LBR1020:/# ls /sys/bus/usb/devices
1-0:1.0  2-1      2-1:1.1  2-1:1.3  3-0:1.0  usb1     usb3
2-0:1.0  2-1:1.0  2-1:1.2  2-1:1.4  4-0:1.0  usb2     usb4*/
qmidevice_detect(qmichannel, usbnet_adapter, sizeof(qmichannel));
 /*从/sys/bus/usb.devices/2-1:1.4中的net, usbmisc中获取到net:wwan0和device: cdc-wdm0
 *qmichannel:/dev/cdc-wdm0 usbnet_adapter:wwan0
 */
ql_qmap_mode_detect
    driver_name: qmi_wwan;
    qmap_mod: 1;
	muxid = 0x81;
	qmapnet_adapter = wwan0;
varify_driver
    /sys/class/net/wwan0/device/bInternetClass : ff;

qmi_main
    ql_sigaciton
    	SIGALRM:write(signal_control_fd[0], SIG_EVENT_START, sizeof());
		others: write(signal_control_fd[0], SIG_EVENT_STOP, sizeof());
				write(signal_control_fd[1], SIG_EVENT_STOP, sizeof());

	socketpair(AF_LOCAL, SOCK_STREAM, 0, signal_control_fd)
    socketpair(AF_LOCAL, SOCK_STREAM, 0, qmidevice_control_fd)
    main_loop:
		qmidevice_detect //wait for quectel module connect
            
    profile->qmi_ops = &qmiwwan_qmidev_ops;
	const struct qmi_device_ops qmiwwan_qmidev_ops = {
        .init = QmiWwanInit,
        .deinit = QmiWwanDeInit,
        .send = QmiWwanSendQMI,
        .read = QmiWwanThread,
    };
	profile->qmi_ops->send;
	pthread_create(&gQmiThreadID, 0, profile->qmi_ops->read, (void *)profile)
        /*QmiWwanThread,成功打开cdc_wdm0设备表明LTE模块已经连接*/
        open(/dev/cdc_wdm0, O_RDWR | O_NONBLOCK |O_NOCTTY);
		fcntl(cdc_wdm_fd, F_SETFL, fcntl(cdc_wdm_fd, F_GETFL) | O_NONBLOCK);
		fcntl(cdc_wdm_fd, F_SETFD, FD_CLOEXEC);
		qmidevice_send_event_to_main(RIL_INDICATE_DEVICE_CONNECTED);
			write(qmidevice_control_fd[1], &triger_event, sizeof(triger_event)); //发送到主线程中
		struct pollfd pollfds[] {{qmidevice_control_fd[1], POLLIN, 0},{cdc_wdm_fd, POLLIN, 0}};
		poll(pollfds, nevents, wait_for_request_quit ? 1000 : -1);
		fd= pollfds[ne].fd;
		fd == qmidevice_control_fd[1]:
			case RIL_REQUEST_QUIT:
				goto __QmiWwanThread_quit;
				break;
			case SIG_EVENT_STOP:
				wait_for_request_quit = 1;
			break;
		fd == cdc_wdm_fd
            PQCQMIMSG pResponse = (PQCQMIMSG)QMIBuf;
			read(fd, QMIBuf, sizeof(QMIBuf));
			QmiThreadRecvQMI(pResponse);
	QmiWwanInit
        QmiThreadSendQMITimeout(QMICTL_SYNC_REQ)
        	qmidev_send(pRequest)
        		QmiWwanSendQMI(pRequest)
        QmiThreadSendQMI(QMICTL_GET_VERSION_REQ, &pResponse)
        profile->qmap_version = pResponse->CTLMsg.GetVersionRsp...
        /*qmiclientId[]*/
		qmiclientId[*] = QmiWwanGetClientID(*)
    /*一般地，组包都是通过ComposeQMUXMsg()这个函数进行*/
    /*而发包都是通过QmiThreadSendQMI,最终调用到QmiWwanSendQMI发送*/
    /*需要获取信息时，也需要将需要先将所需信息组包然后发送到设备，并返回到pResponse中*/
    //config SIM
    qmierr= requestGetSIMstatus(&SIMStatus)
        while(QMI_ERR_OP_DEVICE_UNSUPPORTED)
        SIM_PIN:
				requestEnterSimPin(profile->pincode);
				
    //config apn
	requestSetProfile(profile)
    requestGetProfile(profile)
    
    requestRegistrationState(&PSAttachedState)
    send_signo_to_main(SIG_EVENT_CHECK)//发送给线程
    while()
    {
        send_signo_to_main(SIG_EVENT_CHECK);
        fd: signal_control_fd[1]:
        	read(fd, &signo, sizeof)
            switch(signo)
                case SIG_EVENT_START:
        			requetSetupDataCall(profile, IpFamilyV4);
        			requestGetIPAddress(profile, IpFamilyV4);
        			IPv4ConnectionStatus = QWDS_PKT_DATA_CONNECTED;
		        case SIG_EVENT_CHECK:
        			usbnet_link_change(1, profile); //
        				udhcpc_start(profile)
                            //对于这个模块中的dhcp是通过项目中udhcp的模块完成的
                            
    }

    
	
    
	
```

## 12.2 dirver

涉及模块usbserial 对于一个usbserial来说需要vendor,product,厂商ID和设备ID,它们的作用就是用来匹配。

对于发送过程：tty设备文件在获取了用户要求发送的数据之后传递到下层usbserial模块的核心层，而该核心层就是将数据打包成USB格式的数据并由USB通信发送到设备端去。

对于接收过程：usbserial模块会在该设备打开时就启动一个urb在那等待设备端发送数据过来，收到数据后就push到上层tty设备的缓冲中去，而tty设备在收到数据后就会给用户，或者直接显示在minicon之类的工具上。



option.c

```c
static struct usb_serial_driver option_1port_device = {
    .driver ={
        .owner = THIS_MODULE,
        .name = "option1",
    },
    .description = ...
    .id_table = option_ids,
    .num_ports = 1,
    .probe = option_probe,
    .open = usb_wwan_open,
    .close = usb_wan_close,
    .dtr_rts = usb_wwan_dtr_rts,
    .write = usb_wwan_write,
    .write_room =usb_wwan_write_room,
    ...
};

static struct usb_serial_driver * const serial_drivers[] = {
	&option_1port_device, NULL
};


/*option_ids为厂商ID和设备ID对于quectel EG18:USB_DEVICE(0x2C7C, 0x0512)*/
module_usb_serial_driver(serial_driver, option_ids) 
    usb_serial_module_driver(KBUILD_MODNAME, serial_driver, option_ids)
    	usb_serial_register_drivers(serial_drivers, KBUILD_MODNAME, serial_driver, option_ids)
    		1. 分配usb_driver结构体并初始化，然后注册；初始化drvwrap.driver并注册
    		2. 将serial_drivers中的的usb_driver指向上面初始化的usb_driver,然后注册serial_drivers到usb_serial_driver_list
    		3. udriver->id_table = id_table, driver_attach(&udriver->drvwrap.driver) //设置udriver的id_table and look for matches.
    		/*try to bind driver to devices*/
    		driver_attach(&udriver->drvwrap.driver)
    			bus_for_each_dev(drv->bus, NULL, drv, __driver_attach)
    			/*遍历next_device(&i),再执行以下函数*/
    				__driver_attach(dev, drv)
    
```



qmi_wwan.c

```c
module_usb_driver(qmi_wwan_driver);
	#define module_usb_driver(__usb_driver) module_driver(__usb_driver, usb_register, usb_deregister)
		usb_register(&qmi_wwan_driver)
            usb_register_driver(&qmi_wwan_driver, THIS_MODULE, KBUILD_ MODNAME)
            	
            
static struct usb_driver qmi_wwan_driver = {
	.name = "qmi_wwan",
	.id_table = products, //usb_device_id
	.probe = qmap_qmi_wwan_probe,
	.disconnect = qmap_qmi_wwan_disconnect,
	...
};
qmap_qmi_wwan_probe(struct usb_interface *intf, const struct usb_device_id *prod)
```

从代码来看，usb网卡和serial usb中都注册了相同的usb_device_id的usb_driver.

这样的化hub.c中有设备插入时，匹配到厂商ID和设备ID会调用两个驱动的probe函数还是只调用第一个匹配到的驱动的probe.

# 13. openwrt架构与Makefile

```makefile
world:
include $(TOPDIR)/include/host.mk
#first execute
ifneq ($(OPENWRT_BUILD),1)
	empty:=
	space:=$(empty) $(empty)
	_SINGLE=export MAKEFLAGS=$(space);
	#next time this part can't be executed
	override OPENWRT_BUILD=1
	export OPENWRT_BUILD
	include $(TOPDIR)/include/debug.mk
	include $(TOPDIR)/include/depends.mk
	include $(TOPDIR)/include/toplevel.mk #line 135 explain the rule of world
else
	include rules.mk
	include $(INCLUDE_DIR)/depends.mk
	include $(INCLUDE_DIR)/subdir.mk
#这四个子目录的Makefile实际上是不能独立执行。主要利用subdir.mk动态建立规则，诸如toolchain/stamp-install目标是靠subdir.mk的stampfile函数动态建立。在package/Makefile动态建立了package/stamp-prereq、package/stamp-cleanup、package/stamp-compile、package/stamp-install、package/stamp-rootfs-prepare目标
	include target/Makefile
	include package/Makefile
	include tools/Makefile
	include toolchain/Makefile

$(toolchain/stamp-install): $(tools/stamp-install)
$(target/stamp-compile): $(toolchain/stamp-install) $(tools/stamp-install) $(BUILD_DIR)/.prepared
$(package/stamp-cleanup): $(target/stamp-compile)
$(package/stamp-install): $(package/stamp-compile)
$(package/stamp-rootfs-prepare): $(package/stamp-install)
$(target/stamp-install): $(package/stamp-compile) $(package/stamp-install) $(package/stamp-rootfs-prepare)

printdb:
	@true
prepare: $(target/stamp-compile)

clean: FORCE
#应该是执行build_dir/target_$(broad)/package/clean?
	$(_SINGLE)$(SUBMAKE) target/linux/clean
	rm -rf $(BUILD_DIR) $(BIN_DIR) $(BUILD_LOG_DIR)

dirclean: clean
	rm -rf $(STAGING_DIR) $(STAGING_DIR_HOST) $(STAGING_DIR_TOOLCHAIN) $(TOOLCHAIN_DIR) $(BUILD_DIR_HOST) $(BUILD_DIR_TOOLCHAIN)
	rm -rf $(TMP_DIR)
	



```

> Parameters: <subdir> <name> <target> <depends> <config options> <stampfile location>
>
> $(eval $(call stampfile, $(curdir), target, prereq, .config))
>
> target/stamp-prereq:=$($(STAGING_DIR))/stamp/.target_prereq

1. test

2. training

3. port
4. double check and sync with r7800





