--- linux.org/net/unix/af_unix.c	Thu Aug 14 09:38:34 2014
+++ linux/net/unix/af_unix.c	Thu Jan 18 21:45:28 2018
@@ -316,6 +316,118 @@
 	return s;
 }
 
+/* Support code for asymmetrically connected dgram sockets
+ *
+ * If a datagram socket is connected to a socket not itself connected
+ * to the first socket (eg, /dev/log), clients may only enqueue more
+ * messages if the present receive queue of the server socket is not
+ * "too large". This means there's a second writeability condition
+ * poll and sendmsg need to test. The dgram recv code will do a wake
+ * up on the peer_wait wait queue of a socket upon reception of a
+ * datagram which needs to be propagated to sleeping would-be writers
+ * since these might not have sent anything so far. This can't be
+ * accomplished via poll_wait because the lifetime of the server
+ * socket might be less than that of its clients if these break their
+ * association with it or if the server socket is closed while clients
+ * are still connected to it and there's no way to inform "a polling
+ * implementation" that it should let go of a certain wait queue
+ *
+ * In order to propagate a wake up, a wait_queue_t of the client
+ * socket is enqueued on the peer_wait queue of the server socket
+ * whose wake function does a wake_up on the ordinary client socket
+ * wait queue. This connection is established whenever a write (or
+ * poll for write) hit the flow control condition and broken when the
+ * association to the server socket is dissolved or after a wake up
+ * was relayed.
+ */
+
+static int unix_dgram_peer_wake_relay(wait_queue_t *q, unsigned mode, int flags,
+				      void *key)
+{
+	struct unix_sock *u;
+	wait_queue_head_t *u_sleep;
+
+	u = container_of(q, struct unix_sock, peer_wake);
+
+	__remove_wait_queue(&unix_sk(u->peer_wake.private)->peer_wait,
+			    q);
+	u->peer_wake.private = NULL;
+
+	/* relaying can only happen while the wq still exists */
+	u_sleep = sk_sleep(&u->sk);
+	if (u_sleep)
+		wake_up_interruptible_poll(u_sleep, key);
+
+	return 0;
+}
+
+static int unix_dgram_peer_wake_connect(struct sock *sk, struct sock *other)
+{
+	struct unix_sock *u, *u_other;
+	int rc;
+
+	u = unix_sk(sk);
+	u_other = unix_sk(other);
+	rc = 0;
+	spin_lock(&u_other->peer_wait.lock);
+
+	if (!u->peer_wake.private) {
+		u->peer_wake.private = other;
+		__add_wait_queue(&u_other->peer_wait, &u->peer_wake);
+
+		rc = 1;
+	}
+
+	spin_unlock(&u_other->peer_wait.lock);
+	return rc;
+}
+
+static void unix_dgram_peer_wake_disconnect(struct sock *sk,
+					    struct sock *other)
+{
+	struct unix_sock *u, *u_other;
+
+	u = unix_sk(sk);
+	u_other = unix_sk(other);
+	spin_lock(&u_other->peer_wait.lock);
+
+	if (u->peer_wake.private == other) {
+		__remove_wait_queue(&u_other->peer_wait, &u->peer_wake);
+		u->peer_wake.private = NULL;
+	}
+
+	spin_unlock(&u_other->peer_wait.lock);
+}
+
+static void unix_dgram_peer_wake_disconnect_wakeup(struct sock *sk,
+						   struct sock *other)
+{
+	unix_dgram_peer_wake_disconnect(sk, other);
+	wake_up_interruptible_poll(sk_sleep(sk),
+				   POLLOUT |
+				   POLLWRNORM |
+				   POLLWRBAND);
+}
+
+/* preconditions:
+ *	- unix_peer(sk) == other
+ *	- association is stable
+ */
+static int unix_dgram_peer_wake_me(struct sock *sk, struct sock *other)
+{
+	int connected;
+
+	connected = unix_dgram_peer_wake_connect(sk, other);
+
+	if (unix_recvq_full(other))
+		return 1;
+
+	if (connected)
+		unix_dgram_peer_wake_disconnect(sk, other);
+
+	return 0;
+}
+
 static inline int unix_writable(struct sock *sk)
 {
 	return (atomic_read(&sk->sk_wmem_alloc) << 2) <= sk->sk_sndbuf;
@@ -420,6 +532,7 @@
 			skpair->sk_state_change(skpair);
 			sk_wake_async(skpair, SOCK_WAKE_WAITD, POLL_HUP);
 		}
+		unix_dgram_peer_wake_disconnect(sk, skpair);
 		sock_put(skpair); /* It may now die */
 		unix_peer(sk) = NULL;
 	}
@@ -653,6 +766,7 @@
 	INIT_LIST_HEAD(&u->link);
 	mutex_init(&u->readlock); /* single task reading lock */
 	init_waitqueue_head(&u->peer_wait);
+	init_waitqueue_func_entry(&u->peer_wake, unix_dgram_peer_wake_relay);
 	unix_insert_socket(unix_sockets_unbound(sk), sk);
 out:
 	if (sk == NULL)
@@ -1020,6 +1134,7 @@
 	if (unix_peer(sk)) {
 		struct sock *old_peer = unix_peer(sk);
 		unix_peer(sk) = other;
+		unix_dgram_peer_wake_disconnect_wakeup(sk, old_peer);
 		unix_state_double_unlock(sk, other);
 
 		if (other != old_peer)
@@ -1369,6 +1484,21 @@
 	sock_wfree(skb);
 }
 
+/*
+ * The "user->unix_inflight" variable is protected by the garbage
+ * collection lock, and we just read it locklessly here. If you go
+ * over the limit, there might be a tiny race in actually noticing
+ * it across threads. Tough.
+ */
+static inline bool too_many_unix_fds(struct task_struct *p)
+{
+	struct user_struct *user = current_user();
+
+	if (unlikely(user->unix_inflight > task_rlimit(p, RLIMIT_NOFILE)))
+		return !capable(CAP_SYS_RESOURCE) && !capable(CAP_SYS_ADMIN);
+	return false;
+}
+
 #define MAX_RECURSION_LEVEL 4
 
 static int unix_attach_fds(struct scm_cookie *scm, struct sk_buff *skb)
@@ -1377,6 +1507,8 @@
 	unsigned char max_level = 0;
 	int unix_sock_count = 0;
 
+	if (too_many_unix_fds(current))
+		return -ETOOMANYREFS;
 	for (i = scm->fp->count - 1; i >= 0; i--) {
 		struct sock *sk = unix_get_socket(scm->fp->fp[i]);
 
@@ -1398,10 +1530,8 @@
 	if (!UNIXCB(skb).fp)
 		return -ENOMEM;
 
-	if (unix_sock_count) {
-		for (i = scm->fp->count - 1; i >= 0; i--)
-			unix_inflight(scm->fp->fp[i]);
-	}
+	for (i = scm->fp->count - 1; i >= 0; i--)
+		unix_inflight(scm->fp->fp[i]);
 	return max_level;
 }
 
@@ -1459,6 +1589,7 @@
 	struct scm_cookie tmp_scm;
 	int max_level;
 	int data_len = 0;
+	int sk_locked;
 
 	if (NULL == siocb->scm)
 		siocb->scm = &tmp_scm;
@@ -1536,12 +1667,14 @@
 		goto out_free;
 	}
 
+	sk_locked = 0;
 	unix_state_lock(other);
+restart_locked:
 	err = -EPERM;
 	if (!unix_may_send(sk, other))
 		goto out_unlock;
 
-	if (sock_flag(other, SOCK_DEAD)) {
+	if (unlikely(sock_flag(other, SOCK_DEAD))) {
 		/*
 		 *	Check with 1003.1g - what should
 		 *	datagram error
@@ -1549,10 +1682,13 @@
 		unix_state_unlock(other);
 		sock_put(other);
 
+		if (!sk_locked)
+			unix_state_lock(sk);
+
 		err = 0;
-		unix_state_lock(sk);
 		if (unix_peer(sk) == other) {
 			unix_peer(sk) = NULL;
+			unix_dgram_peer_wake_disconnect_wakeup(sk, other);
 			unix_state_unlock(sk);
 
 			unix_dgram_disconnected(sk, other);
@@ -1578,20 +1714,37 @@
 			goto out_unlock;
 	}
 
-	if (unix_peer(other) != sk && unix_recvq_full(other)) {
-		if (!timeo) {
+	if (unlikely(unix_peer(other) != sk && unix_recvq_full(other))) {
+		if (timeo) {
+			timeo = unix_wait_for_peer(other, timeo);
+
+			err = sock_intr_errno(timeo);
+			if (signal_pending(current))
+				goto out_free;
+
+			goto restart;
+		}
+
+		if (!sk_locked) {
+			unix_state_unlock(other);
+			unix_state_double_lock(sk, other);
+		}
+
+		if (unix_peer(sk) != other ||
+		    unix_dgram_peer_wake_me(sk, other)) {
 			err = -EAGAIN;
+			sk_locked = 1;
 			goto out_unlock;
 		}
 
-		timeo = unix_wait_for_peer(other, timeo);
-
-		err = sock_intr_errno(timeo);
-		if (signal_pending(current))
-			goto out_free;
-
-		goto restart;
-	}
+		if (!sk_locked) {
+			sk_locked = 1;
+			goto restart_locked;
+		}
+	}
+
+	if (unlikely(sk_locked))
+		unix_state_unlock(sk);
 
 	if (sock_flag(other, SOCK_RCVTSTAMP))
 		__net_timestamp(skb);
@@ -1606,6 +1759,8 @@
 	return len;
 
 out_unlock:
+    if (sk_locked)
+		unix_state_unlock(sk);
 	unix_state_unlock(other);
 out_free:
 	kfree_skb(skb);
@@ -2241,14 +2396,16 @@
 		return mask;
 
 	writable = unix_writable(sk);
-	other = unix_peer_get(sk);
-	if (other) {
-		if (unix_peer(other) != sk) {
-			sock_poll_wait(file, &unix_sk(other)->peer_wait, wait);
-			if (unix_recvq_full(other))
-				writable = 0;
-		}
-		sock_put(other);
+	if (writable) {
+		unix_state_lock(sk);
+
+		other = unix_peer(sk);
+		if (other && unix_peer(other) != sk &&
+		    unix_recvq_full(other) &&
+		    unix_dgram_peer_wake_me(sk, other))
+			writable = 0;
+
+		unix_state_unlock(sk);
 	}
 
 	if (writable)
