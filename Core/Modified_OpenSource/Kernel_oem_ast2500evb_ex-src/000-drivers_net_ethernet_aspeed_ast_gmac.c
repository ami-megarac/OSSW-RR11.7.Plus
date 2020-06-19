--- linux.org/drivers/net/ethernet/aspeed/ast_gmac.c	Thu Sep 21 10:51:13 2017
+++ linux/drivers/net/ethernet/aspeed/ast_gmac.c	Thu Sep 21 11:11:45 2017
@@ -222,7 +222,7 @@
 	unsigned int tmp;
 
 	do {
-		mdelay(10);
+		udelay(1);
 		tmp =inl(ioaddr + PHYCR_REG);
 	} while ((tmp&(PHY_READ_bit|PHY_WRITE_bit)) > 0);
 }
