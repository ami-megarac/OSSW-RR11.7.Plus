--- .pristine/helper-2.26.0-src/data/helpmain.c	Tue Oct 31 21:57:30 2017
+++ source/helper-2.26.0-src/data/helpmain.c	Tue Oct 31 23:16:59 2017
@@ -160,7 +160,9 @@
 	{0}		
 #endif
 };
-
+#ifdef CONFIG_SPX_FEATURE_HW_FAILSAFE_BOOT
+extern unsigned char broken_spi_banks;// specify bank#0 of SPI Flash is broken or not
+#endif
 
 #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36))
 static int proc_jiffies(struct ctl_table *ctl,int write,void *buffer,size_t *lenp,loff_t* pos)
@@ -208,7 +210,12 @@
 
 #ifdef CONFIG_SPX_FEATURE_GLOBAL_DUAL_IMAGE_SUPPORT
 	fwinfo1_proc = AddProcEntry(moduledir,"FwInfo1",fwinfo1_read,NULL,NULL);
-	fwinfo2_proc = AddProcEntry(moduledir,"FwInfo2",fwinfo2_read,NULL,NULL);
+#ifdef CONFIG_SPX_FEATURE_HW_FAILSAFE_BOOT
+	if (broken_spi_banks == 0)
+#endif
+	{
+		fwinfo2_proc = AddProcEntry(moduledir,"FwInfo2",fwinfo2_read,NULL,NULL);
+	}
 #endif
 
     fbinfo_proc = AddProcEntry(moduledir,"FbInfo",flashbanksize_read,NULL,NULL);	
@@ -231,7 +238,12 @@
     RemoveProcEntry(fbinfo_proc);
 #ifdef CONFIG_SPX_FEATURE_GLOBAL_DUAL_IMAGE_SUPPORT
 	RemoveProcEntry(fwinfo1_proc);
-	RemoveProcEntry(fwinfo2_proc);
+#ifdef CONFIG_SPX_FEATURE_HW_FAILSAFE_BOOT
+	if (broken_spi_banks == 0)
+#endif
+	{
+		RemoveProcEntry(fwinfo2_proc);
+	}
 #endif
 
 #else
@@ -240,7 +252,12 @@
     RemoveProcEntry(moduledir,"FbInfo");
 #ifdef CONFIG_SPX_FEATURE_GLOBAL_DUAL_IMAGE_SUPPORT
 	RemoveProcEntry(moduledir,"FwInfo1");
-	RemoveProcEntry(moduledir,"FwInfo2");
+#ifdef CONFIG_SPX_FEATURE_HW_FAILSAFE_BOOT
+	if (broken_spi_banks == 0)
+#endif
+	{
+		RemoveProcEntry(moduledir,"FwInfo2");
+	}
 #endif
 	
 #endif
