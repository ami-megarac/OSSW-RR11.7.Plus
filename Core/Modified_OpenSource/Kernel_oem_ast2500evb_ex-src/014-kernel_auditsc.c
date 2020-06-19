--- linux.org/kernel/auditsc.c	Thu Aug 14 09:38:34 2014
+++ linux/kernel/auditsc.c	Tue Jan 16 15:31:05 2018
@@ -68,6 +68,7 @@
 #include <linux/capability.h>
 #include <linux/fs_struct.h>
 #include <linux/compat.h>
+#include <linux/uaccess.h>
 
 #include "audit.h"
 
@@ -76,7 +77,8 @@
 #define AUDITSC_SUCCESS 1
 #define AUDITSC_FAILURE 2
 
-/* no execve audit message should be longer than this (userspace limits) */
+/* no execve audit message should be longer than this (userspace limits),
+ * see the note near the top of audit_log_execve_info() about this value */
 #define MAX_EXECVE_AUDIT_LEN 7500
 
 /* number of audit rules */
@@ -999,185 +1001,179 @@
 	return rc;
 }
 
-/*
- * to_send and len_sent accounting are very loose estimates.  We aren't
- * really worried about a hard cap to MAX_EXECVE_AUDIT_LEN so much as being
- * within about 500 bytes (next page boundary)
- *
- * why snprintf?  an int is up to 12 digits long.  if we just assumed when
- * logging that a[%d]= was going to be 16 characters long we would be wasting
- * space in every audit message.  In one 7500 byte message we can log up to
- * about 1000 min size arguments.  That comes down to about 50% waste of space
- * if we didn't do the snprintf to find out how long arg_num_len was.
- */
-static int audit_log_single_execve_arg(struct audit_context *context,
-					struct audit_buffer **ab,
-					int arg_num,
-					size_t *len_sent,
-					const char __user *p,
-					char *buf)
-{
-	char arg_num_len_buf[12];
-	const char __user *tmp_p = p;
-	/* how many digits are in arg_num? 5 is the length of ' a=""' */
-	size_t arg_num_len = snprintf(arg_num_len_buf, 12, "%d", arg_num) + 5;
-	size_t len, len_left, to_send;
-	size_t max_execve_audit_len = MAX_EXECVE_AUDIT_LEN;
-	unsigned int i, has_cntl = 0, too_long = 0;
-	int ret;
-
-	/* strnlen_user includes the null we don't want to send */
-	len_left = len = strnlen_user(p, MAX_ARG_STRLEN) - 1;
-
-	/*
-	 * We just created this mm, if we can't find the strings
-	 * we just copied into it something is _very_ wrong. Similar
-	 * for strings that are too long, we should not have created
-	 * any.
-	 */
-	if (unlikely((len == -1) || len > MAX_ARG_STRLEN - 1)) {
-		WARN_ON(1);
-		send_sig(SIGKILL, current, 0);
-		return -1;
-	}
-
-	/* walk the whole argument looking for non-ascii chars */
-	do {
-		if (len_left > MAX_EXECVE_AUDIT_LEN)
-			to_send = MAX_EXECVE_AUDIT_LEN;
-		else
-			to_send = len_left;
-		ret = copy_from_user(buf, tmp_p, to_send);
-		/*
-		 * There is no reason for this copy to be short. We just
-		 * copied them here, and the mm hasn't been exposed to user-
-		 * space yet.
-		 */
-		if (ret) {
-			WARN_ON(1);
-			send_sig(SIGKILL, current, 0);
-			return -1;
-		}
-		buf[to_send] = '\0';
-		has_cntl = audit_string_contains_control(buf, to_send);
-		if (has_cntl) {
-			/*
-			 * hex messages get logged as 2 bytes, so we can only
-			 * send half as much in each message
-			 */
-			max_execve_audit_len = MAX_EXECVE_AUDIT_LEN / 2;
-			break;
-		}
-		len_left -= to_send;
-		tmp_p += to_send;
-	} while (len_left > 0);
-
-	len_left = len;
-
-	if (len > max_execve_audit_len)
-		too_long = 1;
-
-	/* rewalk the argument actually logging the message */
-	for (i = 0; len_left > 0; i++) {
-		int room_left;
-
-		if (len_left > max_execve_audit_len)
-			to_send = max_execve_audit_len;
-		else
-			to_send = len_left;
-
-		/* do we have space left to send this argument in this ab? */
-		room_left = MAX_EXECVE_AUDIT_LEN - arg_num_len - *len_sent;
-		if (has_cntl)
-			room_left -= (to_send * 2);
-		else
-			room_left -= to_send;
-		if (room_left < 0) {
-			*len_sent = 0;
-			audit_log_end(*ab);
-			*ab = audit_log_start(context, GFP_KERNEL, AUDIT_EXECVE);
-			if (!*ab)
-				return 0;
-		}
-
-		/*
-		 * first record needs to say how long the original string was
-		 * so we can be sure nothing was lost.
-		 */
-		if ((i == 0) && (too_long))
-			audit_log_format(*ab, " a%d_len=%zu", arg_num,
-					 has_cntl ? 2*len : len);
-
-		/*
-		 * normally arguments are small enough to fit and we already
-		 * filled buf above when we checked for control characters
-		 * so don't bother with another copy_from_user
-		 */
-		if (len >= max_execve_audit_len)
-			ret = copy_from_user(buf, p, to_send);
-		else
-			ret = 0;
-		if (ret) {
-			WARN_ON(1);
-			send_sig(SIGKILL, current, 0);
-			return -1;
-		}
-		buf[to_send] = '\0';
-
-		/* actually log it */
-		audit_log_format(*ab, " a%d", arg_num);
-		if (too_long)
-			audit_log_format(*ab, "[%d]", i);
-		audit_log_format(*ab, "=");
-		if (has_cntl)
-			audit_log_n_hex(*ab, buf, to_send);
-		else
-			audit_log_string(*ab, buf);
-
-		p += to_send;
-		len_left -= to_send;
-		*len_sent += arg_num_len;
-		if (has_cntl)
-			*len_sent += to_send * 2;
-		else
-			*len_sent += to_send;
-	}
-	/* include the null we didn't log */
-	return len + 1;
-}
-
 static void audit_log_execve_info(struct audit_context *context,
 				  struct audit_buffer **ab)
 {
-	int i, len;
-	size_t len_sent = 0;
-	const char __user *p;
+	long len_max;
+	long len_rem;
+	long len_full;
+	long len_buf;
+	long len_abuf;
+	long len_tmp;
+	bool require_data;
+	bool encode;
+	unsigned int iter;
+	unsigned int arg;
+	char *buf_head;
 	char *buf;
-
-	p = (const char __user *)current->mm->arg_start;
-
+	const char __user *p = (const char __user *)current->mm->arg_start;
+
+	/* NOTE: this buffer needs to be large enough to hold all the non-arg
+	 *       data we put in the audit record for this argument (see the
+	 *       code below) ... at this point in time 96 is plenty */
+	char abuf[96];
+
+	/* NOTE: we set MAX_EXECVE_AUDIT_LEN to a rather arbitrary limit, the
+	 *       current value of 7500 is not as important as the fact that it
+	 *       is less than 8k, a setting of 7500 gives us plenty of wiggle
+	 *       room if we go over a little bit in the logging below */
+	WARN_ON_ONCE(MAX_EXECVE_AUDIT_LEN > 7500);
+	len_max = MAX_EXECVE_AUDIT_LEN;
+
+	/* scratch buffer to hold the userspace args */
+	buf_head = kmalloc(MAX_EXECVE_AUDIT_LEN + 1, GFP_KERNEL);
+	if (!buf_head) {
+		audit_panic("out of memory for argv string");
+		return;
+	}
+	buf = buf_head;
+	
 	audit_log_format(*ab, "argc=%d", context->execve.argc);
 
-	/*
-	 * we need some kernel buffer to hold the userspace args.  Just
-	 * allocate one big one rather than allocating one of the right size
-	 * for every single argument inside audit_log_single_execve_arg()
-	 * should be <8k allocation so should be pretty safe.
-	 */
-	buf = kmalloc(MAX_EXECVE_AUDIT_LEN + 1, GFP_KERNEL);
-	if (!buf) {
-		audit_panic("out of memory for argv string\n");
-		return;
-	}
-
-	for (i = 0; i < context->execve.argc; i++) {
-		len = audit_log_single_execve_arg(context, ab, i,
-						  &len_sent, p, buf);
-		if (len <= 0)
-			break;
-		p += len;
-	}
-	kfree(buf);
+	len_rem = len_max;
+	len_buf = 0;
+	len_full = 0;
+	require_data = true;
+	encode = false;
+	iter = 0;
+	arg = 0;
+	do {
+		/* NOTE: we don't ever want to trust this value for anything
+		 *       serious, but the audit record format insists we
+		 *       provide an argument length for really long arguments,
+		 *       e.g. > MAX_EXECVE_AUDIT_LEN, so we have no choice but
+		 *       to use strncpy_from_user() to obtain this value for
+		 *       recording in the log, although we don't use it
+		 *       anywhere here to avoid a double-fetch problem */
+		if (len_full == 0)
+			len_full = strnlen_user(p, MAX_ARG_STRLEN) - 1;
+
+		/* read more data from userspace */
+		if (require_data) {
+			/* can we make more room in the buffer? */
+			if (buf != buf_head) {
+				memmove(buf_head, buf, len_buf);
+				buf = buf_head;
+			}
+
+			/* fetch as much as we can of the argument */
+			len_tmp = strncpy_from_user(&buf_head[len_buf], p,
+						    len_max - len_buf);
+			if (len_tmp == -EFAULT) {
+				/* unable to copy from userspace */
+				send_sig(SIGKILL, current, 0);
+				goto out;
+			} else if (len_tmp == (len_max - len_buf)) {
+				/* buffer is not large enough */
+				require_data = true;
+				/* NOTE: if we are going to span multiple
+				 *       buffers force the encoding so we stand
+				 *       a chance at a sane len_full value and
+				 *       consistent record encoding */
+				encode = true;
+				len_full = len_full * 2;
+				p += len_tmp;
+			} else {
+				require_data = false;
+				if (!encode)
+					encode = audit_string_contains_control(
+								buf, len_tmp);
+				/* try to use a trusted value for len_full */
+				if (len_full < len_max)
+					len_full = (encode ?
+						    len_tmp * 2 : len_tmp);
+				p += len_tmp + 1;
+			}
+			len_buf += len_tmp;
+			buf_head[len_buf] = '\0';
+
+			/* length of the buffer in the audit record? */
+			len_abuf = (encode ? len_buf * 2 : len_buf + 2);
+		}
+		
+
+		/* write as much as we can to the audit log */
+		if (len_buf > 0) {
+			/* NOTE: some magic numbers here - basically if we
+			 *       can't fit a reasonable amount of data into the
+			 *       existing audit buffer, flush it and start with
+			 *       a new buffer */
+			if ((sizeof(abuf) + 8) > len_rem) {
+				len_rem = len_max;
+				audit_log_end(*ab);
+				*ab = audit_log_start(context,
+						      GFP_KERNEL, AUDIT_EXECVE);
+				if (!*ab)
+					goto out;
+			}
+
+		/* create the non-arg portion of the arg record */
+			len_tmp = 0;
+			if (require_data || (iter > 0) ||
+			    ((len_abuf + sizeof(abuf)) > len_rem)) {
+				if (iter == 0) {
+					len_tmp += snprintf(&abuf[len_tmp],
+							sizeof(abuf) - len_tmp,
+							" a%d_len=%lu",
+							arg, len_full);
+				}
+				len_tmp += snprintf(&abuf[len_tmp],
+						    sizeof(abuf) - len_tmp,
+						    " a%d[%d]=", arg, iter++);
+			} else
+				len_tmp += snprintf(&abuf[len_tmp],
+						    sizeof(abuf) - len_tmp,
+						    " a%d=", arg);
+			WARN_ON(len_tmp >= sizeof(abuf));
+			abuf[sizeof(abuf) - 1] = '\0';
+
+			/* log the arg in the audit record */
+			audit_log_format(*ab, "%s", abuf);
+			len_rem -= len_tmp;
+			len_tmp = len_buf;
+			if (encode) {
+				if (len_abuf > len_rem)
+					len_tmp = len_rem / 2; /* encoding */
+				audit_log_n_hex(*ab, buf, len_tmp);
+				len_rem -= len_tmp * 2;
+				len_abuf -= len_tmp * 2;
+			} else {
+				if (len_abuf > len_rem)
+					len_tmp = len_rem - 2; /* quotes */
+				audit_log_n_string(*ab, buf, len_tmp);
+				len_rem -= len_tmp + 2;
+				/* don't subtract the "2" because we still need
+				 * to add quotes to the remaining string */
+				len_abuf -= len_tmp;
+			}
+			len_buf -= len_tmp;
+			buf += len_tmp;
+		}
+
+		/* ready to move to the next argument? */
+		if ((len_buf == 0) && !require_data) {
+			arg++;
+			iter = 0;
+			len_full = 0;
+			require_data = true;
+			encode = false;
+		}
+	} while (arg < context->execve.argc);
+
+	/* NOTE: the caller handles the final audit_log_end() call */
+
+	out:
+	kfree(buf_head);
 }
 
 static void show_special(struct audit_context *context, int *call_panic)
