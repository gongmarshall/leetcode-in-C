ebtables -t filter -F;ebtables -t filter -X
#
ebtables -t filter -N INPUT -P ACCEPT
ebtables -t filter -N FORWARD -P ACCEPT
ebtables -t filter -N OUTPUT -P ACCEPT
ebtables -t filter -N dhcp -P DROP
ebtables -t filter -N pptp -P DROP
ebtables -t filter -N l2tp -P DROP
ebtables -t filter -N pppoe -P DROP
ebtables -t filter -A INPUT -p PPP_DISC -j pppoe
ebtables -t filter -A INPUT -p IPv4 --ip-proto udp --ip-dport 1701 -j l2tp
ebtables -t filter -A INPUT -p IPv4 --ip-proto tcp --ip-dport 1723 -j pptp
ebtables -t filter -A INPUT -p IPv4 --ip-proto udp --ip-dport 67:69 -j dhcp
ebtables -t filter -A dhcp -s 60:66:7:11:2a:21 -j ACCEPT
ebtables -t filter -A dhcp -s 20:14:7:88:2a:21 -j ACCEPT
ebtables -t filter -A dhcp -s b0:39:56:91:54:15 -j ACCEPT
ebtables -t filter -A dhcp -s 0:14:7:11:2a:99 -j ACCEPT
ebtables -t filter -A dhcp -s bc:a5:11:3a:6:58 -j ACCEPT
ebtables -t filter -A dhcp -s 78:d2:94:ae:c9:7d -j ACCEPT
ebtables -t filter -A dhcp -s 30:24:7:11:2a:21 -j ACCEPT
ebtables -t filter -A dhcp -s 78:d2:94:ae:c2:1e -j ACCEPT
ebtables -t filter -A dhcp -s 20:14:7:12:2c:31 -j ACCEPT
ebtables -t filter -A dhcp -s bc:a5:11:3a:6:58 -j ACCEPT
ebtables -t filter -A dhcp -s 78:d2:94:ae:c9:7d -j ACCEPT
ebtables -t filter -A dhcp -s 8c:3b:ad:19:30:29 -j ACCEPT
ebtables -t filter -A dhcp -s 12:73:b5:1:87:df -j ACCEPT
ebtables -t filter -A dhcp -s 8c:3b:ad:15:2:9b -j ACCEPT
ebtables -t filter -A pptp -s 60:66:7:11:2a:21 -j ACCEPT
ebtables -t filter -A pptp -s 0:14:7:11:2a:99 -j ACCEPT
ebtables -t filter -A pptp -s b0:39:56:91:54:15 -j ACCEPT
ebtables -t filter -A pptp -s 20:14:7:88:2a:21 -j ACCEPT
ebtables -t filter -A pptp -s bc:a5:11:3a:6:58 -j ACCEPT
ebtables -t filter -A pptp -s 78:d2:94:ae:c9:7d -j ACCEPT
ebtables -t filter -A pptp -s 30:24:7:11:2a:21 -j ACCEPT
ebtables -t filter -A pptp -s 78:d2:94:ae:c2:1e -j ACCEPT
ebtables -t filter -A pptp -s 20:14:7:12:2c:31 -j ACCEPT
ebtables -t filter -A pptp -s bc:a5:11:3a:6:58 -j ACCEPT
ebtables -t filter -A pptp -s 78:d2:94:ae:c9:7d -j ACCEPT
ebtables -t filter -A pptp -s 8c:3b:ad:19:30:29 -j ACCEPT
ebtables -t filter -A pptp -s 12:73:b5:1:87:df -j ACCEPT
ebtables -t filter -A pptp -s 8c:3b:ad:15:2:9b -j ACCEPT
ebtables -t filter -A l2tp -s 60:66:7:11:2a:21 -j ACCEPT
ebtables -t filter -A l2tp -s 0:14:7:11:2a:99 -j ACCEPT
ebtables -t filter -A l2tp -s b0:39:56:91:54:15 -j ACCEPT
ebtables -t filter -A l2tp -s 20:14:7:88:2a:21 -j ACCEPT
ebtables -t filter -A l2tp -s bc:a5:11:3a:6:58 -j ACCEPT
ebtables -t filter -A l2tp -s 78:d2:94:ae:c9:7d -j ACCEPT
ebtables -t filter -A l2tp -s 30:24:7:11:2a:21 -j ACCEPT
ebtables -t filter -A l2tp -s 78:d2:94:ae:c2:1e -j ACCEPT
ebtables -t filter -A l2tp -s 20:14:7:12:2c:31 -j ACCEPT
ebtables -t filter -A l2tp -s bc:a5:11:3a:6:58 -j ACCEPT
ebtables -t filter -A l2tp -s 78:d2:94:ae:c9:7d -j ACCEPT
ebtables -t filter -A l2tp -s 8c:3b:ad:19:30:29 -j ACCEPT
ebtables -t filter -A l2tp -s 12:73:b5:1:87:df -j ACCEPT
ebtables -t filter -A l2tp -s 8c:3b:ad:15:2:9b -j ACCEPT
ebtables -t filter -A pppoe -s 60:66:7:11:2a:21 -j ACCEPT
ebtables -t filter -A pppoe -s 0:14:7:11:2a:99 -j ACCEPT
ebtables -t filter -A pppoe -s b0:39:56:91:54:15 -j ACCEPT
ebtables -t filter -A pppoe -s 20:14:7:88:2a:21 -j ACCEPT
ebtables -t filter -A pppoe -s bc:a5:11:3a:6:58 -j ACCEPT
ebtables -t filter -A pppoe -s 78:d2:94:ae:c9:7d -j ACCEPT
ebtables -t filter -A pppoe -s 30:24:7:11:2a:21 -j ACCEPT
ebtables -t filter -A pppoe -s 78:d2:94:ae:c2:1e -j ACCEPT
ebtables -t filter -A pppoe -s 20:14:7:12:2c:31 -j ACCEPT
ebtables -t filter -A pppoe -s bc:a5:11:3a:6:58 -j ACCEPT
ebtables -t filter -A pppoe -s 78:d2:94:ae:c9:7d -j ACCEPT
ebtables -t filter -A pppoe -s 8c:3b:ad:19:30:29 -j ACCEPT
ebtables -t filter -A pppoe -s 12:73:b5:1:87:df -j ACCEPT
ebtables -t filter -A pppoe -s 8c:3b:ad:15:2:9b -j ACCEPT

