--- linux.org/drivers/mmc/core/mmc.c	2018-04-26 16:56:12.232120174 +0800
+++ linux/drivers/mmc/core/mmc.c	2018-04-26 16:57:05.226017125 +0800
@@ -293,13 +293,12 @@
 		}
 	}
 
+	/*
+	 * The EXT_CSD format is meant to be forward compatible. As long
+	 * as CSD_STRUCTURE does not change, all values for EXT_CSD_REV
+	 * are authorized, see JEDEC JESD84-B50 section B.8.
+	 */
 	card->ext_csd.rev = ext_csd[EXT_CSD_REV];
-	if (card->ext_csd.rev > 7) {
-		pr_err("%s: unrecognised EXT_CSD revision %d\n",
-			mmc_hostname(card->host), card->ext_csd.rev);
-		err = -EINVAL;
-		goto out;
-	}
 
 	card->ext_csd.raw_sectors[0] = ext_csd[EXT_CSD_SEC_CNT + 0];
 	card->ext_csd.raw_sectors[1] = ext_csd[EXT_CSD_SEC_CNT + 1];
