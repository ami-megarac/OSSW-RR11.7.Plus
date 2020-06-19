--- linux.org/net/ipv4/tcp.c	Thu Aug 14 09:38:34 2014
+++ linux/net/ipv4/tcp.c	Fri Jan 19 11:13:53 2018
@@ -2355,6 +2355,10 @@
 	tcp_set_ca_state(sk, TCP_CA_Open);
 	tcp_clear_retrans(tp);
 	inet_csk_delack_init(sk);
+	/* Initialize rcv_mss to TCP_MIN_MSS to avoid division by 0
+	 * issue in __tcp_select_window()
+	 */
+	icsk->icsk_ack.rcv_mss = TCP_MIN_MSS;
 	tcp_init_send_head(sk);
 	memset(&tp->rx_opt, 0, sizeof(tp->rx_opt));
 	__sk_dst_reset(sk);
