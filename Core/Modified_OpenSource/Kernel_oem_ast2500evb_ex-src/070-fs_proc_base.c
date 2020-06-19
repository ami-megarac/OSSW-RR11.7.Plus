--- linux.org/fs/proc/base.c	Thu Aug 14 09:38:34 2014
+++ linux/fs/proc/base.c	Wed Jan 24 15:22:51 2018
@@ -844,7 +844,8 @@
 	int ret = 0;
 	struct mm_struct *mm = file->private_data;
 
-	if (!mm)
+	/* Ensure the process spawned far enough to have an environment. */
+	if (!mm || !mm->env_end)
 		return 0;
 
 	page = (char *)__get_free_page(GFP_TEMPORARY);
