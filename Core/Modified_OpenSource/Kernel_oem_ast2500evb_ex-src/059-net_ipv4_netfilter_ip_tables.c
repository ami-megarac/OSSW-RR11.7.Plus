--- linux.org/net/ipv4/netfilter/ip_tables.c	Thu Aug 14 09:38:34 2014
+++ linux/net/ipv4/netfilter/ip_tables.c	Mon Jan 22 15:50:13 2018
@@ -515,6 +515,10 @@
 
 				/* Move along one */
 				size = e->next_offset;
+				
+				if (pos + size > newinfo->size - sizeof(*e))
+					return 0;
+				
 				e = (struct ipt_entry *)
 					(entry0 + pos + size);
 				e->counters.pcnt = pos;
@@ -525,19 +529,14 @@
 				if (strcmp(t->target.u.user.name,
 					   XT_STANDARD_TARGET) == 0 &&
 				    newpos >= 0) {
-					if (newpos > newinfo->size -
-						sizeof(struct ipt_entry)) {
-						duprintf("mark_source_chains: "
-							"bad verdict (%i)\n",
-								newpos);
-						return 0;
-					}
 					/* This a jump; chase it. */
 					duprintf("Jump rule %u -> %u\n",
 						 pos, newpos);
 				} else {
 					/* ... this is a fallthru */
 					newpos = pos + e->next_offset;
+					if (newpos > newinfo->size - sizeof(*e))
+						return 0;
 				}
 				e = (struct ipt_entry *)
 					(entry0 + newpos);
