--- .pristine/mod_websocket-1.21.0-src/data/mod_websocket.h	Tue Oct 15 15:54:27 2019
+++ source/mod_websocket-1.21.0-src/data/mod_websocket.h	Tue Oct 15 18:52:25 2019
@@ -46,6 +46,7 @@
 #include <pthread.h>
 #include <sys/types.h>
 #include <sys/stat.h>
+#include <sys/prctl.h>
 #include <fcntl.h>
 
 #ifdef HAVE_STDINT_H
@@ -230,14 +231,17 @@
     int mod_websocket_frame_send(handler_ctx *, mod_websocket_frame_type_t, char *, size_t);
     int mod_websocket_frame_recv(handler_ctx *);
 	
-	int mod_SendDataToDriver(uint8 Instance, char *to_driver_data);
-	void mod_release_usb_dev();
-	int mod_request_usb_dev();
-	u8* mod_AddAuthInfo (int cmd, u8 *data, uint8 Instance);
-	int mod_ProcessDisconnect(uint8 Instance);
-	int mod_ProcessActivate(uint8 Instance);
-	
-	IUSB_IOCTL_DATA mod_IoctlData[1];
+	int sendDataToDriver(uint8 Instance, char *to_driver_data);
+	void releaseUsbDev(int instance);
+	int requestUsbDev(int instnace);
+
+	u8* addAuthInfo (int cmd, u8 *data, uint8 Instance);
+	int processDisconnect(uint8 Instance);
+	int processActivate(uint8 Instance);
+	int sendCmdToCdserver(int server,int cmd,int instance);
+
+	void sendDataToServer(int server, char *tout_buf, int len);
+	IUSB_IOCTL_DATA ioctlData[MAX_CD_INSTANCES];
 	int mod_usbfd;
 	
 #ifdef  __cplusplus
