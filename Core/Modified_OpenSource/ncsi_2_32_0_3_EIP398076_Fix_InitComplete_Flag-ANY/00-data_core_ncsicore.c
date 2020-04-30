--- .pristine/ncsi-2.32.0-src/data/core/ncsicore.c	Mon Apr  2 14:32:06 2018
+++ source/ncsi-2.32.0-src/data/core/ncsicore.c	Mon Apr  2 14:33:25 2018
@@ -1144,6 +1144,7 @@
 		{
 			printk(KERN_DEBUG "NCSI: Same mode, do nothing\n");
 			kfree((void *)data);
+			InitComplete = 1;
 			return;
 		}
 		info->vlanID = work->VLANId;
