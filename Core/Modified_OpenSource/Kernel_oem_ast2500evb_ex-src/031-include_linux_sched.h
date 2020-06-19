--- linux.org/include/linux/sched.h	Thu Aug 14 09:38:34 2014
+++ linux/include/linux/sched.h	Fri Jan 19 09:54:32 2018
@@ -751,7 +751,7 @@
 	unsigned long mq_bytes;	/* How many bytes can be allocated to mqueue? */
 #endif
 	unsigned long locked_shm; /* How many pages of mlocked shm ? */
-
+    unsigned long unix_inflight; /*How many files in flight in unix sockets*/
 #ifdef CONFIG_KEYS
 	struct key *uid_keyring;	/* UID specific keyring */
 	struct key *session_keyring;	/* UID's default session keyring */
