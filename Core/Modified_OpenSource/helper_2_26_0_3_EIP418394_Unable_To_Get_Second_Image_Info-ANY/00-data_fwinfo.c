--- .pristine/helper-2.26.0-src/data/fwinfo.c	Tue Aug  7 11:00:15 2018
+++ source/helper-2.26.0-src/data/fwinfo.c	Wed Aug  8 09:18:49 2018
@@ -654,13 +654,23 @@
 		/* Scan and Get FwInfo */
 		unsigned long flashstartoffset;
 
-                flashstartoffset = CONFIG_SPX_FEATURE_GLOBAL_USED_FLASH_START - CONFIG_SPX_FEATURE_GLOBAL_FLASH_START + CONFIG_SPX_FEATURE_GLOBAL_USED_FLASH_SIZE;		
+		#ifdef CONFIG_SPX_FEATURE_DEDICATED_SPI_FLASH_BANK
+			 flashstartoffset = (CONFIG_SPX_FEATURE_GLOBAL_USED_FLASH_START - CONFIG_SPX_FEATURE_GLOBAL_FLASH_START + CONFIG_SPX_FEATURE_GLOBAL_FLASH_SIZE)/CONFIG_SPX_FEATURE_GLOBAL_FLASH_BANKS;
+			 fwInfo_size = ScanFirmwareModule(flashstartoffset,
+				CONFIG_SPX_FEATURE_GLOBAL_FLASH_SIZE/CONFIG_SPX_FEATURE_GLOBAL_FLASH_BANKS,
+				CONFIG_SPX_FEATURE_GLOBAL_ERASE_BLOCK_SIZE,
+				(unsigned char*)fmhinfo,
+				&flinfo);
+		#else
+             flashstartoffset = CONFIG_SPX_FEATURE_GLOBAL_USED_FLASH_START - CONFIG_SPX_FEATURE_GLOBAL_FLASH_START + CONFIG_SPX_FEATURE_GLOBAL_USED_FLASH_SIZE;		
 		
 		fwInfo_size = ScanFirmwareModule(flashstartoffset,
 				CONFIG_SPX_FEATURE_GLOBAL_USED_FLASH_SIZE,
 				CONFIG_SPX_FEATURE_GLOBAL_ERASE_BLOCK_SIZE,
 				(unsigned char*)fmhinfo,
 				&flinfo);
+
+		#endif
 	
 		if ((signed long)fwInfo_size <= 0 )
 		{
