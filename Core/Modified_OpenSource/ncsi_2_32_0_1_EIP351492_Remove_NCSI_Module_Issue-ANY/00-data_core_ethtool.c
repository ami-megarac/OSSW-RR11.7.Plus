--- .pristine/ncsi-2.32.0-src/data/core/ethtool.c	Fri Aug  4 11:07:20 2017
+++ source/ncsi-2.32.0-src/data/core/ethtool.c	Fri Aug  4 13:19:37 2017
@@ -47,6 +47,7 @@
 static int  ncsi_nway_reset(struct net_device *netdev);
 static int  ncsi_get_settings(struct net_device *netdev, struct ethtool_cmd *cmd);
 static int  ncsi_set_settings(struct net_device *netdev, struct ethtool_cmd *cmd);
+extern void ast_ether_register_ethtool(struct net_device* dev);
 
 extern int UserSpeed;
 extern int UserDuplex;
@@ -127,7 +128,10 @@
 	if (ethindex == -1)
 		return 1;
 
-	return unregister_ethtool_override(ethindex);
+	unregister_ethtool_override(ethindex); 
+	ast_ether_register_ethtool(info->dev);
+
+	return 0;
 
 }
 
