--- .pristine/usbe-2.25.0-src/data/linux.c	Thu Aug 16 14:43:33 2018
+++ source/usbe-2.25.0-src/data/linux.c	Thu Aug 16 14:48:29 2018
@@ -311,7 +311,7 @@
 	if(mutex_trylock(&Dev->txon))
 	{
 		CurrentActiveBufNum = r;
-		if(!in_softirq())		
+		if(!in_softirq()||(!timer_pending(&poll_timer)))	
 		StartTxTimeoutTimer ();
 		Dev->NetTxHandler(&Dev->TxBuff[r]);
 	}	
