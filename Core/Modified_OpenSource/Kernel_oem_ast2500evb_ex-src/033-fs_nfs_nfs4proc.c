--- linux.org/fs/nfs/nfs4proc.c	Thu Aug 14 09:38:34 2014
+++ linux/fs/nfs/nfs4proc.c	Thu Jan 18 21:11:47 2018
@@ -8347,6 +8347,7 @@
 	.reboot_recovery_ops = &nfs41_reboot_recovery_ops,
 	.nograce_recovery_ops = &nfs41_nograce_recovery_ops,
 	.state_renewal_ops = &nfs41_state_renewal_ops,
+	.mig_recovery_ops = &nfs41_mig_recovery_ops,
 };
 #endif
 
