--- .pristine/hid-2.9.0-src/data/keybd.c	Thu Nov  8 12:07:35 2018
+++ source/hid-2.9.0-src/data/keybd.c	Thu Nov  8 14:32:16 2018
@@ -17,9 +17,11 @@
 #include "usb_core.h"
 #include "helper.h"
 #include "dbgout.h"
+#include "usb_hw.h"
 
 #define DATA_QUEUE_SIZE		64
 #define MAX_HID_IFACES		4
+#define DATA_QUEUE_SIZE_WARNING_LIMIT           55
 
 typedef struct
 {
@@ -381,7 +383,7 @@
 		spin_unlock_irqrestore (&KeybdQueue[IfNum].Lock, Flags);
 	}
 
-	return ret;
+	return ( KeybdQueue[IfNum].Size > DATA_QUEUE_SIZE_WARNING_LIMIT )? QUEUE_FULL_WARNING : ret;
 	
 }
 
