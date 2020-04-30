--- .pristine/usb1_hw-2.13.0-ARM-AST-src/data/ast_usbhub.c	Fri Jan  4 11:47:39 2019
+++ source/usb1_hw-2.13.0-ARM-AST-src/data/ast_usbhub.c	Fri Mar  8 11:34:10 2019
@@ -1227,9 +1227,12 @@
 {
 	unsigned int desc_len;
 	unsigned int total_len;
+	uint8_t *ptr = NULL;
 
 	if (!config_descs)
 		return -EINVAL;
+
+	ptr = buf;
 
 	/* fill buffer from config_descs[] until null descriptor pointer */
 	total_len = 0;
@@ -1243,6 +1246,13 @@
 		buf += desc_len;
 		total_len += desc_len;
 		config_descs ++; /* nex descriptor */
+
+		/* dummy read */
+		if(total_len > 3)
+		TDBG_FLAGGED(ast_usbhub_debug_flags, AST_USBHUB_DEBUG_SETUP, "%x %x %x\n",
+				ptr[total_len - 1],
+				ptr[total_len - 2],
+				ptr[total_len - 3]);
 	}
 
 	return total_len;
@@ -1410,6 +1420,7 @@
 	uint16_t w_value = le16_to_cpu(ctrl_req->wValue);
 	uint16_t w_length = le16_to_cpu(ctrl_req->wLength);
 	uint8_t desc_type, desc_index;
+	uint8_t *ptr = NULL;
 	uint8_t port;
 	int value = 0;
     //int value = -EOPNOTSUPP; //Re-Assigned again in all places.
@@ -1431,6 +1442,13 @@
 
 		value = (w_length < USB_DT_HUB_SIZE) ? w_length : USB_DT_HUB_SIZE;
 		memcpy(ast_usbhub_hub_ep0_buf, &ast_usbhub_hub_class_desc, value);
+		/* dummy read */
+		ptr = (uint8_t *)ast_usbhub_hub_ep0_buf;
+		TDBG_FLAGGED(ast_usbhub_debug_flags, AST_USBHUB_DEBUG_SETUP, "%x %x %x\n",
+				ptr[value - 1],
+				ptr[value - 2],
+				ptr[value - 3]);
+
 		break;
 	case USB_REQ_GET_STATUS:
 		ast_usbhub_hub_ctrl_data_stage = EP0_CTRL_DATA_STAGE_IN;
@@ -1442,6 +1460,13 @@
 				break;
 
 			memcpy(ast_usbhub_hub_ep0_buf, (uint8_t *) &ast_usbhub_hub_class_status, USB_HUB_STATUS_SIZE);
+			/* dummy read */
+			ptr = (uint8_t *)ast_usbhub_hub_ep0_buf;
+			TDBG_FLAGGED(ast_usbhub_debug_flags, AST_USBHUB_DEBUG_SETUP, "%x %x %x\n",
+					ptr[AST_USBHUB_DEBUG_SETUP - 1],
+					ptr[AST_USBHUB_DEBUG_SETUP - 2],
+					ptr[AST_USBHUB_DEBUG_SETUP - 3]);
+			
 			value = USB_HUB_STATUS_SIZE;
 			break;
 		case (USB_DIR_IN | USB_RT_PORT):
@@ -1456,6 +1481,13 @@
 				break;
 
 			memcpy(ast_usbhub_hub_ep0_buf, &(ast_usbhub_port_status[port]), USB_PORT_STATUS_SIZE);
+			/* dummy read */
+			ptr = (uint8_t *)ast_usbhub_hub_ep0_buf;
+			TDBG_FLAGGED(ast_usbhub_debug_flags, AST_USBHUB_DEBUG_SETUP, "%x %x %x\n",
+					ptr[USB_PORT_STATUS_SIZE - 1],
+					ptr[USB_PORT_STATUS_SIZE - 2],
+					ptr[USB_PORT_STATUS_SIZE - 3]);
+			
 			value = USB_PORT_STATUS_SIZE;
 			break;
 		}
@@ -1639,6 +1671,7 @@
 	uint8_t desc_type, desc_index, recipient;
 	uint32_t reg;
 	int value = -EOPNOTSUPP;
+	uint8_t *ptr = NULL;
 
 	switch (ctrl_req->bRequest) {
 	case USB_REQ_SET_ADDRESS:
@@ -1661,6 +1694,13 @@
 
 			value = USB_DT_DEVICE_SIZE;
 			memcpy(ast_usbhub_hub_ep0_buf, &ast_usbhub_hub_dev_desc, value);
+			/* dummy read */
+			ptr = (uint8_t *)ast_usbhub_hub_ep0_buf;
+			TDBG_FLAGGED(ast_usbhub_debug_flags, AST_USBHUB_DEBUG_SETUP, "%x %x %x\n",
+					ptr[sizeof(ast_usbhub_hub_dev_desc) - 1],
+					ptr[sizeof(ast_usbhub_hub_dev_desc) - 2],
+					ptr[sizeof(ast_usbhub_hub_dev_desc) - 3]);
+			
 			break;
 		case USB_DT_CONFIG:
 			TDBG_FLAGGED(ast_usbhub_debug_flags, AST_USBHUB_DEBUG_SETUP, "CFG DESC\n");
@@ -1679,6 +1719,13 @@
 			value = sizeof(ast_usbhub_hub_dev_qualifier_desc);
 			ast_usbhub_hub_dev_qualifier_desc.bDeviceProtocol = (ast_usbhub_hub_speed == USB_SPEED_FULL) ? 0x01 : 0x00;
 			memcpy(ast_usbhub_hub_ep0_buf, &ast_usbhub_hub_dev_qualifier_desc, value);
+
+			/* dummy read */
+			ptr = (uint8_t *)ast_usbhub_hub_ep0_buf;
+			TDBG_FLAGGED(ast_usbhub_debug_flags, AST_USBHUB_DEBUG_SETUP, "%x %x %x\n",
+					ptr[sizeof(ast_usbhub_hub_dev_qualifier_desc) - 1],
+					ptr[sizeof(ast_usbhub_hub_dev_qualifier_desc) - 2],
+					ptr[sizeof(ast_usbhub_hub_dev_qualifier_desc) - 3]);
 			break;
 		case USB_DT_OTHER_SPEED_CONFIG:
 
