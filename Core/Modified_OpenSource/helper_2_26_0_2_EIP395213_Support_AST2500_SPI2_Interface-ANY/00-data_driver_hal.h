--- .pristine/helper-2.26.0-src/data/driver_hal.h	Mon Feb 26 11:57:07 2018
+++ source/helper-2.26.0-src/data/driver_hal.h	Thu Mar  1 14:45:55 2018
@@ -49,6 +49,7 @@
 	EDEV_TYPE_SSP,
 	EDEV_TYPE_MAILBOX,
 	EDEV_TYPE_ESPI,
+    EDEV_TYPE_HOST_SPI2_FLASH,
 } ehal_dev_type;
 
 typedef struct
