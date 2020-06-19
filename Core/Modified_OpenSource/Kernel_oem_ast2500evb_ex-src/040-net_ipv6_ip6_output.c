--- linux.org/net/ipv6/ip6_output.c	Thu Aug 14 09:38:34 2014
+++ linux/net/ipv6/ip6_output.c	Thu Jan 18 21:49:48 2018
@@ -1366,6 +1366,11 @@
 			 */
 			alloclen += sizeof(struct frag_hdr);
 
+			copy = datalen - transhdrlen - fraggap;
+			if (copy < 0) {
+				err = -EINVAL;
+				goto error;
+			}
 			if (transhdrlen) {
 				skb = sock_alloc_send_skb(sk,
 						alloclen + hh_len,
@@ -1418,13 +1423,9 @@
 				data += fraggap;
 				pskb_trim_unique(skb_prev, maxfraglen);
 			}
-			copy = datalen - transhdrlen - fraggap;
-
-			if (copy < 0) {
-				err = -EINVAL;
-				kfree_skb(skb);
-				goto error;
-			} else if (copy > 0 && getfrag(from, data + transhdrlen, offset, copy, fraggap, skb) < 0) {
+			if (copy > 0 &&
+			    getfrag(from, data + transhdrlen, offset,
+				    copy, fraggap, skb) < 0) {
 				err = -EFAULT;
 				kfree_skb(skb);
 				goto error;
