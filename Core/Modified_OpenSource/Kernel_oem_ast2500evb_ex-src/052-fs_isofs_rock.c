--- linux.org/fs/isofs/rock.c	Mon Jan 22 14:07:01 2018
+++ linux/fs/isofs/rock.c	Mon Jan 22 15:37:09 2018
@@ -197,6 +197,8 @@
 	int retnamlen = 0;
 	int truncate = 0;
 	int ret = 0;
+	char *p;
+	int len;
 
 	if (!ISOFS_SB(inode->i_sb)->s_rock)
 		return 0;
@@ -261,12 +263,17 @@
 					rr->u.NM.flags);
 				break;
 			}
-			if ((strlen(retname) + rr->len - 5) >= 254) {
+			len = rr->len - 5;
+			if (retnamlen + len >= 254) {
 				truncate = 1;
 				break;
 			}
-			strncat(retname, rr->u.NM.name, rr->len - 5);
-			retnamlen += rr->len - 5;
+			p = memchr(rr->u.NM.name, '\0', len);
+			if (unlikely(p))
+				len = p - rr->u.NM.name;
+			memcpy(retname + retnamlen, rr->u.NM.name, len);
+			retnamlen += len;
+			retname[retnamlen] = '\0';
 			break;
 		case SIG('R', 'E'):
 			kfree(rs.buffer);
