--- .pristine/mod_websocket-1.21.0-src/data/mod_websocket.c	Wed Nov  6 17:39:12 2019
+++ source/mod_websocket-1.21.0-src/data/mod_websocket.c	Wed Nov  6 19:08:32 2019
@@ -597,6 +597,39 @@
     pselect(0, NULL, NULL, NULL, &delay, NULL);
 }
 
+void
+mod_proxySetPriority( int prio )
+{
+	int ret;
+	int policy = 0;
+	struct sched_param params;
+	
+	// We'll operate on the currently running thread.
+	pthread_t this_thread = pthread_self();
+	
+	ret = pthread_getschedparam(this_thread, &policy, &params);
+	if (ret != 0) {
+		printf( "ERROR getting scheduling params ret=%d\n", ret);
+		return;
+	}
+	
+	// We'll set the priority to the maximum.
+	// params.sched_priority = sched_get_priority_max( prio );
+	params.sched_priority += 1;
+	
+	// Attempt to set thread real-time priority to the SCHED_FIFO policy
+	ret = pthread_setschedparam(this_thread, prio, &params);
+	if (ret != 0) {
+		printf( "ERROR setting thread prio to %d ret=%d\n", params.sched_priority, ret);
+	}
+	
+	ret = pthread_getschedparam(this_thread, &policy, &params);
+	if (ret != 0) {
+		printf( "Couldn't retrieve real-time scheduling params ret=%d\n", ret);
+	}
+	
+}
+
 /*
  * sendDataToServer: sends the given data to server socket
  * server: server socket
@@ -752,6 +785,8 @@
 	time_t h5PktRecvdTime = { 0 };
 	struct sysinfo sys_info;
 	double timedout = 0;
+
+	mod_proxySetPriority( SCHED_RR );
 
 	maxfd = ws_ctx_cd->sockfd > ws_ctx_cd->targetfd ? ws_ctx_cd->sockfd + 1 : ws_ctx_cd->targetfd + 1;
 	while (1) {
