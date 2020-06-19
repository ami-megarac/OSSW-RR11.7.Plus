--- /home/nico/MDS/MDS4.0.2/workspace/ast2500evb_workspace/Build/kernel/linux.org/net/ceph/auth_x.c	Thu Aug 14 09:38:34 2014
+++ linux/net/ceph/auth_x.c	Tue Jan 23 16:27:39 2018
@@ -13,7 +13,6 @@
 #include "auth_x.h"
 #include "auth_x_protocol.h"
 
-#define TEMP_TICKET_BUF_LEN	256
 
 static void ceph_x_validate_tickets(struct ceph_auth_client *ac, int *pneed);
 
@@ -64,7 +63,7 @@
 }
 
 static int ceph_x_decrypt(struct ceph_crypto_key *secret,
-			  void **p, void *end, void *obuf, size_t olen)
+			  void **p, void *end, void **obuf, size_t olen)
 {
 	struct ceph_x_encrypt_header head;
 	size_t head_len = sizeof(head);
@@ -75,8 +74,14 @@
 		return -EINVAL;
 
 	dout("ceph_x_decrypt len %d\n", len);
-	ret = ceph_decrypt2(secret, &head, &head_len, obuf, &olen,
-			    *p, len);
+	if (*obuf == NULL) {
+		*obuf = kmalloc(len, GFP_NOFS);
+		if (!*obuf)
+			return -ENOMEM;
+		olen = len;
+	}
+
+	ret = ceph_decrypt2(secret, &head, &head_len, *obuf, &olen, *p, len);
 	if (ret)
 		return ret;
 	if (head.struct_v != 1 || le64_to_cpu(head.magic) != CEPHX_ENC_MAGIC)
@@ -137,18 +142,8 @@
 	int num;
 	void *p = buf;
 	int ret;
-	char *dbuf;
-	char *ticket_buf;
 	u8 reply_struct_v;
-
-	dbuf = kmalloc(TEMP_TICKET_BUF_LEN, GFP_NOFS);
-	if (!dbuf)
-		return -ENOMEM;
-
-	ret = -ENOMEM;
-	ticket_buf = kmalloc(TEMP_TICKET_BUF_LEN, GFP_NOFS);
-	if (!ticket_buf)
-		goto out_dbuf;
+	
 
 	ceph_decode_need(&p, end, 1 + sizeof(u32), bad);
 	reply_struct_v = ceph_decode_8(&p);
@@ -160,11 +155,13 @@
 		int type;
 		u8 tkt_struct_v, blob_struct_v;
 		struct ceph_x_ticket_handler *th;
+		void *dbuf = NULL;
 		void *dp, *dend;
 		int dlen;
 		char is_enc;
 		struct timespec validity;
 		struct ceph_crypto_key old_key;
+		void *ticket_buf = NULL;
 		void *tp, *tpend;
 		struct ceph_timespec new_validity;
 		struct ceph_crypto_key new_session_key;
@@ -188,8 +185,7 @@
 		}
 
 		/* blob for me */
-		dlen = ceph_x_decrypt(secret, &p, end, dbuf,
-				      TEMP_TICKET_BUF_LEN);
+		dlen = ceph_x_decrypt(secret, &p, end, &dbuf, 0);
 		if (dlen <= 0) {
 			ret = dlen;
 			goto out;
@@ -216,20 +212,26 @@
 
 		/* ticket blob for service */
 		ceph_decode_8_safe(&p, end, is_enc, bad);
-		tp = ticket_buf;
+
 		if (is_enc) {
 			/* encrypted */
 			dout(" encrypted ticket\n");
-			dlen = ceph_x_decrypt(&old_key, &p, end, ticket_buf,
-					      TEMP_TICKET_BUF_LEN);
+			dlen = ceph_x_decrypt(&old_key, &p, end, &ticket_buf, 0);
 			if (dlen < 0) {
 				ret = dlen;
 				goto out;
 			}
+			tp = ticket_buf;
 			dlen = ceph_decode_32(&tp);
 		} else {
 			/* unencrypted */
 			ceph_decode_32_safe(&p, end, dlen, bad);
+			ticket_buf = kmalloc(dlen, GFP_NOFS);
+		if (!ticket_buf) {
+			ret = -ENOMEM;
+			goto out;
+		}
+		tp = ticket_buf;
 			ceph_decode_need(&p, end, dlen, bad);
 			ceph_decode_copy(&p, ticket_buf, dlen);
 		}
@@ -261,7 +263,6 @@
 	ret = 0;
 out:
 	kfree(ticket_buf);
-out_dbuf:
 	kfree(dbuf);
 	return ret;
 
@@ -583,13 +584,14 @@
 	struct ceph_x_ticket_handler *th;
 	int ret = 0;
 	struct ceph_x_authorize_reply reply;
+	void *preply = &reply;
 	void *p = au->reply_buf;
 	void *end = p + sizeof(au->reply_buf);
 
 	th = get_ticket_handler(ac, au->service);
 	if (IS_ERR(th))
 		return PTR_ERR(th);
-	ret = ceph_x_decrypt(&th->session_key, &p, end, &reply, sizeof(reply));
+	ret = ceph_x_decrypt(&th->session_key, &p, end, &preply, sizeof(reply));
 	if (ret < 0)
 		return ret;
 	if (ret != sizeof(reply))
