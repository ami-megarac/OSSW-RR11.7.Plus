--- linux.org/fs/proc/proc_sysctl.c	Thu Aug 14 09:38:34 2014
+++ linux/fs/proc/proc_sysctl.c	Fri Jan 19 11:28:57 2018
@@ -666,7 +666,7 @@
 	ctl_dir = container_of(head, struct ctl_dir, header);
 
 	if (!dir_emit_dots(file, ctx))
-		return 0;
+		goto out;
 
 	pos = 2;
 
@@ -676,6 +676,7 @@
 			break;
 		}
 	}
+out:
 	sysctl_head_finish(head);
 	return 0;
 }
