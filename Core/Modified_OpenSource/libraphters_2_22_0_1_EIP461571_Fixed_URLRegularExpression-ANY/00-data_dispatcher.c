--- .pristine/libraphters-2.22.0-src/data/dispatcher.c	Mon Mar 11 10:00:32 2019
+++ source/libraphters-2.22.0-src/data/dispatcher.c	Mon Mar 11 10:18:47 2019
@@ -89,13 +89,38 @@
 }
 
 void init_handlers() {
-    handler *cur = head;
-    while (cur != NULL) {
-        if (regcomp(&cur->regex, cur->regex_str, REG_EXTENDED) != 0) {
-            FAIL_WITH_ERROR("could not compile regex");
-        }
-        cur = cur->next;
-    }
+	handler *cur = head;
+	while (cur != NULL) {
+		/* We do change regular expression string here
+		 * allocate memory for original string and append with terminator(1), "$"(1) 
+		 */
+		char *changed_reg = NULL;
+		size_t len = strlen(cur->regex_str)+2;
+		changed_reg = (char *) malloc(len);
+		if(!(changed_reg)) {
+			FAIL_WITH_ERROR("Failed to allocate memory");
+		}else{
+			if(snprintf(changed_reg, len, "%s", cur->regex_str) >= (signed)(len))
+			{
+				if(changed_reg){
+					free(changed_reg);
+				}
+				FAIL_WITH_ERROR("Buffer overflow");
+			}else{
+				strncat(changed_reg, "$", 1); 
+				if (regcomp(&cur->regex, changed_reg, REG_EXTENDED) != 0) {
+					if(changed_reg){
+						free(changed_reg);
+					}
+					FAIL_WITH_ERROR("could not compile regex");
+				}
+			}
+		}
+		if(changed_reg){
+			free(changed_reg);
+		}
+		cur = cur->next;
+	}
 }
 
 void cleanup_handlers() {
