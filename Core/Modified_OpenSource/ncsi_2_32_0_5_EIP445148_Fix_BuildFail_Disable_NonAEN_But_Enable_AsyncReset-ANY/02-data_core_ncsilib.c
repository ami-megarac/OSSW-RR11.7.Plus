--- .pristine/ncsi-2.32.0-src/data/core/ncsilib.c	Tue Dec  4 12:08:16 2018
+++ source/ncsi-2.32.0-src/data/core/ncsilib.c	Wed Dec  5 17:21:31 2018
@@ -183,7 +183,8 @@
 	return;
 }
 
-#ifdef CONFIG_SPX_FEATURE_NCSI_GET_LINK_STATUS_FOR_NON_AEN_SUPPORTED_CONTROLLERS
+#if defined(CONFIG_SPX_FEATURE_NCSI_GET_LINK_STATUS_FOR_NON_AEN_SUPPORTED_CONTROLLERS) ||\
+	defined(CONFIG_SPX_FEATURE_POLL_FOR_ASYNC_RESET ) 
 /*
 *@fn CheckAENSupport
 *@brief This function is used to check whether the NC-SI controller supports AEN Packets or not
