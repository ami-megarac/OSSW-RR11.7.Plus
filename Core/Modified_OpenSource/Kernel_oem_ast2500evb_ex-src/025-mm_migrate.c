--- linux.org/mm/migrate.c	Thu Aug 14 09:38:34 2014
+++ linux/mm/migrate.c	Wed Jan 17 14:08:26 2018
@@ -37,6 +37,7 @@
 #include <linux/gfp.h>
 #include <linux/balloon_compaction.h>
 #include <linux/mmu_notifier.h>
+#include <linux/ptrace.h>
 
 #include <asm/tlbflush.h>
 
@@ -1454,8 +1455,7 @@
 		const void __user * __user *, pages,
 		const int __user *, nodes,
 		int __user *, status, int, flags)
-{
-	const struct cred *cred = current_cred(), *tcred;
+{	
 	struct task_struct *task;
 	struct mm_struct *mm;
 	int err;
@@ -1479,14 +1479,9 @@
 
 	/*
 	 * Check if this process has the right to modify the specified
-	 * process. The right exists if the process has administrative
-	 * capabilities, superuser privileges or the same
-	 * userid as the target process.
-	 */
-	tcred = __task_cred(task);
-	if (!uid_eq(cred->euid, tcred->suid) && !uid_eq(cred->euid, tcred->uid) &&
-	    !uid_eq(cred->uid,  tcred->suid) && !uid_eq(cred->uid,  tcred->uid) &&
-	    !capable(CAP_SYS_NICE)) {
+	 * process. Use the regular "ptrace_may_access()" checks.
+	 */
+	if (!ptrace_may_access(task, PTRACE_MODE_READ_REALCREDS)) {
 		rcu_read_unlock();
 		err = -EPERM;
 		goto out;
