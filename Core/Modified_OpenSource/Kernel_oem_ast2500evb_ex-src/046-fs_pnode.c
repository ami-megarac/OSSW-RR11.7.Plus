--- linux.org/fs/pnode.c	Thu Aug 14 09:38:34 2014
+++ linux/fs/pnode.c	Fri Jan 19 10:39:30 2018
@@ -198,7 +198,7 @@
 
 /* all accesses are serialized by namespace_sem */
 static struct user_namespace *user_ns;
-static struct mount *last_dest, *last_source, *dest_master;
+static struct mount *last_dest, *first_source, *last_source, *dest_master;
 static struct mountpoint *mp;
 static struct hlist_head *list;
 
@@ -216,20 +216,22 @@
 		type = CL_MAKE_SHARED;
 	} else {
 		struct mount *n, *p;
+		bool done;
 		for (n = m; ; n = p) {
 			p = n->mnt_master;
-			if (p == dest_master || IS_MNT_MARKED(p)) {
-				while (last_dest->mnt_master != p) {
-					last_source = last_source->mnt_master;
-					last_dest = last_source->mnt_parent;
-				}
-				if (n->mnt_group_id != last_dest->mnt_group_id) {
-					last_source = last_source->mnt_master;
-					last_dest = last_source->mnt_parent;
-				}
-				break;
-			}
+			if (p == dest_master || IS_MNT_MARKED(p))
+				break;
 		}
+		do {
+			struct mount *parent = last_source->mnt_parent;
+			if (last_source == first_source)
+				break;
+			done = parent->mnt_master == p;
+			if (done)
+				break;
+			last_source = last_source->mnt_master;
+		} while (!done);
+		
 		type = CL_SLAVE;
 		/* beginning of peer group among the slaves? */
 		if (IS_MNT_SHARED(m))
@@ -280,6 +282,7 @@
 	 */
 	user_ns = current->nsproxy->mnt_ns->user_ns;
 	last_dest = dest_mnt;
+	first_source = source_mnt;
 	last_source = source_mnt;
 	mp = dest_mp;
 	list = tree_list;
