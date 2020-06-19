--- linux.org/net/ipv6/output_core.c	Thu Aug 14 09:38:34 2014
+++ linux/net/ipv6/output_core.c	Thu Jan 18 21:51:34 2018
@@ -10,7 +10,7 @@
 
 int ip6_find_1stfragopt(struct sk_buff *skb, u8 **nexthdr)
 {
-	u16 offset = sizeof(struct ipv6hdr);
+	unsigned int offset = sizeof(struct ipv6hdr);
 	struct ipv6_opt_hdr *exthdr =
 				(struct ipv6_opt_hdr *)(ipv6_hdr(skb) + 1);
 	unsigned int packet_len = skb_tail_pointer(skb) -
@@ -20,6 +20,7 @@
 
 	while (offset + 1 <= packet_len) {
 
+		unsigned int len;
 		switch (**nexthdr) {
 
 		case NEXTHDR_HOP:
@@ -39,7 +40,10 @@
 			return offset;
 		}
 
-		offset += ipv6_optlen(exthdr);
+		len = ipv6_optlen(exthdr);
+		if (len + offset >= IPV6_MAXPLEN)
+			return -EINVAL;
+		offset += len;
 		*nexthdr = &exthdr->nexthdr;
 		exthdr = (struct ipv6_opt_hdr *)(skb_network_header(skb) +
 						 offset);
