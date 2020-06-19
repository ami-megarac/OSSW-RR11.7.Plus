--- linux.org/include/net/tcp.h	Thu Aug 14 09:38:34 2014
+++ linux/include/net/tcp.h	Fri Jan 19 10:43:17 2018
@@ -1388,6 +1388,8 @@
 {
 	if (sk->sk_send_head == skb_unlinked)
 		sk->sk_send_head = NULL;
+	if (tcp_sk(sk)->highest_sack == skb_unlinked)
+		tcp_sk(sk)->highest_sack = NULL;
 }
 
 static inline void tcp_init_send_head(struct sock *sk)
