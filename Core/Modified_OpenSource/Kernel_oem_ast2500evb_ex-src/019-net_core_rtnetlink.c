--- linux.org/net/core/rtnetlink.c	Tue Jan 16 17:23:59 2018
+++ linux/net/core/rtnetlink.c	Tue Jan 16 17:55:40 2018
@@ -1009,14 +1009,15 @@
 		goto nla_put_failure;
 
 	if (1) {
-		struct rtnl_link_ifmap map = {
-			.mem_start   = dev->mem_start,
-			.mem_end     = dev->mem_end,
-			.base_addr   = dev->base_addr,
-			.irq         = dev->irq,
-			.dma         = dev->dma,
-			.port        = dev->if_port,
-		};
+		struct rtnl_link_ifmap map;
+			memset(&map, 0, sizeof(map));
+			map.mem_start   = dev->mem_start;
+			map.mem_end     = dev->mem_end;
+			map.base_addr   = dev->base_addr;
+			map.irq         = dev->irq;
+			map.dma         = dev->dma;
+			map.port        = dev->if_port;
+
 		if (nla_put(skb, IFLA_MAP, sizeof(map), &map))
 			goto nla_put_failure;
 	}
