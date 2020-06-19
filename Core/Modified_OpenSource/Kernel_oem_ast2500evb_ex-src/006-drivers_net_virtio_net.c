--- linux.org/drivers/net/virtio_net.c	Thu Aug 14 09:38:34 2014
+++ linux/drivers/net/virtio_net.c	Tue Jan 16 10:45:19 2018
@@ -1652,9 +1652,9 @@
 	/* Do we support "hardware" checksums? */
 	if (virtio_has_feature(vdev, VIRTIO_NET_F_CSUM)) {
 		/* This opens up the world of extra features. */
-		dev->hw_features |= NETIF_F_HW_CSUM|NETIF_F_SG|NETIF_F_FRAGLIST;
+		dev->hw_features |= NETIF_F_HW_CSUM|NETIF_F_SG;
 		if (csum)
-			dev->features |= NETIF_F_HW_CSUM|NETIF_F_SG|NETIF_F_FRAGLIST;
+			dev->features |= NETIF_F_HW_CSUM|NETIF_F_SG;
 
 		if (virtio_has_feature(vdev, VIRTIO_NET_F_GSO)) {
 			dev->hw_features |= NETIF_F_TSO | NETIF_F_UFO
