--- linux.org/fs/exec.c	2014-08-14 09:38:34.000000000 +0800
+++ linux/fs/exec.c	2018-01-22 16:20:09.376260999 +0800
@@ -176,6 +176,28 @@
 	add_mm_counter(mm, MM_ANONPAGES, diff);
 }
 
+typedef __u8  __attribute__((__may_alias__))  __u8_alias_t;
+typedef __u16 __attribute__((__may_alias__)) __u16_alias_t;
+typedef __u32 __attribute__((__may_alias__)) __u32_alias_t;
+typedef __u64 __attribute__((__may_alias__)) __u64_alias_t;
+
+static __always_inline void __read_once_size(const volatile void *p, void *res, int size)
+{
+        switch (size) {
+        case 1: *(__u8_alias_t  *) res = *(volatile __u8_alias_t  *) p; break;
+        case 2: *(__u16_alias_t *) res = *(volatile __u16_alias_t *) p; break;
+        case 4: *(__u32_alias_t *) res = *(volatile __u32_alias_t *) p; break;
+        case 8: *(__u64_alias_t *) res = *(volatile __u64_alias_t *) p; break;
+        default:
+                barrier();
+                __builtin_memcpy((void *)res, (const void *)p, size);
+                barrier();
+        }
+}
+
+#define READ_ONCE(x) \
+        ({ typeof(x) __val; __read_once_size(&x, &__val, sizeof(__val)); __val; })
+
 static struct page *get_arg_page(struct linux_binprm *bprm, unsigned long pos,
 		int write)
 {
@@ -196,8 +218,26 @@
 
 	if (write) {
 		unsigned long size = bprm->vma->vm_end - bprm->vma->vm_start;
+		unsigned long ptr_size;
 		struct rlimit *rlim;
 
+		/*
+		 * Since the stack will hold pointers to the strings, we
+		 * must account for them as well.
+		 *
+		 * The size calculation is the entire vma while each arg page is
+		 * built, so each time we get here it's calculating how far it
+		 * is currently (rather than each call being just the newly
+		 * added size from the arg page).  As a result, we need to
+		 * always add the entire size of the pointers, so that on the
+		 * last call to get_arg_page() we'll actually have the entire
+		 * correct size.
+		 */
+		ptr_size = (bprm->argc + bprm->envc) * sizeof(void *);
+		if (ptr_size > ULONG_MAX - size)
+			goto fail;
+		size += ptr_size;
+		
 		acct_arg_size(bprm, size / PAGE_SIZE);
 
 		/*
@@ -215,13 +255,14 @@
 		 *    to work from.
 		 */
 		rlim = current->signal->rlim;
-		if (size > ACCESS_ONCE(rlim[RLIMIT_STACK].rlim_cur) / 4) {
-			put_page(page);
-			return NULL;
-		}
+		if (size > READ_ONCE(rlim[RLIMIT_STACK].rlim_cur) / 4)
+			goto fail;
 	}
 
 	return page;
+fail:
+	put_page(page);
+	return NULL;
 }
 
 static void put_arg_page(struct page *page)
