--- .pristine/mod_websocket-1.21.0-src/data/mod_websocket.h	Thu Feb 22 17:48:58 2018
+++ source/mod_websocket-1.21.0-src/data/mod_websocket.h	Thu Mar  8 15:40:12 2018
@@ -32,6 +32,21 @@
 #include "config.h"
 
 #include <time.h>
+#include "dbgout.h"
+#include "sessioncfg.h"
+#include "vmedia_instance.h"
+#include "racsessioninfo.h"
+#include "vmedia_ifc.h"
+#include "apphead.h"
+#include  "userprivilege.h"
+#include "redirimage.h"
+#include "vmedia_cfg.h"
+#include <sys/prctl.h>
+#include <errno.h>
+#include <pthread.h>
+#include <sys/types.h>
+#include <sys/stat.h>
+#include <fcntl.h>
 
 #ifdef HAVE_STDINT_H
 # include <stdint.h>
@@ -214,7 +229,17 @@
 
     int mod_websocket_frame_send(handler_ctx *, mod_websocket_frame_type_t, char *, size_t);
     int mod_websocket_frame_recv(handler_ctx *);
-
+	
+	int mod_SendDataToDriver(uint8 Instance, char *to_driver_data);
+	void mod_release_usb_dev();
+	int mod_request_usb_dev();
+	u8* mod_AddAuthInfo (int cmd, u8 *data, uint8 Instance);
+	int mod_ProcessDisconnect(uint8 Instance);
+	int mod_ProcessActivate(uint8 Instance);
+	
+	IUSB_IOCTL_DATA mod_IoctlData[1];
+	int mod_usbfd;
+	
 #ifdef  __cplusplus
 }
 #endif
