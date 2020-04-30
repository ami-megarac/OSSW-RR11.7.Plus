--- .pristine/fb_hw-1.10.0-ARM-AST-src/data/ast_fb.c	Thu Oct  5 19:27:17 2017
+++ source/fb_hw-1.10.0-ARM-AST-src/data/ast_fb.c	Fri Oct  6 14:41:17 2017
@@ -667,18 +667,64 @@
 	return 0;
 }
 
+static void astfb_free_bigphysarea(struct astfb_info *dinfo)
+{
+	unsigned long addr;
+	unsigned long size;
+
+	if (dinfo == NULL) {
+		printk(KERN_ERR "NULL POINTER\n");
+		return ;
+	}
+
+	if (dinfo->frame_buf == NULL) {
+		printk(KERN_ERR "NULL POINTER\n");
+		return ;
+	}
+	
+printk(KERN_INFO "big phys free\n");
+	// printk (KERN_INFO "Releasing FRAME BUFFER Big Physical memory starting :0x%lx\n", dinfo->frame_buf);
+	
+	addr = (unsigned long) dinfo->frame_buf;
+	size = dinfo->frame_buf_sz;
+	while (size > 0) {
+		ClearPageReserved(virt_to_page(addr));
+		addr += PAGE_SIZE;
+		size -= PAGE_SIZE;
+	}
+	bigphysarea_free_pages(dinfo->frame_buf);
+	dinfo->frame_buf = NULL;
+	dinfo->frame_buf_sz = 0;
+
+}
+
 static void cleanup(struct astfb_info *dinfo)
 {
 	if (!dinfo)
 		return;
-
+printk(KERN_INFO "cleanup\n");
 	#if Enable2D
 	ast_fb_disable_2d_engine(dinfo);
 	#endif
 
 	if (dinfo->frame_buf != NULL)
-		iounmap(dinfo->frame_buf);
-
+		astfb_free_bigphysarea(dinfo);
+
+	if (dinfo->io)
+	{
+		release_mem_region(AST_GRAPHICS_REG_BASE, AST_GRAPHICS_REG_SZ);
+		iounmap(dinfo->io);
+		dinfo->io = NULL;
+	}
+
+	if(dinfo->io_2d)
+	{
+		release_mem_region(AST_2D_ENGINE_REG_BASE, AST_2D_ENGINE_REG_SZ);
+		iounmap(dinfo->io_2d);
+		dinfo->io_2d = NULL;
+	}
+
+	fb_dealloc_cmap(&(dinfo->info->cmap));
 	if (dinfo->registered) {
 		unregister_framebuffer(dinfo->info);
 		framebuffer_release(dinfo->info);
