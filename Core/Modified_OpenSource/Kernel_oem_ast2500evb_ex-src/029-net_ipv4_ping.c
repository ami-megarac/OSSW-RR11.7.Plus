--- linux.org/net/ipv4/ping.c	Thu Aug 14 09:38:34 2014
+++ linux/net/ipv4/ping.c	Wed Jan 17 17:20:49 2018
@@ -158,6 +158,7 @@
 	if (sk_hashed(sk)) {
 		write_lock_bh(&ping_table.lock);
 		hlist_nulls_del(&sk->sk_nulls_node);
+		sk_nulls_node_init(&sk->sk_nulls_node);
 		sock_put(sk);
 		isk->inet_num = 0;
 		isk->inet_sport = 0;
