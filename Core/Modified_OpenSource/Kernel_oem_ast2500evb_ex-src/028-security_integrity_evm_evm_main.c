--- linux.org/security/integrity/evm/evm_main.c	Thu Aug 14 09:38:34 2014
+++ linux/security/integrity/evm/evm_main.c	Wed Jan 17 11:24:12 2018
@@ -21,6 +21,7 @@
 #include <linux/integrity.h>
 #include <linux/evm.h>
 #include <crypto/hash.h>
+#include <crypto/algapi.h>
 #include "evm.h"
 
 int evm_initialized;
@@ -132,7 +133,7 @@
 				   xattr_value_len, calc.digest);
 		if (rc)
 			break;
-		rc = memcmp(xattr_data->digest, calc.digest,
+		rc = cryto_memneq(xattr_data->digest, calc.digest,
 			    sizeof(calc.digest));
 		if (rc)
 			rc = -EINVAL;
