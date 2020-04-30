--- .pristine/jtag-2.8.0-src/data/jtagmain.c	Thu Aug 31 16:54:12 2017
+++ source/jtag-2.8.0-src/data/jtagmain.c	Fri Sep  1 16:07:01 2017
@@ -270,9 +270,9 @@
 
 		case IOCTL_JTAG_UPDATE_JBC:
 			if(Kernal_IO_Data.Data < AST_FW_BUFFER_SIZE){
-				ret = copy_from_user ((u32 *)JTAG_write_buffer, (u8 *)Kernal_IO_Data.Input_Buffer_Base, Kernal_IO_Data.Data);
+				ret = copy_from_user ((u32 *)JTAG_other_buffer, (u8 *)Kernal_IO_Data.Input_Buffer_Base, Kernal_IO_Data.Data);
 				if(pjhwd_ops->set_hw_device_ctl != NULL){
-					ret = pjhwd_ops->set_hw_device_ctl(IOCTL_JTAG_UPDATE_DEVICE,(void*)JTAG_write_buffer,Kernal_IO_Data.Data);
+					ret = pjhwd_ops->set_hw_device_ctl(IOCTL_JTAG_UPDATE_DEVICE,(void*)JTAG_other_buffer,Kernal_IO_Data.Data);
 				}
 				else{
 					ret = 1;
