--- .pristine/ncsi-2.32.0-src/data/include/ncsi.h	Tue Dec  4 12:08:16 2018
+++ source/ncsi-2.32.0-src/data/include/ncsi.h	Wed Dec  5 17:23:04 2018
@@ -790,7 +790,8 @@
 void ProcessNCSI(NCSI_IF_INFO *info, struct sk_buff *skb);
 int SendNCSICommand(NCSI_IF_INFO *info);
 int CheckIfAEN(NCSI_IF_INFO *info, struct sk_buff *skb);
-#ifdef CONFIG_SPX_FEATURE_NCSI_GET_LINK_STATUS_FOR_NON_AEN_SUPPORTED_CONTROLLERS
+#if defined(CONFIG_SPX_FEATURE_NCSI_GET_LINK_STATUS_FOR_NON_AEN_SUPPORTED_CONTROLLERS) ||\
+	defined(CONFIG_SPX_FEATURE_POLL_FOR_ASYNC_RESET )
 int CheckAENSupport(NCSI_IF_INFO *info, UINT8 PackageID, UINT8 ChannelID);
 #endif
 void DisplayLinkStatus(NCSI_IF_INFO* info, UINT32 Link, UINT8 verbose);
