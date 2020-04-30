--- .pristine/mod_websocket-1.21.0-src/data/cdservice.c	Tue Oct 15 15:54:27 2019
+++ source/mod_websocket-1.21.0-src/data/cdservice.c	Tue Oct 15 17:30:26 2019
@@ -1,126 +1,141 @@
 #include <mod_websocket.h>
 
-#define gMod_NumCdInstances 1
+IUSB_REQ_REL_DEVICE_INFO iUSBDeviceInfo[MAX_CD_INSTANCES];
 
-//static unsigned long gracsession_id[gMod_NumCdInstances]={-1};
-IUSB_REQ_REL_DEVICE_INFO gmod_iUSBDevInfo[gMod_NumCdInstances];
+/*
+ * addAuthInfo: adds auth info for the given instance
+ * cmd: usb cdrom command
+ * data: iusb header/data
+ * instance : cd usb instance
+ */
+u8*
+addAuthInfo(int cmd, u8 *data, uint8 Instance) {
+	if (Instance >= MAX_CD_INSTANCES) {
+		TCRIT("Current CD instance exceeded the maximum allowed CD instances");
+		return NULL;
+	}
 
-u8*
-mod_AddAuthInfo (int cmd, u8 *data, uint8 Instance)
-{
-    if(Instance>=MAX_CD_INSTANCES)
-    {
-	TCRIT("Current CD instance exceeded the maximum allowed CD instances");
-	return NULL;
-    }
-
-    switch (cmd)
-    {
+	switch (cmd) {
 	case USB_CDROM_RES:
 	case USB_CDROM_REQ:
-	    SetAuthInfo (gmod_iUSBDevInfo[Instance], data, 1);
-	    ((IUSB_HEADER*)data)->Instance = Instance;
-	    return data;
+		SetAuthInfo(iUSBDeviceInfo[Instance], data, 1);
+		((IUSB_HEADER*) data)->Instance = Instance;
+		return data;
 	case USB_CDROM_EXIT:
 	case USB_CDROM_ACTIVATE:
 	case USB_DEVICE_DISCONNECT:
 	case USB_DEVICE_RECONNECT:
-	    SetAuthInfo (gmod_iUSBDevInfo[Instance],(u8*)&mod_IoctlData[Instance], 0);
-	    mod_IoctlData[Instance].DevInfo.Instance = Instance;
-	    return (u8*)&mod_IoctlData[Instance];
+		SetAuthInfo(iUSBDeviceInfo[Instance], (u8*) &ioctlData[Instance], 0);
+		ioctlData[Instance].DevInfo.Instance = Instance;
+		return (u8*) &ioctlData[Instance];
 	default:
-	    TWARN ("Error: AddAuthInfo() received unsupported IOCTL\n");
-    }
-    return NULL;
-}
-void mod_release_usb_dev()
-{
-	uint8 InstanceIx;
-	for (InstanceIx = 0; InstanceIx < gMod_NumCdInstances; InstanceIx++)
-	{
-		ReleaseiUSBDevice (&gmod_iUSBDevInfo[InstanceIx]);
+		TWARN("Error: AddAuthInfo() received unsupported IOCTL\n");
 	}
-	if(mod_usbfd >= 0)
-		close (mod_usbfd);
-
+	return NULL;
 }
 
-int mod_request_usb_dev()
-{
-	uint8 InstanceIx;
-
-	for (InstanceIx = 0; InstanceIx < gMod_NumCdInstances; InstanceIx++)
-	{
-		gmod_iUSBDevInfo[InstanceIx].DevInfo.DeviceType = IUSB_CDROM_FLOPPY_COMBO;
-		gmod_iUSBDevInfo[InstanceIx].DevInfo.LockType = LOCK_TYPE_SHARED;
-		if (0 != RequestiUSBDevice (&gmod_iUSBDevInfo[InstanceIx]))
-		{
-			gmod_iUSBDevInfo[InstanceIx].DevInfo.DeviceType = IUSB_DEVICE_CDROM;
-			gmod_iUSBDevInfo[InstanceIx].DevInfo.LockType = LOCK_TYPE_SHARED;
-			if (0 != RequestiUSBDevice (&gmod_iUSBDevInfo[InstanceIx]))
-			{
-				TCRIT ("Error Unable to find iUSB CDROM Device \n");
-				return 1;
-			}
-		}
+/*
+ * releaseUsbDev: releases cd usb device for the given instance
+ * 
+ */
+void releaseUsbDev(int instance) {
+	uint8 InstanceIx = instance;
+	if (&iUSBDeviceInfo[InstanceIx]) {
+		ReleaseiUSBDevice(&iUSBDeviceInfo[InstanceIx]);
 	}
-
-	/* Open the USB Device */
-	mod_usbfd = open(USB_DEVICE,O_RDWR);
-	if (mod_usbfd < 0)
-	{
-		TCRIT("Error in opening USB Device\n" );
-		return 1;
-	}
-	return 0;
 }
 
-int
-mod_ProcessActivate(uint8 Instance)
-{
-	u8* ModifiedData;
+/*
+ * requestUsbDev: request cd usb device for the given instance
+ * 
+ */
+int requestUsbDev(int instance) {
+	uint8 InstanceIx = instance;
 
-	ModifiedData = (u8*) mod_AddAuthInfo (USB_CDROM_ACTIVATE, NULL, Instance);
-	if (ioctl (mod_usbfd, USB_CDROM_ACTIVATE, ModifiedData))
-	{
-		TWARN ("ProcessActivate(): USB_CDROM_ACTIVATE ioctl failed for Instance %d\n", Instance);
+	iUSBDeviceInfo[InstanceIx].DevInfo.DeviceType = IUSB_CDROM_FLOPPY_COMBO;
+	iUSBDeviceInfo[InstanceIx].DevInfo.LockType = LOCK_TYPE_SHARED;
+	if (0 != RequestiUSBDevice(&iUSBDeviceInfo[InstanceIx])) {
+		iUSBDeviceInfo[InstanceIx].DevInfo.DeviceType = IUSB_DEVICE_CDROM;
+		iUSBDeviceInfo[InstanceIx].DevInfo.LockType = LOCK_TYPE_SHARED;
+		if (0 != RequestiUSBDevice(&iUSBDeviceInfo[InstanceIx])) {
+			TCRIT("Error Unable to find iUSB CDROM Device \n");
+			return 1;
+		}
 	}
 
 	return 0;
 }
 
-int
-mod_ProcessDisconnect(uint8 Instance)
-{
+/*
+ * processActivate: processes activate for the given cd usb instance
+ * 
+ */
+int processActivate(uint8 Instance) {
+	u8* ModifiedData;
+	ModifiedData = (u8*) addAuthInfo(USB_CDROM_ACTIVATE, NULL, Instance);
+	if (ioctl(mod_usbfd, USB_CDROM_ACTIVATE, ModifiedData)) {
+		TWARN("ProcessActivate(): USB_CDROM_ACTIVATE ioctl failed for Instance %d\n", Instance);
+	}
+	return 0;
+}
+
+/*
+ * processDisconnect: processes disconnect for the given cd usb instance
+ * 
+ */
+int processDisconnect(uint8 Instance) {
 	u8* ModifiedData;
 	/*
 	 * Issue Terminate Ioctl to driver, which releases the wait ioctl,
 	 * which returns to the child thread  with error and the child
 	 * thread will terminate itself
 	 */
-
 	TDBG(" Issuing Terminate Ioctl to the Driver\n");
 
-	ModifiedData = (u8*) mod_AddAuthInfo (USB_CDROM_EXIT, NULL, Instance);
-	if(ioctl(mod_usbfd,USB_CDROM_EXIT,ModifiedData))
-	{
+	ModifiedData = (u8*) addAuthInfo(USB_CDROM_EXIT, NULL, Instance);
+	if (ioctl(mod_usbfd, USB_CDROM_EXIT, ModifiedData)) {
 		return -1;
 	}
 	return 0;
 }
 
-int
-mod_SendDataToDriver(uint8 Instance, char *to_driver_data)
-{
-    u8* ModifiedData;
-	IUSB_SCSI_PACKET *mod_ResPkt = NULL;
+/*
+ * sendDataToDriver: sends given *to_driver_date to usb driver
+ * Instance : cd usb instance
+ * to_driver_data : scsi packet to be sent
+ */
+int sendDataToDriver(uint8 Instance, char *to_driver_data) {
+	u8* ModifiedData;
+	IUSB_SCSI_PACKET *resPacket = NULL;
 	int ret = -1;
-	
-	mod_ResPkt = (IUSB_SCSI_PACKET *)to_driver_data;
-    ModifiedData = (u8*) mod_AddAuthInfo (USB_CDROM_RES, (u8*)mod_ResPkt, Instance);
 
-	ret = ioctl(mod_usbfd,USB_CDROM_RES,ModifiedData);
-	mod_ResPkt = NULL;
-    return ret;
+	resPacket = (IUSB_SCSI_PACKET *) to_driver_data;
+	ModifiedData = (u8*) addAuthInfo(USB_CDROM_RES, (u8*) resPacket, Instance);
+	ret = ioctl(mod_usbfd, USB_CDROM_RES, ModifiedData);
+	resPacket = NULL;
+	return ret;
 }
 
+/*
+ * sendCmdToCdserver : sends given SCSI cmd to cdserver
+ * server : cdserver socket
+ * cmd : scsi opcode
+ * instance: cd instance number
+ * 
+ */
+int sendCmdToCdserver(int server, int cmd, int instance) {
+
+	int len;
+	IUSB_SCSI_PACKET RemoteHBPkt;
+	memset(&RemoteHBPkt, 0, sizeof(IUSB_SCSI_PACKET));
+
+	RemoteHBPkt.Header.Instance = instance;
+	strncpy((char *) (RemoteHBPkt.Header.Signature), IUSB_SIG, strlen(IUSB_SIG));
+	RemoteHBPkt.CommandPkt.OpCode = cmd; // OpCode 
+	RemoteHBPkt.Header.DataPktLen = mac2long(sizeof(IUSB_SCSI_PACKET) - sizeof(IUSB_HEADER));
+	len = mac2long(RemoteHBPkt.Header.DataPktLen) + sizeof(IUSB_HEADER);
+	sendDataToServer(server, (char *) &RemoteHBPkt, len);
+
+	return 0;
+}
+
