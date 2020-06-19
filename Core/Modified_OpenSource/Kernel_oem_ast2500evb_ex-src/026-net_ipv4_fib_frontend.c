--- linux.org/net/ipv4/fib_frontend.c	Thu Aug 14 09:38:34 2014
+++ linux/net/ipv4/fib_frontend.c	Wed Jan 17 11:12:33 2018
@@ -811,6 +811,9 @@
 			  any, ifa->ifa_prefixlen, prim);
 		subnet = 1;
 	}
+	
+	if (in_dev->dead)
+		goto no_promotions;
 
 	/* Deletion is more complicated than add.
 	 * We should take care of not to delete too much :-)
@@ -887,6 +890,7 @@
 		}
 	}
 
+no_promotions:
 	if (!(ok & BRD_OK))
 		fib_magic(RTM_DELROUTE, RTN_BROADCAST, ifa->ifa_broadcast, 32, prim);
 	if (subnet && ifa->ifa_prefixlen < 31) {
