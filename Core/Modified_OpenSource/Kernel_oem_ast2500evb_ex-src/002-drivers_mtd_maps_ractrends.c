--- kernel/drivers/mtd/maps/ractrends.c	2017-10-31 22:08:34.642556200 -0700
+++ kernel.new/drivers/mtd/maps/ractrends.c	2017-10-31 23:26:56.225852100 -0700
@@ -39,6 +39,9 @@
 
 #define FULL_FLASH_PARTITION_NAME	"fullpart"
 unsigned char total_active_spi_banks;
+#ifdef CONFIG_SPX_FEATURE_HW_FAILSAFE_BOOT
+unsigned char broken_spi_banks = 0;// specify bank#0 of SPI Flash is broken or not
+#endif
 
 unsigned long
 GetFMHPartitions(unsigned long FlashStart, uint64_t FlashSize, uint32_t SectorSize,struct mtd_info *ractrendsmtd,unsigned long *PartCount)
@@ -171,7 +174,12 @@
 		startingsector=startingsector_p;
 		endingsector=endingsector_p;
 		startaddress=startaddress_p;
-		goto START_SCAN;// NOW START SCANNING FOR INACTIVE IMAGE
+#ifdef CONFIG_SPX_FEATURE_HW_FAILSAFE_BOOT
+		if (broken_spi_banks == 0)
+#endif
+		{
+			goto START_SCAN;// NOW START SCANNING FOR INACTIVE IMAGE
+		}
 	}
 #endif
 
@@ -179,6 +187,9 @@
 }
 
 EXPORT_SYMBOL(ractrends_mtd) ;
+#ifdef CONFIG_SPX_FEATURE_HW_FAILSAFE_BOOT
+EXPORT_SYMBOL(broken_spi_banks) ;
+#endif
 
 struct map_info ractrends_flash_map[MAX_BANKS];
 static struct mtd_info *concat_mtd = NULL;
@@ -234,6 +245,10 @@
 				printk("ERROR: init_ractrends_flash: flash probe failed\n");
 				return -ENXIO;
 			}
+#ifdef CONFIG_SPX_FEATURE_HW_FAILSAFE_BOOT
+			if (bank == 1)
+				broken_spi_banks = 1;
+#endif 
 			break;
 		}
 		ractrends_mtd[bank]->owner = THIS_MODULE;
