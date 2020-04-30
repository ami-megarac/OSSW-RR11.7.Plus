--- .pristine/mod_websocket-1.21.0-src/data/mod_websocket.c	Tue Jul 10 14:40:12 2018
+++ source/mod_websocket-1.21.0-src/data/mod_websocket.c	Fri Aug 31 14:24:14 2018
@@ -532,7 +532,7 @@
 
 	if (framecount > 1) {
 		snprintf((char *) cntstr, 3, "%d", framecount);
-		printf("cntstr :%s\n", (char *) cntstr);
+//		printf("cntstr :%s\n", (char *) cntstr);
 	}
 
 	*left = remaining;
@@ -760,6 +760,10 @@
 	IUSB_SCSI_PACKET *TempResPkt = NULL;
 	DEV_REDIR_ACK	*AckCommand = NULL;
 	pthread_t mod_ChildId;
+	
+	time_t last_h5viewer_pkt_time = 0;
+	struct sysinfo sys_info;
+	double timedout = 0;
 
 	len = sin_start = sin_end = sout_end = 0;
 	cd_instance = -1;
@@ -890,6 +894,12 @@
 			
 			//Send data to server which is received from client
 			{
+				if(last_h5viewer_pkt_time == 0) {
+					if(!sysinfo(&sys_info)) {
+						last_h5viewer_pkt_time = sys_info.uptime;
+					}
+				}
+				
 				if(TempResPkt->CommandPkt.OpCode != MEDIA_SESSION_DISCONNECT &&
 						TempResPkt->CommandPkt.OpCode != AUTH_CMD &&
 						TempResPkt->CommandPkt.OpCode != DEVICE_INFO &&
@@ -900,6 +910,9 @@
 				else
 				{
 					sendToCdserver(server, ws_ctx->tout_buf, len);
+					if(!sysinfo(&sys_info)) {
+						last_h5viewer_pkt_time = sys_info.uptime;
+					}
 
 					if(TempResPkt->CommandPkt.OpCode == MEDIA_SESSION_DISCONNECT) 
 					{
@@ -911,6 +924,23 @@
 				sout_end = 0;
 				sin_start = 0;
 				sin_end = 0;
+
+				if(!sysinfo(&sys_info)) {
+					timedout = difftime(sys_info.uptime, last_h5viewer_pkt_time);
+					if(timedout > (SELECT_REMOTE_RESPONSE_TIMEOUT_SECS / 2) ) {
+						IUSB_SCSI_PACKET RemoteHBPkt;
+						memset(&RemoteHBPkt,0,sizeof(IUSB_SCSI_PACKET));
+
+						RemoteHBPkt.Header.Instance = cd_instance;
+						strncpy((char *)(RemoteHBPkt.Header.Signature), "IUSB    ", strlen("IUSB    ") );
+						RemoteHBPkt.CommandPkt.OpCode = WEBSOCKET_KEEP_ALIVE_CMD; /* OpCode */
+						RemoteHBPkt.Header.DataPktLen = mac2long(sizeof(IUSB_SCSI_PACKET) - sizeof(IUSB_HEADER));
+
+						len = mac2long(RemoteHBPkt.Header.DataPktLen) + sizeof(IUSB_HEADER);
+						sendToCdserver(server, (char *)&RemoteHBPkt, len);
+						last_h5viewer_pkt_time = 0;
+					}
+				}
 			}
 		}
 
