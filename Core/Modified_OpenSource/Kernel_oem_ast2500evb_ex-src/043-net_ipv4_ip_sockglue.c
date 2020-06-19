--- linux.org/net/ipv4/ip_sockglue.c	Thu Aug 14 09:38:34 2014
+++ linux/net/ipv4/ip_sockglue.c	Thu Jan 18 21:30:37 2018
@@ -1064,7 +1064,14 @@
 		pktinfo->ipi_ifindex = 0;
 		pktinfo->ipi_spec_dst.s_addr = 0;
 	}
-	skb_dst_drop(skb);
+	/* We need to keep the dst for __ip_options_echo()
+	 * We could restrict the test to opt.ts_needtime || opt.srr,
+	 * but the following is good enough as IP options are not often used.
+	 */
+	if (unlikely(IPCB(skb)->opt.optlen))
+		skb_dst_force(skb);
+	else
+		skb_dst_drop(skb);
 }
 
 int ip_setsockopt(struct sock *sk, int level,
