--- /dev/null	Thu Jan  1 08:00:00 1970
+++ source/mod_websocket-1.21.0-src/data/cdservice.c	Thu Mar  8 15:39:14 2018
@@ -0,0 +1,126 @@
+#include <mod_websocket.h>
+
+#define gMod_NumCdInstances 1
+
+//static unsigned long gracsession_id[gMod_NumCdInstances]={-1};
+IUSB_REQ_REL_DEVICE_INFO gmod_iUSBDevInfo[gMod_NumCdInstances];
+
+u8*
+mod_AddAuthInfo (int cmd, u8 *data, uint8 Instance)
+{
+    if(Instance>=MAX_CD_INSTANCES)
+    {
+	TCRIT("Current CD instance exceeded the maximum allowed CD instances");
+	return NULL;
+    }
+
+    switch (cmd)
+    {
+	case USB_CDROM_RES:
+	case USB_CDROM_REQ:
+	    SetAuthInfo (gmod_iUSBDevInfo[Instance], data, 1);
+	    ((IUSB_HEADER*)data)->Instance = Instance;
+	    return data;
+	case USB_CDROM_EXIT:
+	case USB_CDROM_ACTIVATE:
+	case USB_DEVICE_DISCONNECT:
+	case USB_DEVICE_RECONNECT:
+	    SetAuthInfo (gmod_iUSBDevInfo[Instance],(u8*)&mod_IoctlData[Instance], 0);
+	    mod_IoctlData[Instance].DevInfo.Instance = Instance;
+	    return (u8*)&mod_IoctlData[Instance];
+	default:
+	    TWARN ("Error: AddAuthInfo() received unsupported IOCTL\n");
+    }
+    return NULL;
+}
+void mod_release_usb_dev()
+{
+	uint8 InstanceIx;
+	for (InstanceIx = 0; InstanceIx < gMod_NumCdInstances; InstanceIx++)
+	{
+		ReleaseiUSBDevice (&gmod_iUSBDevInfo[InstanceIx]);
+	}
+	if(mod_usbfd >= 0)
+		close (mod_usbfd);
+
+}
+
+int mod_request_usb_dev()
+{
+	uint8 InstanceIx;
+
+	for (InstanceIx = 0; InstanceIx < gMod_NumCdInstances; InstanceIx++)
+	{
+		gmod_iUSBDevInfo[InstanceIx].DevInfo.DeviceType = IUSB_CDROM_FLOPPY_COMBO;
+		gmod_iUSBDevInfo[InstanceIx].DevInfo.LockType = LOCK_TYPE_SHARED;
+		if (0 != RequestiUSBDevice (&gmod_iUSBDevInfo[InstanceIx]))
+		{
+			gmod_iUSBDevInfo[InstanceIx].DevInfo.DeviceType = IUSB_DEVICE_CDROM;
+			gmod_iUSBDevInfo[InstanceIx].DevInfo.LockType = LOCK_TYPE_SHARED;
+			if (0 != RequestiUSBDevice (&gmod_iUSBDevInfo[InstanceIx]))
+			{
+				TCRIT ("Error Unable to find iUSB CDROM Device \n");
+				return 1;
+			}
+		}
+	}
+
+	/* Open the USB Device */
+	mod_usbfd = open(USB_DEVICE,O_RDWR);
+	if (mod_usbfd < 0)
+	{
+		TCRIT("Error in opening USB Device\n" );
+		return 1;
+	}
+	return 0;
+}
+
+int
+mod_ProcessActivate(uint8 Instance)
+{
+	u8* ModifiedData;
+
+	ModifiedData = (u8*) mod_AddAuthInfo (USB_CDROM_ACTIVATE, NULL, Instance);
+	if (ioctl (mod_usbfd, USB_CDROM_ACTIVATE, ModifiedData))
+	{
+		TWARN ("ProcessActivate(): USB_CDROM_ACTIVATE ioctl failed for Instance %d\n", Instance);
+	}
+
+	return 0;
+}
+
+int
+mod_ProcessDisconnect(uint8 Instance)
+{
+	u8* ModifiedData;
+	/*
+	 * Issue Terminate Ioctl to driver, which releases the wait ioctl,
+	 * which returns to the child thread  with error and the child
+	 * thread will terminate itself
+	 */
+
+	TDBG(" Issuing Terminate Ioctl to the Driver\n");
+
+	ModifiedData = (u8*) mod_AddAuthInfo (USB_CDROM_EXIT, NULL, Instance);
+	if(ioctl(mod_usbfd,USB_CDROM_EXIT,ModifiedData))
+	{
+		return -1;
+	}
+	return 0;
+}
+
+int
+mod_SendDataToDriver(uint8 Instance, char *to_driver_data)
+{
+    u8* ModifiedData;
+	IUSB_SCSI_PACKET *mod_ResPkt = NULL;
+	int ret = -1;
+	
+	mod_ResPkt = (IUSB_SCSI_PACKET *)to_driver_data;
+    ModifiedData = (u8*) mod_AddAuthInfo (USB_CDROM_RES, (u8*)mod_ResPkt, Instance);
+
+	ret = ioctl(mod_usbfd,USB_CDROM_RES,ModifiedData);
+	mod_ResPkt = NULL;
+    return ret;
+}
+
