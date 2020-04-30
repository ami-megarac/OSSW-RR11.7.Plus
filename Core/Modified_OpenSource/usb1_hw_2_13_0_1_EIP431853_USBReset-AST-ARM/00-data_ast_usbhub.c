--- .pristine/usb1_hw-2.13.0-ARM-AST-src/data/ast_usbhub.c	Mon Dec 24 15:36:45 2018
+++ source/usb1_hw-2.13.0-ARM-AST-src/data/ast_usbhub.c	Thu Dec 27 15:25:18 2018
@@ -863,6 +863,8 @@
 int ast_usbhub_write_data_to_host(uint8_t dev_num, uint8_t ep_num, uint8_t *buf, uint16_t len)
 {
 	uint32_t reg;
+	char *dma_ptr = NULL;
+	int index = 0;
 
 	TDBG_FLAGGED(ast_usbhub_debug_flags, AST_USBHUB_DEBUG_CALL, "call write %d %d %d\n", dev_num, ep_num, len);
 	if (ep_num > AST_USBHUB_DEV_EP_NUM)
@@ -875,6 +877,20 @@
 		if (len != 0) {
 			/* write data from buffer into DMA memory */
 			memcpy(ast_usbhub_dev_ep0_buf_addr[dev_num], buf, len);
+			dma_ptr = (char *)ast_usbhub_dev_ep0_buf_addr[dev_num];
+
+			/*
+			  workaround for resolved USB reset that stress test send 
+			  the same command from Host
+			  make sure to flush the CPU write buffers before 
+			  telling devices to read that DMA
+			*/
+			for( index = len - 1 ; index > (len - 4) ; index-- ) {
+				if(dma_ptr[index] != buf[index]) {
+					TCRIT("EP0 DMA copy with ep0 error, memory copy again");
+					memcpy(ast_usbhub_dev_ep0_buf_addr[dev_num], buf, len);
+				}
+			}
 		}
 
 		wmb();
@@ -888,6 +904,20 @@
 		if (len != 0) {
 			/* write data from buffer into DMA memory */
 			memcpy(ast_usbhub_ep_buf_addr[ep_num], buf, len);
+			dma_ptr = (char *)ast_usbhub_ep_buf_addr[ep_num];
+			
+			/*
+			  workaround for resolved USB reset that stress test send 
+			  the same command from Host
+			  make sure to flush the CPU write buffers before 
+			  telling devices to read that DMA
+			*/
+			for( index = len - 1 ; index > (len - 4) ; index-- ) {
+				if(dma_ptr[index] != buf[index]) {
+					TCRIT("EP DMA copy with ep error, memory copy again");
+					memcpy(ast_usbhub_ep_buf_addr[ep_num], buf, len);
+				}
+			}
 		}
 
 		wmb();
