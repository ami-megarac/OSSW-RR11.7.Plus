--- linux.org/net/packet/af_packet.c	Thu Aug 14 09:38:34 2014
+++ linux/net/packet/af_packet.c	Mon Jan 22 15:34:22 2018
@@ -3264,19 +3264,25 @@
 
 		if (optlen != sizeof(val))
 			return -EINVAL;
-		if (po->rx_ring.pg_vec || po->tx_ring.pg_vec)
-			return -EBUSY;
 		if (copy_from_user(&val, optval, sizeof(val)))
 			return -EFAULT;
 		switch (val) {
 		case TPACKET_V1:
 		case TPACKET_V2:
 		case TPACKET_V3:
-			po->tp_version = val;
-			return 0;
+			break;
 		default:
 			return -EINVAL;
 		}
+		lock_sock(sk);
+		if (po->rx_ring.pg_vec || po->tx_ring.pg_vec) {
+			ret = -EBUSY;
+		} else {
+			po->tp_version = val;
+			ret = 0;
+		}
+		release_sock(sk);
+		return ret;
 	}
 	case PACKET_RESERVE:
 	{
@@ -3284,12 +3290,17 @@
 
 		if (optlen != sizeof(val))
 			return -EINVAL;
-		if (po->rx_ring.pg_vec || po->tx_ring.pg_vec)
-			return -EBUSY;
 		if (copy_from_user(&val, optval, sizeof(val)))
 			return -EFAULT;
-		po->tp_reserve = val;
-		return 0;
+		lock_sock(sk);
+		if (po->rx_ring.pg_vec || po->tx_ring.pg_vec) {
+			ret = -EBUSY;
+		} else {
+			po->tp_reserve = val;
+			ret = 0;
+		}
+		release_sock(sk);
+		return ret;
 	}
 	case PACKET_LOSS:
 	{
@@ -3739,6 +3750,7 @@
 	/* Added to avoid minimal code churn */
 	struct tpacket_req *req = &req_u->req;
 
+	lock_sock(sk);
 	/* Opening a Tx-ring is NOT supported in TPACKET_V3 */
 	if (!closing && tx_ring && (po->tp_version > TPACKET_V2)) {
 		WARN(1, "Tx-ring is not supported.\n");
@@ -3816,7 +3828,6 @@
 			goto out;
 	}
 
-	lock_sock(sk);
 
 	/* Detach socket from network */
 	spin_lock(&po->bind_lock);
@@ -3870,6 +3881,7 @@
 	if (pg_vec)
 		free_pg_vec(pg_vec, order, req->tp_block_nr);
 out:
+	release_sock(sk);
 	return err;
 }
 
