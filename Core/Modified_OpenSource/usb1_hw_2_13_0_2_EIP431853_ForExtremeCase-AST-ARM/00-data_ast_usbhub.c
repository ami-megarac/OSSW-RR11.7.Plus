--- .pristine/usb1_hw-2.13.0-ARM-AST-src/data/ast_usbhub.c	Thu Dec 27 16:11:44 2018
+++ source/usb1_hw-2.13.0-ARM-AST-src/data/ast_usbhub.c	Fri Dec 28 10:01:24 2018
@@ -886,6 +886,8 @@
 			  telling devices to read that DMA
 			*/
 			for( index = len - 1 ; index > (len - 4) ; index-- ) {
+				if(index < 0)
+					break;
 				if(dma_ptr[index] != buf[index]) {
 					TCRIT("EP0 DMA copy with ep0 error, memory copy again");
 					memcpy(ast_usbhub_dev_ep0_buf_addr[dev_num], buf, len);
@@ -913,6 +915,8 @@
 			  telling devices to read that DMA
 			*/
 			for( index = len - 1 ; index > (len - 4) ; index-- ) {
+				if(index < 0)
+					break;
 				if(dma_ptr[index] != buf[index]) {
 					TCRIT("EP DMA copy with ep error, memory copy again");
 					memcpy(ast_usbhub_ep_buf_addr[ep_num], buf, len);
