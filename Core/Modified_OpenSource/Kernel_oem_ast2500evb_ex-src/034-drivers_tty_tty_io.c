--- linux.org/drivers/tty/tty_io.c	Thu Aug 14 09:38:34 2014
+++ linux/drivers/tty/tty_io.c	Thu Jan 18 21:15:16 2018
@@ -2576,6 +2576,28 @@
 }
 
 /**
+ *	tiocgetd	-	get line discipline
+ *	@tty: tty device
+ *	@p: pointer to user data
+ *
+ *	Retrieves the line discipline id directly from the ldisc.
+ *
+ *	Locking: waits for ldisc reference (in case the line discipline
+ *		is changing or the tty is being hungup)
+ */
+
+static int tiocgetd(struct tty_struct *tty, int __user *p)
+{
+	struct tty_ldisc *ld;
+	int ret;
+
+	ld = tty_ldisc_ref_wait(tty);
+	ret = put_user(ld->ops->num, p);
+	tty_ldisc_deref(ld);
+	return ret;
+}
+
+/**
  *	send_break	-	performed time break
  *	@tty: device to break on
  *	@duration: timeout in mS
@@ -2789,7 +2811,7 @@
 	case TIOCGSID:
 		return tiocgsid(tty, real_tty, p);
 	case TIOCGETD:
-		return put_user(tty->ldisc->ops->num, (int __user *)p);
+		return tiocgetd(tty, p);
 	case TIOCSETD:
 		return tiocsetd(tty, p);
 	case TIOCVHANGUP:
