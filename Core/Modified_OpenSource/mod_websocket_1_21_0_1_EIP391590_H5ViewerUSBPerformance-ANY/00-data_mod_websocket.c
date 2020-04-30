--- .pristine/mod_websocket-1.21.0-src/data/mod_websocket.c	Thu Feb 22 17:48:58 2018
+++ source/mod_websocket-1.21.0-src/data/mod_websocket.c	Thu Mar  8 16:21:43 2018
@@ -13,8 +13,8 @@
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
-
-#include "dbgout.h"
+#include <time.h>
+
 #include "dlfcn.h"
 #include<errno.h>
 #include <sys/socket.h>
@@ -23,6 +23,7 @@
 #include    <ncml.h>
 #include <arpa/inet.h>
 #include <sys/syscall.h>
+#include <sys/sysinfo.h>
 #include    <vmedia_cfg.h>
 #include "iusb.h"
 
@@ -41,8 +42,8 @@
 #include "apphead.h"
 #include "sessioncfg.h"
 #include "redirimage.h"
+#include "token.h"
 #include <semaphore.h>
-#include <pthread.h>
 
 #ifdef	HAVE_PCRE_H
 # include <pcre.h>
@@ -69,6 +70,7 @@
 static handler_t mod_websocket_handle_subrequest(server *, connection *, void *);
 int get_service_config(buffer *service, SERVICE_CONF_STRUCT *ServiceConfig);
 int get_port(SERVICE_CONF_STRUCT ServiceConfig, buffer *port);
+static int cd_instance = -1;
 #define OPCODE 2
 typedef struct {
     int        targetfd;
@@ -82,8 +84,8 @@
     char      *cout_buf;
     char      *tin_buf;
     char      *tout_buf;
-  int        cli_stopped;
-  int        ser_stopped;
+    int        cli_stopped;
+    int        ser_stopped;
 } webs_ctx;
 
 /**
@@ -99,7 +101,7 @@
 
 #define IVTP_CONNECTION_FAILED				(0x003B)
 
-#define MAX_INSTANCES	4
+#define MAX_INSTANCES	8
 pthread_t WebSockThread[MAX_INSTANCES];
 
 pthread_t ReadFromCliThread[MAX_INSTANCES];
@@ -118,6 +120,8 @@
 #define MAXLINE 80
 #define MAX_WEBSOCK_HEADER_LEN 10
 #define BUFSIZEWOHDR (BUFSIZE - MAX_WEBSOCK_HEADER_LEN)  //buffer without header
+
+#define IUSB_HEADER_SIZE 61
 
 #define ntohl64(p) \
     ((((uint64_t)((p)[7])) <<  0) + (((uint64_t)((p)[6])) <<  8) +\
@@ -573,13 +577,169 @@
 			WebSockCtx[(int) instance].ssl = NULL;
 	}
 	WebSockCtx[(int) instance].media = 0;
-	
+	/* To ensure proper cleanup, memset the websocket instance once the
+	** cleanup is done. */
+	memset(&WebSockCtx[instance], 0, sizeof(webs_ctx));
 	return 0;
 }
 
 void NanoSleep(unsigned long nsec) {
     struct timespec delay = { nsec / 1000000000, nsec % 1000000000 };
     pselect(0, NULL, NULL, NULL, &delay, NULL);
+}
+
+void
+mod_proxySetPriority( int prio )
+{
+  int ret;
+  int policy = 0;
+  struct sched_param params;
+
+  // We'll operate on the currently running thread.
+  pthread_t this_thread = pthread_self();
+
+  ret = pthread_getschedparam(this_thread, &policy, &params);
+  if (ret != 0) {
+    printf( "ERROR getting scheduling params ret=%d\n", ret);
+    return;
+  }
+
+  // We'll set the priority to the maximum.
+//  params.sched_priority = sched_get_priority_max( prio );
+  params.sched_priority += 1;
+
+  // Attempt to set thread real-time priority to the SCHED_FIFO policy
+  ret = pthread_setschedparam(this_thread, prio, &params);
+  if (ret != 0) {
+    printf( "ERROR setting thread prio to %d ret=%d\n", params.sched_priority, ret);
+  }
+
+  ret = pthread_getschedparam(this_thread, &policy, &params);
+  if (ret != 0) {
+    printf( "Couldn't retrieve real-time scheduling params ret=%d\n", ret);;
+  }
+
+}
+
+void sendToCdserver(int server, char *tout_buf, int len)
+{
+	int bytes = 0;
+	unsigned int sout_start, sout_end;
+
+	sout_start = 0;
+	sout_end = len;
+
+	while(1) {
+		bytes = send(server, tout_buf, len, 0);
+		if (bytes < 0) {
+			printf("\ntarget connection error: %s\n", strerror(errno));
+			break;
+		}
+		sout_start += bytes;
+		if (sout_start >= sout_end) {
+			sout_start = sout_end = 0;
+			break;
+		}
+	}
+}
+
+void sendToH5Viewer(void *instance, char *cin_buf, int bytes, int hybi)
+{
+	int len = 0, cout_start = 0, cout_end = 0, start_point = 0;
+	webs_ctx *ws_ctx = &(WebSockCtx[(int) instance]);
+	int ssl_err = 0;
+
+	cout_start = 0;
+	if (hybi) {
+		cout_end = form_encode_websocket_hybi_hdr(bytes, cin_buf, BUFSIZE, &start_point);
+	}
+	if (cout_end < 0) {
+		printf("encoding error\n");
+		fflush(stdout);
+		return;
+	}
+	
+	cout_start = MAX_WEBSOCK_HEADER_LEN - start_point;
+	//Send data to client which is received from server
+	{
+		write_again: 
+		len = cout_end - cout_start;
+		if(len > 0)
+		{
+			bytes = ws_send(ws_ctx, cin_buf + cout_start, len);
+			if (bytes < 0) {
+				if (ws_ctx->ssl != NULL) {
+					ssl_err = SSL_get_error(ws_ctx->ssl, bytes);
+					if ((errno == EAGAIN) && ((ssl_err == SSL_ERROR_WANT_WRITE) || (ssl_err == SSL_ERROR_SYSCALL)))
+						goto write_again;
+					printf("\n read failed: errno == %d\t %d\n", errno, SSL_get_error(ws_ctx->ssl, bytes));
+				}
+				else {
+					if (errno == EAGAIN) {
+						goto write_again;
+					}
+					printf("\n Non SSL read failed: errno == %d \n", errno);
+				}
+				return;
+			}
+			if (len < 3) {
+				printf("len: %d, bytes: %d: %d\n", (int) len, (int) bytes, (int) *(ws_ctx->cout_buf + cout_start));
+			}
+			cout_start += bytes;
+		}
+		// if the whole data is not sent to H5Viewer, call write again
+		if ((cout_end - cout_start)> 0)
+		{
+			goto write_again;
+		}
+		else
+		{
+			len = cout_start = cout_end = 0;
+		}
+	}
+	
+	ws_ctx = NULL;
+}
+
+void *WebSCSIHndlr(void *instance) {
+	pthread_t self;
+	webs_ctx *ws_ctx = &(WebSockCtx[(int) instance]);
+	IUSB_SCSI_PACKET *tmp_ReqPkt = NULL;
+	u8* ioctl_data;
+	int Retval = 0, cmd_len;
+	
+
+	/* Create commuincation Pipe between parent and thread */
+	if(cd_instance != -1) {
+		Retval = mod_request_usb_dev();
+		if(Retval == 1)
+		{
+			printf("WebSCSIHndlr, request usb dev failed\n");
+			return NULL;
+		}
+		
+		mod_ProcessActivate (cd_instance);
+	}		
+
+	while (cd_instance != -1) {
+		ioctl_data = (u8*) mod_AddAuthInfo (USB_CDROM_REQ,(u8*)ws_ctx->cin_buf + MAX_WEBSOCK_HEADER_LEN, cd_instance);
+
+		Retval = ioctl(mod_usbfd,USB_CDROM_REQ,ioctl_data);
+		if (Retval < 0) {
+			break;
+		}
+						
+		tmp_ReqPkt =(IUSB_SCSI_PACKET *) ( ws_ctx->cin_buf + MAX_WEBSOCK_HEADER_LEN);
+		cmd_len = mac2long(tmp_ReqPkt->Header.DataPktLen) + sizeof(IUSB_HEADER);			
+		
+		sendToH5Viewer(instance, ws_ctx->cin_buf, cmd_len, ws_ctx->hybi);		
+	}
+	//disconnect the thread
+	ws_ctx = NULL;
+	tmp_ReqPkt = NULL;
+	self = pthread_self();
+	pthread_detach(self);
+	pthread_exit(NULL);
 }
 
 void *WebSocketProxy(void *instance) {
@@ -589,17 +749,27 @@
 	int ret, client = ws_ctx->sockfd;
 	int server = 0;
 	unsigned int opcode, left;
-	unsigned int sout_start, sout_end;
+	unsigned int sout_end;
 	uint64_t sin_start, sin_end;
-	int len, bytes, cout_start, cout_end, maxfd,start_point;
+	int len, bytes, maxfd;
 	int current_fin = -1;
 	pthread_t self;
 	int ssl_err = 0;
 	int flag = 1; 
-
-	len = sout_start = sout_end = cout_start = cout_end = 0 ,start_point=0;
-	sin_start = sin_end = 0;
-
+	
+	IUSB_SCSI_PACKET *TempResPkt = NULL;
+	DEV_REDIR_ACK	*AckCommand = NULL;
+	pthread_t mod_ChildId;
+
+	len = sin_start = sin_end = sout_end = 0;
+	cd_instance = -1;
+
+	// Set thread name to something usefull
+	prctl(PR_SET_NAME,__FUNCTION__,0,0,0);
+
+	// Raise priority of the input processing (mouse/keyboard)
+	mod_proxySetPriority( SCHED_RR );
+	
 	server = ws_ctx->targetfd;
 	ws_ctx->hybi = 1;
 	maxfd = client > server ? client + 1 : server + 1;
@@ -616,15 +786,9 @@
 
 		FD_SET(client, &exlist);
 		FD_SET(server, &exlist);
-
-		if (sout_end == sout_start) {
-			// nothing is left for server , so read from client
-			FD_SET(client, &rdlist);
-		}
-		if (cout_end == cout_start) {
-			// nothing is left for client, so read from server
-			FD_SET(server, &rdlist);
-		}
+		
+		FD_SET(client, &rdlist);
+		FD_SET(server, &rdlist);
 
 		ret = select(maxfd, &rdlist, NULL, &exlist, &tv);
 		if (ret <= 0) {
@@ -682,7 +846,7 @@
 			sin_end += bytes;
 
 			if (ws_ctx->hybi) {
-				len = decode_websocket_hybi((unsigned char *) ws_ctx->tin_buf + sin_start, (uint64_t) sin_end - sin_start, (u_char *) ws_ctx->tout_buf, BUFSIZE - 1, &opcode, &left, &current_fin);
+				len = decode_websocket_hybi((unsigned char *) ws_ctx->tin_buf + sin_start, (uint64_t) sin_end - sin_start, (u_char *) ws_ctx->tout_buf + sout_end, BUFSIZE - 1, &opcode, &left, &current_fin);
 			}
 
 			if (opcode == 8) {
@@ -702,39 +866,55 @@
 				sin_end = 0;
 			}
 
-			sout_start = 0;
-			sout_end = len;
+			sout_end += len;
 
 			//Need to read more data inorder to complete the frame
 			// again read the data from socket.
 			if (left > 0) {
 				goto read_again;
 			}
+			TempResPkt = (IUSB_SCSI_PACKET *)ws_ctx->tout_buf;
+			if( TempResPkt->DataLen + IUSB_HEADER_SIZE != sout_end )
+			{
+				// If the fin is 0 then it means it is not the end frame. And more frames will be received as continuation frames.
+				// If the fin value is 1 then it means it is the end frame.
+				if (current_fin == 0) {
+					// Since there are multiple packets sent simultaneously in case of continuation frames
+					// so we can directly go for socket read without checking for socket event.
+					// Incase of continuation frames, the data should be read from the socket
+					// till we receive the end of the frames denoting the completion of continuation frames
+					// so that we don't miss out any packets in the socket
+					goto read_again;
+				}
+			}
+			
 			//Send data to server which is received from client
 			{
-				len = sout_end - sout_start;
-				bytes = send(server, ws_ctx->tout_buf + sout_start, len, 0);
-				if (bytes < 0) {
-					printf("\ntarget connection error: %s\n", strerror(errno));
-					break;
+				if(TempResPkt->CommandPkt.OpCode != MEDIA_SESSION_DISCONNECT &&
+						TempResPkt->CommandPkt.OpCode != AUTH_CMD &&
+						TempResPkt->CommandPkt.OpCode != DEVICE_INFO &&
+						TempResPkt->CommandPkt.OpCode != CDROM_KEEPALIVE_SCSI_CMD) 
+				{
+					mod_SendDataToDriver(cd_instance, ws_ctx->tout_buf + 0);
 				}
-				sout_start += bytes;
-				if (sout_start >= sout_end) {
-					sout_start = sout_end = 0;
+				else
+				{
+					sendToCdserver(server, ws_ctx->tout_buf, len);
+
+					if(TempResPkt->CommandPkt.OpCode == MEDIA_SESSION_DISCONNECT) 
+					{
+						mod_ProcessDisconnect(cd_instance);
+						cd_instance = -1;
+						break;
+					}
 				}
-			}
-			// If the fin is 0 then it means it is not the end frame. And more frames will be received as continuation frames.
-			// If the fin value is 1 then it means it is the end frame.
-			if (current_fin == 0) {
-				// Since there are multiple packets sent simultaneously in case of continuation frames
-				// so we can directly go for socket read without checking for socket event.
-				// Incase of continuation frames, the data should be read from the socket
-				// till we receive the end of the frames denoting the completion of continuation frames
-				// so that we don't miss out any packets in the socket
-				goto read_again;
-			}
-		}
-
+				sout_end = 0;
+				sin_start = 0;
+				sin_end = 0;
+			}
+		}
+
+		//if there are some response from cdserver, it will received data and then send to H5Viewer
 		// read fd is set for server, so read data from server socket
 		if (FD_ISSET(server, &rdlist)) {
 			bytes = recv(server, ws_ctx->cin_buf + MAX_WEBSOCK_HEADER_LEN, BUFSIZEWOHDR, 0);
@@ -742,60 +922,37 @@
 				printf("\ntarget closed connection  errno:%d\n", errno);
 				break;
 			}
-
-			cout_start = 0;
-			if (ws_ctx->hybi) {
-				cout_end = form_encode_websocket_hybi_hdr(bytes, ws_ctx->cin_buf, BUFSIZE, &start_point);
-			}
-			if (cout_end < 0) {
-				printf("encoding error\n");
-				fflush(stdout);
-				break;
-			}
-			cout_start = MAX_WEBSOCK_HEADER_LEN - start_point;
-			//Send data to client which is received from server
-			{
-				write_again: 
-				len = cout_end - cout_start;
-				if(len > 0)
-				{
-					bytes = ws_send(ws_ctx, ws_ctx->cin_buf + cout_start, len);
-					if (bytes < 0) {
-						if (ws_ctx->ssl != NULL) {
-							ssl_err = SSL_get_error(ws_ctx->ssl, bytes);
-							if ((errno == EAGAIN) && ((ssl_err == SSL_ERROR_WANT_WRITE) || (ssl_err == SSL_ERROR_SYSCALL)))
-								goto write_again;
-							printf("\n read failed: errno == %d\t %d\n", errno,	SSL_get_error(ws_ctx->ssl, bytes));
-						}
-						else{
-							if (errno == EAGAIN) {
-								goto write_again;
-							}
-							printf("\n Non SSL read failed: errno == %d \n", errno);
-						}
+			
+			AckCommand = (DEV_REDIR_ACK	*)(ws_ctx->cin_buf + MAX_WEBSOCK_HEADER_LEN);
+			if(AckCommand->iUsbScsiPkt.CommandPkt.OpCode == DEVICE_REDIRECTION_ACK) {
+				if(AckCommand->ConnectionStatus == CONNECTION_ACCEPTED) {
+					
+					cd_instance = AckCommand->iUsbScsiPkt.Header.Instance;
+					if (0 != pthread_create(&mod_ChildId, NULL, WebSCSIHndlr, (void *) instance) ) {
+						printf("\nUnable to Create WebSockThread  no:%d \n", (int)instance);
 						break;
 					}
-					if (len < 3) {
-						printf("len: %d, bytes: %d: %d\n", (int) len, (int) bytes, (int) *(ws_ctx->cout_buf + cout_start));
-					}
-					cout_start += bytes;
 				}
-				// if the whole data is not sent to H5Viewer, call write again
-				if ((cout_end - cout_start)> 0)
-				{
-					goto write_again;
-				}
-				else
-				{
-					len = cout_start = cout_end = 0;
-				}
-			}
-		}
+				else//cdserver reject connection
+					break;
+			}
+
+			sendToH5Viewer(instance, ws_ctx->cin_buf, bytes, ws_ctx->hybi);
+		}
+
 		// Add sleep to make CPU happy.
 		// Without this sleep, the thread might shoot up the CPU usage if the server runs faster than the kvm client
-		NanoSleep(1);
-	}
-
+		if ((FD_ISSET(client, &rdlist)) || (FD_ISSET(server, &rdlist))) 
+		{
+		    NanoSleep(1);
+		}
+	}
+	TempResPkt = NULL;
+	AckCommand = NULL;
+	pthread_cancel (mod_ChildId);
+	pthread_join(mod_ChildId, NULL);
+
+	mod_release_usb_dev();
 	// cleanup resources
 	cleanup_instance((int) instance);
 
@@ -807,8 +964,6 @@
 	// exit
 	return NULL;
 }
-
-
 
 void *ReadFromWebClient(void *instance) {
 	fd_set rdlist,  exlist;
@@ -863,20 +1018,23 @@
 			break;
 		}
 
-		if (ret == -1) {
+		if (ret == -1) {//select error
 			printf("select(): %s\n", strerror(errno));
 			break;
-		} else if (ret == 0) {
-			//printf("\nselect timedout\n");
-			continue;
-		}
-
+		} else if (ret == 0) {//select timedout
+				continue;
+		}
 		// read fd is set for client, so read data from client socket
 		if (FD_ISSET(client, &rdlist)) {
 			read_again:
 			//when full websocket frame is not received in single read
 			//should read the socket directly without going for select()
 			//removing "goto" jump will break huge data transfer
+
+			//stress test for start / stop H5Viewer, there is missing ser_stopped
+			//and make a slot not free
+			if(ws_ctx->ser_stopped == 1)
+				break;
 
 			bytes = (uint64_t) ws_recv(ws_ctx, ws_ctx->tin_buf + sin_end, BUFSIZE - 1);
 			if (bytes <= 0) {
@@ -963,6 +1121,9 @@
 		//		NanoSleep(1);
 		//	}
 	}
+	//to prevent client and server exit while loop at the same time
+	// and both of them did not free slot, let server stop delay 1 nano sec
+	NanoSleep(1);
 
 	if(ws_ctx->ser_stopped == 1)
 	{
@@ -1064,7 +1225,13 @@
 			cout_start = MAX_WEBSOCK_HEADER_LEN - start_point;
 			//Send data to client which is received from server
 			{
-				WebClient_write_again: 
+				WebClient_write_again:
+
+				//stress test for start / stop H5Viewer, there is missing cli_stopped
+				//and make a slot not free
+				if(ws_ctx->cli_stopped == 1)
+					break;
+
 				len = cout_end - cout_start;
 				if(len > 0)
 				{
@@ -2036,14 +2203,21 @@
 			}
 		}
 
-	fdevent_event_del(srv->ev, &(con->fde_ndx), con->fd);
-	fdevent_event_set(srv->ev, &(con->fde_ndx), con->fd, FDEVENT_IN);
-	//Dont set the connection state here
-	//connection_set_state(srv, hctx->con, CON_STATE_READ_CONTINUOUS_WEB_APP_PROXY);
-
+		fdevent_event_del(srv->ev, &(con->fde_ndx), con->fd);
+		fdevent_event_set(srv->ev, &(con->fde_ndx), con->fd, FDEVENT_IN);
+		//Dont set the connection state here
+		//connection_set_state(srv, hctx->con, CON_STATE_READ_CONTINUOUS_WEB_APP_PROXY);
         return HANDLER_WAIT_FOR_EVENT;
 
     case MOD_WEBSOCKET_STATE_CONNECTED:
+    	/* incase of kvm/cd connections, return handler_wait_for_event
+    	 * breaking out of the switch case might lead to lighttpd crash
+    	 * since h5viewer kvm/cd connection doesn't use hctx->fromcli,
+    	 * hctx->tocli, hctx->fd, hctx->tosrv etc
+    	 */
+    	if ((strncmp(hctx->con->request.uri->ptr, MOD_WEBSOCKET_KVM_URI, strlen(MOD_WEBSOCKET_KVM_URI)) == 0) || (strncmp(hctx->con->request.uri->ptr, MOD_WEBSOCKET_CDSERVER_URI, strlen(MOD_WEBSOCKET_CDSERVER_URI)) == 0)) {
+    		return HANDLER_WAIT_FOR_EVENT;
+    	}
         if (hctx->con->fd < 0) {
             break;
         } else {
