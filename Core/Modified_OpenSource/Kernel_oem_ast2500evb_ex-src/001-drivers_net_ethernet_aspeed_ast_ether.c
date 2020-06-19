--- linux.org/drivers/net/ethernet/aspeed/ast_ether.c	Thu Sep 21 10:51:14 2017
+++ linux/drivers/net/ethernet/aspeed/ast_ether.c	Thu Sep 21 11:11:21 2017
@@ -74,9 +74,9 @@
 
 	count = 0;
 	while (inl(ioaddr + AST_ETHER_PHYCR) & (PHYCR_READ | PHYCR_WRITE)) {
-		mdelay(10);
+		udelay(1);
 		count ++;
-		if (count > 100)
+		if (count > 1000000)
 			break;
 	}
 }
