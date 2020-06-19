--- /home/nico/MDS/MDS4.0.2/workspace/ast2500evb_workspace/Build/kernel/linux.org/ipc/shm.c	Thu Aug 14 09:38:34 2014
+++ linux/ipc/shm.c	Tue Jan 23 16:21:23 2018
@@ -1062,8 +1062,13 @@
 		goto out;
 	else if ((addr = (ulong)shmaddr)) {
 		if (addr & (shmlba - 1)) {
-			if (shmflg & SHM_RND)
-				addr &= ~(shmlba - 1);	   /* round down */
+			/*  
+			 * Round down to the nearest multiple of shmlba.  
+			 * For sane do_mmap_pgoff() parameters, avoid  
+			 * round downs that trigger nil-page and MAP_FIXED.  
+			 */  
+			if ((shmflg & SHM_RND) && addr >= shmlba)  
+				addr &= ~(shmlba - 1); 
 			else
 #ifndef __ARCH_FORCE_SHMLBA
 				if (addr & ~PAGE_MASK)
