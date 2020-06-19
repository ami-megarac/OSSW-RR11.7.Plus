--- linux.org/net/unix/garbage.c	Thu Aug 14 09:38:34 2014
+++ linux/net/unix/garbage.c	Thu Jan 18 21:43:23 2018
@@ -125,9 +125,9 @@
 void unix_inflight(struct file *fp)
 {
 	struct sock *s = unix_get_socket(fp);
+	spin_lock(&unix_gc_lock);
 	if (s) {
 		struct unix_sock *u = unix_sk(s);
-		spin_lock(&unix_gc_lock);
 		if (atomic_long_inc_return(&u->inflight) == 1) {
 			BUG_ON(!list_empty(&u->link));
 			list_add_tail(&u->link, &gc_inflight_list);
@@ -135,22 +135,24 @@
 			BUG_ON(list_empty(&u->link));
 		}
 		unix_tot_inflight++;
-		spin_unlock(&unix_gc_lock);
-	}
+	}
+	fp->f_cred->user->unix_inflight++;
+	spin_unlock(&unix_gc_lock);
 }
 
 void unix_notinflight(struct file *fp)
 {
 	struct sock *s = unix_get_socket(fp);
+	spin_lock(&unix_gc_lock);
 	if (s) {
 		struct unix_sock *u = unix_sk(s);
-		spin_lock(&unix_gc_lock);
 		BUG_ON(list_empty(&u->link));
 		if (atomic_long_dec_and_test(&u->inflight))
 			list_del_init(&u->link);
-		unix_tot_inflight--;
-		spin_unlock(&unix_gc_lock);
-	}
+		unix_tot_inflight--;		
+	}
+	fp->f_cred->user->unix_inflight--;
+	spin_unlock(&unix_gc_lock);
 }
 
 static void scan_inflight(struct sock *x, void (*func)(struct unix_sock *),
