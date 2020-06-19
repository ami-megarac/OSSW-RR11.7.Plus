--- /home/nico/MDS/MDS4.0.2/workspace/ast2500evb_workspace/Build/kernel/linux.org/net/packet/af_packet.c	Tue Jan 23 16:07:16 2018
+++ linux/net/packet/af_packet.c	Tue Jan 23 16:45:49 2018
@@ -3254,6 +3254,8 @@
 			return -EINVAL;
 		if (copy_from_user(&val, optval, sizeof(val)))
 			return -EFAULT;
+		if (val > INT_MAX)
+			return -EINVAL;
 
 		pkt_sk(sk)->copy_thresh = val;
 		return 0;
