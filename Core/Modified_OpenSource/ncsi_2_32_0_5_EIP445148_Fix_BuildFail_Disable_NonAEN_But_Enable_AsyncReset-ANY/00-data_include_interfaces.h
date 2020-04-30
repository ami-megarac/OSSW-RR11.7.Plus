--- .pristine/ncsi-2.32.0-src/data/include/interfaces.h	Tue Dec  4 12:08:16 2018
+++ source/ncsi-2.32.0-src/data/include/interfaces.h	Wed Dec  5 17:12:36 2018
@@ -125,7 +125,8 @@
 	UINT8 Flowcontrol;
 } SetFlowcontrolReq_T;
 
-#ifdef CONFIG_SPX_FEATURE_NCSI_GET_LINK_STATUS_FOR_NON_AEN_SUPPORTED_CONTROLLERS
+#if defined(CONFIG_SPX_FEATURE_NCSI_GET_LINK_STATUS_FOR_NON_AEN_SUPPORTED_CONTROLLERS) ||\
+	defined(CONFIG_SPX_FEATURE_POLL_FOR_ASYNC_RESET ) 
 typedef struct{
   struct work_struct GetLinkStatusWork;
   char InterfaceName[8];
