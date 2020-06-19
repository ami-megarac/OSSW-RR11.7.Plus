--- linux.org/net/ipv4/devinet.c	Wed Jan 17 09:40:14 2018
+++ linux/net/ipv4/devinet.c	Wed Jan 17 11:10:17 2018
@@ -328,6 +328,9 @@
 	int do_promote = IN_DEV_PROMOTE_SECONDARIES(in_dev);
 
 	ASSERT_RTNL();
+	
+	if (in_dev->dead)
+		goto no_promotions;
 
 	/* 1. Deleting primary ifaddr forces deletion all secondaries
 	 * unless alias promotion is set
@@ -375,6 +378,7 @@
 			fib_del_ifaddr(ifa, ifa1);
 	}
 
+no_promotions:
 	/* 2. Unlink it */
 
 	*ifap = ifa1->ifa_next;
