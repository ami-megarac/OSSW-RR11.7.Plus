--- linux.org/net/xfrm/xfrm_user.c	Thu Aug 14 09:38:34 2014
+++ linux/net/xfrm/xfrm_user.c	Mon Jan 22 14:53:38 2018
@@ -1530,31 +1530,33 @@
 
 static int xfrm_dump_policy_done(struct netlink_callback *cb)
 {
-	struct xfrm_policy_walk *walk = (struct xfrm_policy_walk *) &cb->args[1];
+	struct xfrm_policy_walk *walk = (struct xfrm_policy_walk *)cb->args;
 	struct net *net = sock_net(cb->skb->sk);
 
 	xfrm_policy_walk_done(walk, net);
 	return 0;
 }
 
+static int xfrm_dump_policy_start(struct netlink_callback *cb)
+{
+	struct xfrm_policy_walk *walk = (struct xfrm_policy_walk *)cb->args;
+
+	BUILD_BUG_ON(sizeof(*walk) > sizeof(cb->args));
+
+	xfrm_policy_walk_init(walk, XFRM_POLICY_TYPE_ANY);
+	return 0;
+}
+
 static int xfrm_dump_policy(struct sk_buff *skb, struct netlink_callback *cb)
 {
 	struct net *net = sock_net(skb->sk);
-	struct xfrm_policy_walk *walk = (struct xfrm_policy_walk *) &cb->args[1];
+	struct xfrm_policy_walk *walk = (struct xfrm_policy_walk *)cb->args;
 	struct xfrm_dump_info info;
-
-	BUILD_BUG_ON(sizeof(struct xfrm_policy_walk) >
-		     sizeof(cb->args) - sizeof(cb->args[0]));
 
 	info.in_skb = cb->skb;
 	info.out_skb = skb;
 	info.nlmsg_seq = cb->nlh->nlmsg_seq;
 	info.nlmsg_flags = NLM_F_MULTI;
-
-	if (!cb->args[0]) {
-		cb->args[0] = 1;
-		xfrm_policy_walk_init(walk, XFRM_POLICY_TYPE_ANY);
-	}
 
 	(void) xfrm_policy_walk(net, walk, dump_one_policy, &info);
 
@@ -2306,6 +2308,7 @@
 
 static const struct xfrm_link {
 	int (*doit)(struct sk_buff *, struct nlmsghdr *, struct nlattr **);
+	int (*start)(struct netlink_callback *);
 	int (*dump)(struct sk_buff *, struct netlink_callback *);
 	int (*done)(struct netlink_callback *);
 } xfrm_dispatch[XFRM_NR_MSGTYPES] = {
@@ -2317,6 +2320,7 @@
 	[XFRM_MSG_NEWPOLICY   - XFRM_MSG_BASE] = { .doit = xfrm_add_policy    },
 	[XFRM_MSG_DELPOLICY   - XFRM_MSG_BASE] = { .doit = xfrm_get_policy    },
 	[XFRM_MSG_GETPOLICY   - XFRM_MSG_BASE] = { .doit = xfrm_get_policy,
+							.start = xfrm_dump_policy_start,
 						   .dump = xfrm_dump_policy,
 						   .done = xfrm_dump_policy_done },
 	[XFRM_MSG_ALLOCSPI    - XFRM_MSG_BASE] = { .doit = xfrm_alloc_userspi },
@@ -2360,6 +2364,7 @@
 
 		{
 			struct netlink_dump_control c = {
+				.start = link->start,
 				.dump = link->dump,
 				.done = link->done,
 			};
