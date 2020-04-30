--- .pristine/mod_websocket-1.21.0-src/data/mod_websocket.c	Tue Oct 15 15:54:27 2019
+++ source/mod_websocket-1.21.0-src/data/mod_websocket.c	Wed Oct 16 15:57:00 2019
@@ -43,6 +43,8 @@
 #include "sessioncfg.h"
 #include "redirimage.h"
 #include "token.h"
+#include <sys/sysinfo.h>
+#include <sys/prctl.h>
 #include <semaphore.h>
 
 #ifdef	HAVE_PCRE_H
@@ -70,7 +72,8 @@
 static handler_t mod_websocket_handle_subrequest(server *, connection *, void *);
 int get_service_config(buffer *service, SERVICE_CONF_STRUCT *ServiceConfig);
 int get_port(SERVICE_CONF_STRUCT ServiceConfig, buffer *port);
-static int cd_instance = -1;
+int cd_instance = -1;
+int media_instance = 0; // thread instance number
 #define OPCODE 2
 typedef struct {
     int        targetfd;
@@ -87,6 +90,7 @@
     int        cli_stopped;
     int        ser_stopped;
 } webs_ctx;
+webs_ctx *ws_ctx_cd;
 
 /**
  * IVTP packet header
@@ -123,6 +127,8 @@
 
 #define IUSB_HEADER_SIZE 61
 
+#define SELECT_REMOTE_RESPONSE_TIMEOUT_SECS        20
+
 #define ntohl64(p) \
     ((((uint64_t)((p)[7])) <<  0) + (((uint64_t)((p)[6])) <<  8) +\
      (((uint64_t)((p)[5])) << 16) + (((uint64_t)((p)[4])) << 24) +\
@@ -161,13 +167,13 @@
 
 	dl_ncmlhandle = dlopen(NCML_LIB, RTLD_LAZY);
 	if (dl_ncmlhandle == NULL) {
-		printf("Problem in loading library:%s\n", dlerror());
+		TCRIT("Problem in loading library:%s\n", dlerror());
 		return -1;
 	}
 
 	dl_getNotEditableData = dlsym(dl_ncmlhandle, "getNotEditableData");
 	if (dl_getNotEditableData == NULL) {
-		printf("No Symbol Found:%s\n", dlerror());
+		TCRIT("No Symbol Found:%s\n", dlerror());
 		dlclose(dl_ncmlhandle);
 		return -1;
 	}
@@ -268,12 +274,12 @@
 	snprintf(serv, sizeof(serv), "%d", PortNo);
 
 	if ((error = getaddrinfo(NULL, serv, &hints, &res))) {
-		printf("\n Couldn't get bind addresses: %s\n", gai_strerror(error));
+		TCRIT("\n Couldn't get bind addresses: %s\n", gai_strerror(error));
 		return -1;
 	}
 
 	if (res == NULL) {
-		printf("\n No addresses to bind to.\n");
+		TCRIT("\n No addresses to bind to.\n");
 		return -1;
 	}
 
@@ -287,22 +293,22 @@
 		if (r->ai_family != AF_INET6)
 			continue;
 		if ((sock = socket(r->ai_family, r->ai_socktype, IPPROTO_TCP)) < 0) {
-			printf("\n Couldn't make socket");
+			TCRIT("\n Couldn't make socket");
 			continue;
 		}
 		if (strcasecmp(InterfaceName, BOTH_INTERFACES) != 0) {
 			if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, InterfaceName,
 					strlen(InterfaceName)) == -1)
-				printf("\n Setsockopt(SO_BINDTODEVICE) Failed for server socket\n");
+				TCRIT("\n Setsockopt(SO_BINDTODEVICE) Failed for server socket\n");
 		}
 
 		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int))
 				== -1)
-			printf("\n Setsockopt(SO_REUSEADDR) Failed for server socket\n");
+			TCRIT("\n Setsockopt(SO_REUSEADDR) Failed for server socket\n");
 
 		/* Establish socket connection with (remote) server. */
 		if (connect(sock, r->ai_addr, r->ai_addrlen) < 0) {
-			printf("\n Server connect failure %d\n", errno);
+			TCRIT("\n Server connect failure %d\n", errno);
 			close(sock);
 			continue;
 			//return INVALID_SOCKET;
@@ -382,7 +388,7 @@
 	}
 
 	if (srclength > (destsize - *payload_offset)) {
-		printf("\npacket is beyond targsize\n");
+		TCRIT("\npacket is beyond targsize\n");
 		return -1;
 	}
 
@@ -428,7 +434,7 @@
 	}
 
 	if (srclength > (destsize - payload_offset)) {
-		printf("\npacket is beyond targsize\n");
+		TCRIT("\npacket is beyond targsize\n");
 		return -1;
 	}
 
@@ -503,7 +509,7 @@
 		// ignore websocket opcodes other than 0(continuation frames),1(text),2(binary) and 0xa(pong)
 		if ((*opcode > 2)) {
 			if (*opcode != 0xa) {
-				printf("Ignoring non-data frame, opcode 0x%x\n", *opcode);
+				TCRIT("Ignoring non-data frame, opcode 0x%x\n", *opcode);
 			}
 			continue;
 		}
@@ -532,7 +538,7 @@
 
 	if (framecount > 1) {
 		snprintf((char *) cntstr, 3, "%d", framecount);
-//		printf("cntstr :%s\n", (char *) cntstr);
+		TCRIT("cntstr :%s\n", (char *) cntstr);
 	}
 
 	*left = remaining;
@@ -573,8 +579,11 @@
 		close(WebSockCtx[(int) instance].sockfd);
 		if(WebSockCtx[(int) instance].ssl_ctx != NULL)
 			WebSockCtx[(int) instance].ssl_ctx = NULL;
-		if(WebSockCtx[(int) instance].ssl != NULL)
+		if(WebSockCtx[(int) instance].ssl != NULL){
+			SSL_free(WebSockCtx[(int) instance].ssl);
 			WebSockCtx[(int) instance].ssl = NULL;
+		}
+		WebSockCtx[(int) instance].sockfd = -1;
 	}
 	WebSockCtx[(int) instance].media = 0;
 	/* To ensure proper cleanup, memset the websocket instance once the
@@ -588,51 +597,24 @@
     pselect(0, NULL, NULL, NULL, &delay, NULL);
 }
 
-void
-mod_proxySetPriority( int prio )
-{
-  int ret;
-  int policy = 0;
-  struct sched_param params;
-
-  // We'll operate on the currently running thread.
-  pthread_t this_thread = pthread_self();
-
-  ret = pthread_getschedparam(this_thread, &policy, &params);
-  if (ret != 0) {
-    printf( "ERROR getting scheduling params ret=%d\n", ret);
-    return;
-  }
-
-  // We'll set the priority to the maximum.
-//  params.sched_priority = sched_get_priority_max( prio );
-  params.sched_priority += 1;
-
-  // Attempt to set thread real-time priority to the SCHED_FIFO policy
-  ret = pthread_setschedparam(this_thread, prio, &params);
-  if (ret != 0) {
-    printf( "ERROR setting thread prio to %d ret=%d\n", params.sched_priority, ret);
-  }
-
-  ret = pthread_getschedparam(this_thread, &policy, &params);
-  if (ret != 0) {
-    printf( "Couldn't retrieve real-time scheduling params ret=%d\n", ret);;
-  }
-
-}
-
-void sendToCdserver(int server, char *tout_buf, int len)
+/*
+ * sendDataToServer: sends the given data to server socket
+ * server: server socket
+ * tout_buf: data to be sent
+ * len: length of data.
+ *
+ */
+void sendDataToServer(int server, char *tout_buf, int len)
 {
 	int bytes = 0;
 	unsigned int sout_start, sout_end;
 
 	sout_start = 0;
 	sout_end = len;
-
 	while(1) {
 		bytes = send(server, tout_buf, len, 0);
 		if (bytes < 0) {
-			printf("\ntarget connection error: %s\n", strerror(errno));
+			TCRIT("\nunable to send to cdserver: %d %s\n", errno, strerror(errno));
 			break;
 		}
 		sout_start += bytes;
@@ -642,21 +624,24 @@
 		}
 	}
 }
-
-void sendToH5Viewer(void *instance, char *cin_buf, int bytes, int hybi)
+/*
+ * sendDataToH5Viewer: sends the given data to h5viewer client
+ * cin_buf: data to be send
+ * bytes: data length
+ * hybi: is websocket version hybi
+ */
+int sendDataToH5Viewer(char *cin_buf, int bytes, int hybi)
 {
 	int len = 0, cout_start = 0, cout_end = 0, start_point = 0;
-	webs_ctx *ws_ctx = &(WebSockCtx[(int) instance]);
 	int ssl_err = 0;
-
 	cout_start = 0;
 	if (hybi) {
 		cout_end = form_encode_websocket_hybi_hdr(bytes, cin_buf, BUFSIZE, &start_point);
 	}
 	if (cout_end < 0) {
-		printf("encoding error\n");
+		TCRIT("encoding error\n");
 		fflush(stdout);
-		return;
+		return -1;
 	}
 	
 	cout_start = MAX_WEBSOCK_HEADER_LEN - start_point;
@@ -666,24 +651,23 @@
 		len = cout_end - cout_start;
 		if(len > 0)
 		{
-			bytes = ws_send(ws_ctx, cin_buf + cout_start, len);
+			bytes = ws_send(ws_ctx_cd, cin_buf + cout_start, len);
 			if (bytes < 0) {
-				if (ws_ctx->ssl != NULL) {
-					ssl_err = SSL_get_error(ws_ctx->ssl, bytes);
+				if (ws_ctx_cd->ssl != NULL) {
+					ssl_err = SSL_get_error(ws_ctx_cd->ssl, bytes);
 					if ((errno == EAGAIN) && ((ssl_err == SSL_ERROR_WANT_WRITE) || (ssl_err == SSL_ERROR_SYSCALL)))
 						goto write_again;
-					printf("\n read failed: errno == %d\t %d\n", errno, SSL_get_error(ws_ctx->ssl, bytes));
 				}
 				else {
 					if (errno == EAGAIN) {
 						goto write_again;
 					}
-					printf("\n Non SSL read failed: errno == %d \n", errno);
-				}
-				return;
+					TCRIT("\n Non SSL read failed: errno == %d \n", errno);
+				}
+				return -1;
 			}
 			if (len < 3) {
-				printf("len: %d, bytes: %d: %d\n", (int) len, (int) bytes, (int) *(ws_ctx->cout_buf + cout_start));
+				TCRIT("len: %d, bytes: %d: %d\n", (int) len, (int) bytes, (int) *(ws_ctx_cd->cout_buf + cout_start));
 			}
 			cout_start += bytes;
 		}
@@ -697,201 +681,208 @@
 			len = cout_start = cout_end = 0;
 		}
 	}
-	
-	ws_ctx = NULL;
-}
-
-void *WebSCSIHndlr(void *instance) {
+	return 0;
+}
+
+/*
+ * WebSCSIHndlr: SCSI handler for the cd_instance
+ * reads data from cd usb instance and sends the data to H5viewer client
+ *
+ */
+void *WebSCSIHndlr() {
 	pthread_t self;
-	webs_ctx *ws_ctx = &(WebSockCtx[(int) instance]);
 	IUSB_SCSI_PACKET *tmp_ReqPkt = NULL;
 	u8* ioctl_data;
 	int Retval = 0, cmd_len;
-	
-
+	int ret;
+	prctl(PR_SET_NAME,__FUNCTION__,0,0,0);
 	/* Create commuincation Pipe between parent and thread */
 	if(cd_instance != -1) {
-		Retval = mod_request_usb_dev();
+		Retval = requestUsbDev(cd_instance);
 		if(Retval == 1)
 		{
-			printf("WebSCSIHndlr, request usb dev failed\n");
+			TCRIT("WebSCSIHndlr, request usb dev failed\n");
 			return NULL;
 		}
 		
-		mod_ProcessActivate (cd_instance);
+		processActivate (cd_instance);
 	}		
-
 	while (cd_instance != -1) {
-		ioctl_data = (u8*) mod_AddAuthInfo (USB_CDROM_REQ,(u8*)ws_ctx->cin_buf + MAX_WEBSOCK_HEADER_LEN, cd_instance);
+		ioctl_data = (u8*) addAuthInfo (USB_CDROM_REQ,(u8*)ws_ctx_cd->cout_buf + MAX_WEBSOCK_HEADER_LEN, cd_instance);
 
 		Retval = ioctl(mod_usbfd,USB_CDROM_REQ,ioctl_data);
 		if (Retval < 0) {
 			break;
 		}
 						
-		tmp_ReqPkt =(IUSB_SCSI_PACKET *) ( ws_ctx->cin_buf + MAX_WEBSOCK_HEADER_LEN);
+		tmp_ReqPkt =(IUSB_SCSI_PACKET *) ( ws_ctx_cd->cout_buf + MAX_WEBSOCK_HEADER_LEN);
 		cmd_len = mac2long(tmp_ReqPkt->Header.DataPktLen) + sizeof(IUSB_HEADER);			
 		
-		sendToH5Viewer(instance, ws_ctx->cin_buf, cmd_len, ws_ctx->hybi);		
+		ret = sendDataToH5Viewer(ws_ctx_cd->cout_buf, cmd_len, ws_ctx_cd->hybi);
+		if(ret!=0){
+			break;
+		}
 	}
 	//disconnect the thread
-	ws_ctx = NULL;
 	tmp_ReqPkt = NULL;
+	releaseUsbDev(cd_instance);
 	self = pthread_self();
 	pthread_detach(self);
 	pthread_exit(NULL);
 }
 
-void *WebSocketProxy(void *instance) {
+void *WebSocketProxy() {
 	fd_set rdlist,  exlist;
 	struct timeval tv;
-	webs_ctx *ws_ctx = &(WebSockCtx[(int) instance]);
-	int ret, client = ws_ctx->sockfd;
-	int server = 0;
-	unsigned int opcode, left;
-	unsigned int sout_end;
-	uint64_t sin_start, sin_end;
-	int len, bytes, maxfd;
+	int ret;
+	int maxfd = 0;
+	unsigned int opcode = 0, left = 0;
+	unsigned int sout_end = 0;
+	uint64_t sin_start = 0, sin_end = 0;
+	int len = 0, bytes;
 	int current_fin = -1;
 	pthread_t self;
 	int ssl_err = 0;
-	int flag = 1; 
 	
 	IUSB_SCSI_PACKET *TempResPkt = NULL;
 	DEV_REDIR_ACK	*AckCommand = NULL;
 	pthread_t mod_ChildId;
-	
-	time_t last_h5viewer_pkt_time = 0;
+	prctl(PR_SET_NAME, __FUNCTION__, 0, 0, 0);
+
+	time_t h5PktRecvdTime = { 0 };
 	struct sysinfo sys_info;
 	double timedout = 0;
 
-	len = sin_start = sin_end = sout_end = 0;
-	cd_instance = -1;
-
-	// Set thread name to something usefull
-	prctl(PR_SET_NAME,__FUNCTION__,0,0,0);
-
-	// Raise priority of the input processing (mouse/keyboard)
-	mod_proxySetPriority( SCHED_RR );
-	
-	server = ws_ctx->targetfd;
-	ws_ctx->hybi = 1;
-	maxfd = client > server ? client + 1 : server + 1;
-
-	setsockopt(server, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
-
+	maxfd = ws_ctx_cd->sockfd > ws_ctx_cd->targetfd ? ws_ctx_cd->sockfd + 1 : ws_ctx_cd->targetfd + 1;
 	while (1) {
 
+		FD_ZERO(&rdlist);
+		FD_ZERO(&exlist);
 		tv.tv_sec = 2;
 		tv.tv_usec = 0;
 
-		FD_ZERO(&rdlist);
-		FD_ZERO(&exlist);
-
-		FD_SET(client, &exlist);
-		FD_SET(server, &exlist);
-		
-		FD_SET(client, &rdlist);
-		FD_SET(server, &rdlist);
+		FD_SET(ws_ctx_cd->targetfd, &exlist);
+		FD_SET(ws_ctx_cd->targetfd, &rdlist);
+
+		FD_SET(ws_ctx_cd->sockfd, &rdlist);
+		FD_SET(ws_ctx_cd->sockfd, &exlist);
 
 		ret = select(maxfd, &rdlist, NULL, &exlist, &tv);
-		if (ret <= 0) {
-			if (errno != 0) {
-				if(errno != EAGAIN) {
-					printf("\nWebSocketProxy: select failed ret:%d err:%d :%s\n", ret, errno, strerror(errno));
-					break;
-				}
-			}
-		}
-
-		if (FD_ISSET(server, &exlist)) {
-			printf("\ntarget exception occurred\n");
-			break;
-		}
-		if (FD_ISSET(client, &exlist)) {
-			printf("\nclient exception occurred\n");
-			break;
-		}
 
 		if (ret == -1) {
-			printf("select(): %s\n", strerror(errno));
+			TCRIT("select(): %s\n", strerror(errno));
 			break;
 		} else if (ret == 0) {
-			//printf("\nselect timedout\n");
 			continue;
 		}
 
 		// read fd is set for client, so read data from client socket
-		if (FD_ISSET(client, &rdlist)) {
-			read_again:
-			//when full websocket frame is not received in single read
-			//should read the socket directly without going for select()
-			//removing "goto" jump will break huge data transfer
-
-			bytes = (uint64_t) ws_recv(ws_ctx, ws_ctx->tin_buf + sin_end, BUFSIZE - 1);
-			if (bytes <= 0) {
-
-				if (ws_ctx->ssl != NULL) {
-					ssl_err = SSL_get_error(ws_ctx->ssl, bytes);
-					if ((ssl_err == SSL_ERROR_WANT_READ) || (ssl_err == SSL_ERROR_SYSCALL)) {
+		if (ws_ctx_cd->sockfd != -1) {
+			if (FD_ISSET(ws_ctx_cd->sockfd, &rdlist)) {
+				read_again:
+				//when full websocket frame is not received in single read
+				//should read the socket directly without going for select()
+				//removing "goto" jump will break huge data transfer
+				
+				bytes = (uint64_t) ws_recv(ws_ctx_cd, ws_ctx_cd->tin_buf + sin_end, BUFSIZE - 1);
+				TempResPkt = (IUSB_SCSI_PACKET *) ws_ctx_cd->tout_buf;
+				if (bytes <= 0) {
+
+					if (ws_ctx_cd->ssl != NULL) {
+						ssl_err = SSL_get_error(ws_ctx_cd->ssl, bytes);
+						if ((ssl_err == SSL_ERROR_WANT_READ) || (ssl_err == SSL_ERROR_SYSCALL)) {
+							goto read_again;
+						}
+						TCRIT("\n SSL client closed connection  sslerr:%d \n", SSL_get_error(ws_ctx_cd->ssl, bytes));
+					}
+					else{
+						if (errno == EAGAIN) {
+							goto read_again;
+						}
+						TCRIT("\n  Non SSL client closed connection  err:%d \n", errno);
+					}
+					cleanup_instance(media_instance);
+					break;
+				}
+
+				sin_end += bytes;
+
+				if (ws_ctx_cd->hybi) {
+					len = decode_websocket_hybi((unsigned char *) ws_ctx_cd->tin_buf + sin_start, (uint64_t) sin_end - sin_start, (u_char *) ws_ctx_cd->tout_buf +sout_end, BUFSIZE - 1, &opcode, &left, &current_fin);
+				}
+
+				if (opcode == 8) {
+					TCRIT("Client sent orderly close frame\n");
+					ws_ctx_cd->sockfd = -1;
+					cleanup_instance(media_instance);
+					break;
+				}
+
+				if (len < 0) {
+					TCRIT("Decoding error\n");
+					fflush(stdout);
+				}
+				
+				if (left) {
+					sin_start = sin_end - left;
+				} else {
+					sin_start = 0;
+					sin_end = 0;
+				}
+				sout_end += len;
+
+				//Need to read more data inorder to complete the frame
+				// again read the data from socket.
+				if (left > 0) {
+					goto read_again;
+				}
+				left = 0;
+				TempResPkt = (IUSB_SCSI_PACKET *)ws_ctx_cd->tout_buf;
+				if( TempResPkt->DataLen + IUSB_HEADER_SIZE != sout_end ){
+					// If the fin is 0 then it means it is not the end frame. And more frames will be received as continuation frames.
+					// If the fin value is 1 then it means it is the end frame.
+					if (current_fin == 0) {
+						// Since there are multiple packets sent simultaneously in case of continuation frames
+						// so we can directly go for socket read without checking for socket event.
+						// Incase of continuation frames, the data should be read from the socket
+						// till we receive the end of the frames denoting the completion of continuation frames
+						// so that we don't miss out any packets in the socket
 						goto read_again;
 					}
-					printf("\n SSL client closed connection  sslerr:%d \n", SSL_get_error(ws_ctx->ssl, bytes));
-				}
-				else{
-					if (errno == EAGAIN) {
-						goto read_again;
+				}
+				
+				//Send data to server or USB driver which is received from client
+				{
+					// based on the opcode received from the H5Viewer client, send AMI USB SCSI opcode packets to CD server and
+					// common USB SCSI opcode packets to USB driver directly.
+					if (TempResPkt->CommandPkt.OpCode != MEDIA_SESSION_DISCONNECT &&
+							TempResPkt->CommandPkt.OpCode != AUTH_CMD &&
+							TempResPkt->CommandPkt.OpCode != DEVICE_INFO &&
+							TempResPkt->CommandPkt.OpCode != CDROM_KEEPALIVE_SCSI_CMD){
+						sendDataToDriver(cd_instance, ws_ctx_cd->tout_buf + 0);
+						// update h5viewer packet received time
+						if (!sysinfo(&sys_info)) {
+							h5PktRecvdTime = sys_info.uptime;
+						}
+					} else {
+						sendDataToServer(ws_ctx_cd->targetfd, ws_ctx_cd->tout_buf, len);
+						
+						if (TempResPkt->CommandPkt.OpCode == MEDIA_SESSION_DISCONNECT) {
+							processDisconnect(cd_instance);
+							pthread_cancel(mod_ChildId);
+							pthread_join(mod_ChildId, NULL);
+							releaseUsbDev(cd_instance);
+							cleanup_instance(media_instance);
+							break;
+						}
 					}
-					printf("\n  Non SSL client closed connection  err:%d \n", errno);
-				}
-				break;
-			}
-
-			sin_end += bytes;
-
-			if (ws_ctx->hybi) {
-				len = decode_websocket_hybi((unsigned char *) ws_ctx->tin_buf + sin_start, (uint64_t) sin_end - sin_start, (u_char *) ws_ctx->tout_buf + sout_end, BUFSIZE - 1, &opcode, &left, &current_fin);
-			}
-
-			if (opcode == 8) {
-				printf("\nclient sent orderly close frame\n");
-				break;
-			}
-
-			if (len < 0) {
-				printf("\ndecoding error\n");
-				fflush(stdout);
-			}
-
-			if (left) {
-				sin_start = sin_end - left;
-			} else {
-				sin_start = 0;
-				sin_end = 0;
-			}
-
-			sout_end += len;
-
-			//Need to read more data inorder to complete the frame
-			// again read the data from socket.
-			if (left > 0) {
-				goto read_again;
-			}
-			TempResPkt = (IUSB_SCSI_PACKET *)ws_ctx->tout_buf;
-			if( TempResPkt->DataLen + IUSB_HEADER_SIZE != sout_end )
-			{
-				// If the fin is 0 then it means it is not the end frame. And more frames will be received as continuation frames.
-				// If the fin value is 1 then it means it is the end frame.
-				if (current_fin == 0) {
-					// Since there are multiple packets sent simultaneously in case of continuation frames
-					// so we can directly go for socket read without checking for socket event.
-					// Incase of continuation frames, the data should be read from the socket
-					// till we receive the end of the frames denoting the completion of continuation frames
-					// so that we don't miss out any packets in the socket
-					goto read_again;
-				}
-			}
-			
+					sout_end = 0;
+					sin_start = 0;
+					sin_end = 0;
+				}
+			}
+		}
+		/*		
 			//Send data to server which is received from client
 			{
 				if(last_h5viewer_pkt_time == 0) {
@@ -933,7 +924,7 @@
 
 						RemoteHBPkt.Header.Instance = cd_instance;
 						strncpy((char *)(RemoteHBPkt.Header.Signature), "IUSB    ", strlen("IUSB    ") );
-						RemoteHBPkt.CommandPkt.OpCode = WEBSOCKET_KEEP_ALIVE_CMD; /* OpCode */
+						RemoteHBPkt.CommandPkt.OpCode = WEBSOCKET_KEEP_ALIVE_CMD;
 						RemoteHBPkt.Header.DataPktLen = mac2long(sizeof(IUSB_SCSI_PACKET) - sizeof(IUSB_HEADER));
 
 						len = mac2long(RemoteHBPkt.Header.DataPktLen) + sizeof(IUSB_HEADER);
@@ -943,50 +934,72 @@
 				}
 			}
 		}
-
-		//if there are some response from cdserver, it will received data and then send to H5Viewer
+		*/
 		// read fd is set for server, so read data from server socket
-		if (FD_ISSET(server, &rdlist)) {
-			bytes = recv(server, ws_ctx->cin_buf + MAX_WEBSOCK_HEADER_LEN, BUFSIZEWOHDR, 0);
+		if (FD_ISSET(ws_ctx_cd->targetfd, &rdlist)) {
+			bytes = recv(ws_ctx_cd->targetfd, ws_ctx_cd->cin_buf + MAX_WEBSOCK_HEADER_LEN, BUFSIZEWOHDR, 0);
+			AckCommand = (DEV_REDIR_ACK *) (ws_ctx_cd->cin_buf + MAX_WEBSOCK_HEADER_LEN);
 			if (bytes <= 0) {
-				printf("\ntarget closed connection  errno:%d\n", errno);
+				TCRIT("\ntarget closed connection  errno:%d\n", errno);
+				ws_ctx_cd->targetfd = -1;
 				break;
 			}
 			
-			AckCommand = (DEV_REDIR_ACK	*)(ws_ctx->cin_buf + MAX_WEBSOCK_HEADER_LEN);
+			AckCommand = (DEV_REDIR_ACK	*) (ws_ctx_cd->cin_buf + MAX_WEBSOCK_HEADER_LEN);
 			if(AckCommand->iUsbScsiPkt.CommandPkt.OpCode == DEVICE_REDIRECTION_ACK) {
 				if(AckCommand->ConnectionStatus == CONNECTION_ACCEPTED) {
-					
 					cd_instance = AckCommand->iUsbScsiPkt.Header.Instance;
-					if (0 != pthread_create(&mod_ChildId, NULL, WebSCSIHndlr, (void *) instance) ) {
-						printf("\nUnable to Create WebSockThread  no:%d \n", (int)instance);
+					if (0 != pthread_create(&mod_ChildId, NULL, WebSCSIHndlr, (void *) NULL)) {
 						break;
 					}
 				}
-				else//cdserver reject connection
+				else
+					//cdserver reject connection
 					break;
 			}
-
-			sendToH5Viewer(instance, ws_ctx->cin_buf, bytes, ws_ctx->hybi);
+			//Based on lastpacket received time out send Keepalive pkt to cdserver/remote client
+			//KEEP ALIVE MECHANISM:
+			// When cdserver sends keep alive, check if there is data transfer between h5viewer and usb driver. (i.e based on h5PktRecvdTime)
+			// If there is data transfer between h5viewer and usb driver, no need to send and wait for response from h5viewer.
+			// so mod_websocket will send the keepalive response to the cdserver by itself.
+			// If there is no data transfer between h5viewer and usb driver, then mod_websocket will send the keepalive
+			// received from cdserver to h5viewer. and h5viewer will reply to the keep alive.
+			if (AckCommand->iUsbScsiPkt.CommandPkt.OpCode == CDROM_KEEPALIVE_SCSI_CMD) {
+				if (!sysinfo(&sys_info)) {
+					timedout = difftime(sys_info.uptime, h5PktRecvdTime);
+					if (timedout > (SELECT_REMOTE_RESPONSE_TIMEOUT_SECS)) {
+						sendDataToH5Viewer(ws_ctx_cd->cin_buf, bytes, ws_ctx_cd->hybi);
+						h5PktRecvdTime = 0;
+					} else {
+						sendCmdToCdserver(ws_ctx_cd->targetfd, CDROM_KEEPALIVE_SCSI_CMD, cd_instance);
+						h5PktRecvdTime = 0;
+					}
+				}
+			} else {
+				sendDataToH5Viewer(ws_ctx_cd->cin_buf, bytes, ws_ctx_cd->hybi);
+			}
+		}
+		//while Reconnecting network back sending disconnect cmd based on opcode  value
+		if (opcode == 8) {
+			sendCmdToCdserver(ws_ctx_cd->targetfd, MEDIA_SESSION_DISCONNECT, cd_instance);
+			pthread_cancel(mod_ChildId);
+			pthread_join(mod_ChildId, NULL);
+			releaseUsbDev(cd_instance);
+			cleanup_instance(media_instance);
 		}
 
 		// Add sleep to make CPU happy.
 		// Without this sleep, the thread might shoot up the CPU usage if the server runs faster than the kvm client
-		if ((FD_ISSET(client, &rdlist)) || (FD_ISSET(server, &rdlist))) 
-		{
+		if ((FD_ISSET(ws_ctx_cd->sockfd, &rdlist)) || (FD_ISSET(ws_ctx_cd->targetfd, &rdlist))) {
 		    NanoSleep(1);
 		}
 	}
-	TempResPkt = NULL;
-	AckCommand = NULL;
-	pthread_cancel (mod_ChildId);
-	pthread_join(mod_ChildId, NULL);
-
-	mod_release_usb_dev();
-	// cleanup resources
-	cleanup_instance((int) instance);
 
 	// disconnect the thread
+	if (mod_usbfd >= 0) {
+		close(mod_usbfd);
+		mod_usbfd = -1;
+	}
 	self = pthread_self();
 	pthread_detach(self);
 	pthread_exit(NULL);
@@ -1037,19 +1050,19 @@
 		if (ret <= 0) {
 			if (errno != 0) {
 				if(errno != EAGAIN) {
-					printf("\nWebSocketProxy: select failed ret:%d err:%d :%s\n", ret, errno, strerror(errno));
+					TCRIT("\nWebSocketProxy: select failed ret:%d err:%d :%s\n", ret, errno, strerror(errno));
 					break;
 				}
 			}
 		}
 
 		if (FD_ISSET(client, &exlist)) {
-			printf("\nclient exception occurred\n");
+			TCRIT("\nclient exception occurred\n");
 			break;
 		}
 
 		if (ret == -1) {//select error
-			printf("select(): %s\n", strerror(errno));
+			TCRIT("select(): %s\n", strerror(errno));
 			break;
 		} else if (ret == 0) {//select timedout
 				continue;
@@ -1074,13 +1087,13 @@
 							(SSL_get_error(ws_ctx->ssl, bytes) == SSL_ERROR_SYSCALL)) {
 						goto read_again;
 					}
-					printf("\n SSL client closed connection  sslerr:%d \n", SSL_get_error(ws_ctx->ssl, bytes));
+					TCRIT("\n SSL client closed connection  sslerr:%d \n", SSL_get_error(ws_ctx->ssl, bytes));
 				}
 				else{
 					if (errno == EAGAIN) {
 						goto read_again;
 					}
-					printf("\n  Non SSL client closed connection  err:%d \n", errno);
+					TCRIT("\n  Non SSL client closed connection  err:%d \n", errno);
 				}
 				break;
 			}
@@ -1092,12 +1105,12 @@
 			}
 
 			if (opcode == 8) {
-				printf("\nclient sent orderly close frame\n");
+				TCRIT("\nclient sent orderly close frame\n");
 				break;
 			}
 
 			if (len < 0) {
-				printf("\ndecoding error\n");
+				TCRIT("\ndecoding error\n");
 				fflush(stdout);
 			}
 
@@ -1121,7 +1134,7 @@
 				len = sout_end - sout_start;
 				bytes = send(server, ws_ctx->tout_buf + sout_start, len, 0);
 				if (bytes < 0) {
-					printf("\ntarget connection error: %s\n", strerror(errno));
+					TCRIT("\ntarget connection error: %s\n", strerror(errno));
 					break;
 				}
 				sout_start += bytes;
@@ -1216,22 +1229,22 @@
 		if (ret <= 0) {
 			if (errno != 0) {
 				if(errno != EAGAIN) {
-					printf("\nWebSocketProxy: select failed ret:%d err:%d :%s\n", ret, errno, strerror(errno));
+					TCRIT("\nWebSocketProxy: select failed ret:%d err:%d :%s\n", ret, errno, strerror(errno));
 					break;
 				}
 			}
 		}
 
 		if (FD_ISSET(server, &exlist)) {
-			printf("\ntarget exception occurred\n");
+			TCRIT("\ntarget exception occurred\n");
 			break;
 		}
 
 		if (ret == -1) {
-			printf("select(): %s\n", strerror(errno));
+			TCRIT("select(): %s\n", strerror(errno));
 			break;
 		} else if (ret == 0) {
-			//printf("\nselect timedout\n");
+			//TCRIT("\nselect timedout\n");
 			continue;
 		}
 
@@ -1239,7 +1252,7 @@
 		if (FD_ISSET(server, &rdlist)) {
 			bytes = recv(server, ws_ctx->cin_buf + MAX_WEBSOCK_HEADER_LEN, BUFSIZEWOHDR, 0);
 			if (bytes <= 0) {
-				printf("\ntarget closed connection  errno:%d\n", errno);
+				TCRIT("\ntarget closed connection  errno:%d\n", errno);
 				break;
 			}
 
@@ -1248,7 +1261,7 @@
 				cout_end = form_encode_websocket_hybi_hdr(bytes, ws_ctx->cin_buf, BUFSIZE, &start_point);
 			}
 			if (cout_end < 0) {
-				printf("encoding error\n");
+				TCRIT("encoding error\n");
 				fflush(stdout);
 				break;
 			}
@@ -1272,18 +1285,18 @@
 							if ((errno == EAGAIN) && ((ssl_err == SSL_ERROR_WANT_WRITE) || (ssl_err == SSL_ERROR_SYSCALL))){
 								goto WebClient_write_again;
 							}
-							printf("\n WritetoWebClient read failed: errno == %d\t %d\n", errno,	SSL_get_error(ws_ctx->ssl, bytes));
+							TCRIT("\n WritetoWebClient read failed: errno == %d\t %d\n", errno,	SSL_get_error(ws_ctx->ssl, bytes));
 						}
 						else{
 							if (errno == EAGAIN) {
 								goto WebClient_write_again;
 							}
-							printf("\n Non SSL read failed: errno == %d \n", errno);
+							TCRIT("\n Non SSL read failed: errno == %d \n", errno);
 						}
 						break;
 					}
 					if (len < 3) {
-						printf("len: %d, bytes: %d: %d\n", (int) len, (int) bytes, (int) *(ws_ctx->cout_buf + cout_start));
+						TCRIT("len: %d, bytes: %d: %d\n", (int) len, (int) bytes, (int) *(ws_ctx->cout_buf + cout_start));
 					}
 					cout_start += bytes;
 				}
@@ -1347,21 +1360,30 @@
 	{
 		if (0 != pthread_create(&ReadFromCliThread[thread_no], NULL, ReadFromWebClient,
 				(void *) thread_no)) {
-			printf("\nUnable to Create write_thread  no:%d \n", thread_no);
+			TCRIT("\nUnable to Create write_thread  no:%d \n", thread_no);
 			return -1;
 		}
 
 		if (0 != pthread_create(&WriteToCliThread[thread_no], NULL, WritetoWebClient,
 				(void *) thread_no)) {
-			printf("\nUnable to Create write_thread  no:%d \n", thread_no);
+			TCRIT("\nUnable to Create write_thread  no:%d \n", thread_no);
 			return -1;
 		}
 		
 	}
 	else{
-		if (0 != pthread_create(&WebSockThread[thread_no], NULL, WebSocketProxy,
-				(void *) thread_no)) {
-			printf("\nUnable to Create WebSockThread  no:%d \n", thread_no);
+		ws_ctx_cd = &WebSockCtx[thread_no];
+		media_instance = thread_no;
+		if (mod_usbfd <= 0) {
+			/* Open the USB Device */
+			mod_usbfd = open(USB_DEVICE, O_RDWR);
+			if (mod_usbfd < 0) {
+				TCRIT("Error in opening USB Device\n");
+				return 1;
+			}
+		}
+		if (0 != pthread_create(&WebSockThread[thread_no], NULL, WebSocketProxy, NULL)) {
+			TCRIT("\nUnable to Create WebSockThread  no:%d \n", thread_no);
 			return -1;
 		}
 	}
@@ -1543,14 +1565,14 @@
     dl_servicehandle = dlopen(NCML_LIB,RTLD_LAZY);
     if(dl_servicehandle == NULL)
     {
-        printf("Problem in loading library:%s\n",dlerror());
+        TCRIT("Problem in loading library:%s\n",dlerror());
         return ERR_GET_CONF;
     }
 
     dl_get_service_conf = dlsym(dl_servicehandle,"get_service_configurations");
     if(dl_get_service_conf == NULL)
     {
-        printf("No Symbol Found:%s\n",dlerror());
+        TCRIT("No Symbol Found:%s\n",dlerror());
         dlclose(dl_servicehandle);
         return ERR_GET_CONF;
     }
@@ -1593,14 +1615,14 @@
     dl_ncmlhandle = dlopen(NCML_LIB,RTLD_LAZY);
     if(dl_ncmlhandle == NULL)
     {
-        printf("Problem in loading library:%s\n",dlerror());
+        TCRIT("Problem in loading library:%s\n",dlerror());
         return -1;
     }
 if(0)port_num=port_num;
     dl_getNotEditableData = dlsym(dl_ncmlhandle, "getNotEditableData");
     if(dl_getNotEditableData == NULL)
     {
-        printf("No Symbol Found:%s\n",dlerror());
+        TCRIT("No Symbol Found:%s\n",dlerror());
         dlclose(dl_ncmlhandle);
         return -1;
     }
@@ -2036,19 +2058,19 @@
 	memset(&(WebSockCtx[Instance]), 0, sizeof(webs_ctx));
 
 	if (!(WebSockCtx[Instance].cin_buf = malloc(BUFSIZE))) {
-		printf("malloc of cin_buf\n");
+		TCRIT("malloc of cin_buf\n");
 		return -1;
 	}
 	if (!(WebSockCtx[Instance].cout_buf = malloc(BUFSIZE))) {
-		printf("malloc of cout_buf\n");
+		TCRIT("malloc of cout_buf\n");
 		return -1;
 	}
 	if (!(WebSockCtx[Instance].tin_buf = malloc(BUFSIZE))) {
-		printf("malloc of tin_buf\n");
+		TCRIT("malloc of tin_buf\n");
 		return -1;
 	}
 	if (!(WebSockCtx[Instance].tout_buf = malloc(BUFSIZE))) {
-		printf("malloc of tout_buf\n");
+		TCRIT("malloc of tout_buf\n");
 		return -1;
 	}
 
@@ -2208,7 +2230,7 @@
 			Instance = WSFindFreeSlot();
 
 			if (Instance < 0 || Instance > MAX_INSTANCES) {
-				printf("\nInvalid instance %d\n", Instance);
+				TCRIT("\nInvalid instance %d\n", Instance);
 				break;
 			}
 
