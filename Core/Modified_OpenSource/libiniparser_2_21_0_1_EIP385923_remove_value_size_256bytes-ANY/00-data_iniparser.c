--- .pristine/libiniparser-2.21.0-src/data/iniparser.c	Wed Jan 31 16:57:21 2018
+++ source/libiniparser-2.21.0-src/data/iniparser.c	Wed Jan 31 17:16:06 2018
@@ -283,9 +283,7 @@
         if(!handler || !(handler->hashTable) || (!hashstr) || (hashstr[0] == '\0')) {
                         return;
         }
-        if ((val) && ((strnlen(val, MAX_CONF_STR_LENGTH) >= MAX_CONF_STR_LENGTH))) {
-            return;
-        }
+
         hashTable = handler->hashTable;
         hashval = hasher31(hashstr);
     if(hashTable[hashval] == NULL) {
