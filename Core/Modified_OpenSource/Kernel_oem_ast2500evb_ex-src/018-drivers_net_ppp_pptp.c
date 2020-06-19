--- linux.org/drivers/net/ppp/pptp.c	Thu Aug 14 09:38:34 2014
+++ linux/drivers/net/ppp/pptp.c	Tue Jan 16 15:48:36 2018
@@ -420,6 +420,9 @@
 	struct pptp_opt *opt = &po->proto.pptp;
 	int error = 0;
 
+	if (sockaddr_len < sizeof(struct sockaddr_pppox))
+		return -EINVAL;
+
 	lock_sock(sk);
 
 	opt->src_addr = sp->sa_addr.pptp;
@@ -440,6 +443,9 @@
 	struct rtable *rt;
 	struct flowi4 fl4;
 	int error = 0;
+
+	if (sockaddr_len < sizeof(struct sockaddr_pppox))
+		return -EINVAL;
 
 	if (sp->sa_protocol != PX_PROTO_PPTP)
 		return -EINVAL;
