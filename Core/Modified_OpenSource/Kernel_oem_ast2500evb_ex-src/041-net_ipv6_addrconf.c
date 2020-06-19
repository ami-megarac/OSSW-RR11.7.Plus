--- linux.org/net/ipv6/addrconf.c	Thu Aug 14 09:38:34 2014
+++ linux/net/ipv6/addrconf.c	Thu Jan 18 21:29:25 2018
@@ -4798,6 +4798,21 @@
 	return ret;
 }
 
+static
+int addrconf_sysctl_mtu(struct ctl_table *ctl, int write,
+			void __user *buffer, size_t *lenp, loff_t *ppos)
+{
+	struct inet6_dev *idev = ctl->extra1;
+	int min_mtu = IPV6_MIN_MTU;
+	struct ctl_table lctl;
+
+	lctl = *ctl;
+	lctl.extra1 = &min_mtu;
+	lctl.extra2 = idev ? &idev->dev->mtu : NULL;
+
+	return proc_dointvec_minmax(&lctl, write, buffer, lenp, ppos);
+}
+
 static void dev_disable_change(struct inet6_dev *idev)
 {
 	struct netdev_notifier_info info;
@@ -4949,7 +4964,7 @@
 			.data		= &ipv6_devconf.mtu6,
 			.maxlen		= sizeof(int),
 			.mode		= 0644,
-			.proc_handler	= proc_dointvec,
+			.proc_handler	= addrconf_sysctl_mtu,
 		},
 		{
 			.procname	= "accept_ra",
