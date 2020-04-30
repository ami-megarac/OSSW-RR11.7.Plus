--- .pristine/usbe-2.25.0-src/data/linux.c	Tue Dec  4 18:16:06 2018
+++ source/usbe-2.25.0-src/data/linux.c	Wed Dec  5 11:12:59 2018
@@ -311,7 +311,7 @@
 	if(mutex_trylock(&Dev->txon))
 	{
 		CurrentActiveBufNum = r;
-		if(!in_softirq()||(!timer_pending(&poll_timer)))	
+		if(!in_softirq()||(!timer_pending(&poll_timer)))		
 		StartTxTimeoutTimer ();
 		Dev->NetTxHandler(&Dev->TxBuff[r]);
 	}	
@@ -345,7 +345,7 @@
 	ether_setup(dev);
 	dev->tx_queue_len = 0;
 	dev->flags &= ~IFF_MULTICAST;
-	random_ether_addr(dev->dev_addr);
+	//random_ether_addr(dev->dev_addr);	/* moved to Net_OSRegisterDriver */
 }
 
 int Net_OSRegisterDriver(NET_DEVICE *Dev)
@@ -355,11 +355,18 @@
 #if (LINUX_VERSION_CODE >= KERNEL_VERSION (2,6,30))	
 		struct NET_DEVICE *TempPriv;
 #endif
+	const uint8 empty_mac[ETH_ALEN] = { 0 };
 
 	if((ethdev = alloc_netdev(sizeof(NET_DEVICE *), "usb%d", SetupEthernet)) == NULL)
 		return -ENOMEM;
 	
 	ethdev->mtu = Dev->Mtu;
+
+	if (memcmp(Dev->DevMac, empty_mac, ETH_ALEN) != 0) {
+		memcpy(ethdev->dev_addr, Dev->DevMac, ETH_ALEN);
+	} else {
+		random_ether_addr(ethdev->dev_addr);
+	}
 
 	GlobalNetDev = Dev;
 	if((r = register_netdev(ethdev)) != 0)
