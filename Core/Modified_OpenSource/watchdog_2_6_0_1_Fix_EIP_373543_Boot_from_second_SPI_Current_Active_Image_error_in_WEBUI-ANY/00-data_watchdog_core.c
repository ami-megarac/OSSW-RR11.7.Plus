--- .pristine/watchdog-2.6.0-src/data/watchdog_core.c	Fri Dec 15 14:32:43 2017
+++ source/watchdog-2.6.0-src/data/watchdog_core.c	Thu Jan  4 17:46:48 2018
@@ -73,7 +73,10 @@
 {
 	
 	 int len=0;
-
+#ifdef CONFIG_SPX_FEATURE_HW_FAILSAFE_BOOT
+	 int len_status=0;
+	 char watchdog_status[256] = {0};
+#endif
 /* We don't support seeked reads. read should start from 0 */	
 #if (LINUX_VERSION_CODE > KERNEL_VERSION(3,4,11))
 	if (*offset != 0)
@@ -90,7 +93,13 @@
 #endif
 
 	 len = sprintf(buf, "%d\n", watchdog_timeout);
-
+#ifdef CONFIG_SPX_FEATURE_HW_FAILSAFE_BOOT
+	 if (watchdog_hal->hal_ops->get_status)
+	 {	
+		 len_status = sprintf(watchdog_status, "%d\n", watchdog_hal->hal_ops->get_status());
+		 len += snprintf(&buf[len], (len + len_status), "%s", watchdog_status);
+	 }
+#endif
 #if (LINUX_VERSION_CODE > KERNEL_VERSION(3,4,11))
 	 *offset +=len;
 #endif
