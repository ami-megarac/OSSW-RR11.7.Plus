--- linux.org/kernel/events/core.c	Thu Aug 14 09:38:34 2014
+++ linux/kernel/events/core.c	Tue Jan 23 17:13:28 2018
@@ -894,6 +894,78 @@
 	}
 }
 
+/*
+ * Because of perf_event::ctx migration in sys_perf_event_open::move_group and
+ * perf_pmu_migrate_context() we need some magic.
+ *
+ * Those places that change perf_event::ctx will hold both
+ * perf_event_ctx::mutex of the 'old' and 'new' ctx value.
+ *
+ * Lock ordering is by mutex address. There is one other site where
+ * perf_event_context::mutex nests and that is put_event(). But remember that
+ * that is a parent<->child context relation, and migration does not affect
+ * children, therefore these two orderings should not interact.
+ *
+ * The change in perf_event::ctx does not affect children (as claimed above)
+ * because the sys_perf_event_open() case will install a new event and break
+ * the ctx parent<->child relation, and perf_pmu_migrate_context() is only
+ * concerned with cpuctx and that doesn't have children.
+ *
+ * The places that change perf_event::ctx will issue:
+ *
+ *   perf_remove_from_context();
+ *   synchronize_rcu();
+ *   perf_install_in_context();
+ *
+ * to affect the change. The remove_from_context() + synchronize_rcu() should
+ * quiesce the event, after which we can install it in the new location. This
+ * means that only external vectors (perf_fops, prctl) can perturb the event
+ * while in transit. Therefore all such accessors should also acquire
+ * perf_event_context::mutex to serialize against this.
+ *
+ * However; because event->ctx can change while we're waiting to acquire
+ * ctx->mutex we must be careful and use the below perf_event_ctx_lock()
+ * function.
+ *
+ * Lock order:
+ *	task_struct::perf_event_mutex
+ *	  perf_event_context::mutex
+ *	    perf_event_context::lock
+ *	    perf_event::child_mutex;
+ *	    perf_event::mmap_mutex
+ *	    mmap_sem
+*/
+static struct perf_event_context *perf_event_ctx_lock(struct perf_event *event)
+{
+	struct perf_event_context *ctx;
+
+again:
+	rcu_read_lock();
+	ctx = ACCESS_ONCE(event->ctx);
+	if (!atomic_inc_not_zero(&ctx->refcount)) {
+		rcu_read_unlock();
+		goto again;
+	}
+	rcu_read_unlock();
+
+	mutex_lock(&ctx->mutex);
+	if (event->ctx != ctx) {
+		mutex_unlock(&ctx->mutex);
+		put_ctx(ctx);
+		goto again;
+	}
+
+	return ctx;
+}
+
+static void perf_event_ctx_unlock(struct perf_event *event,
+				  struct perf_event_context *ctx)
+{
+	mutex_unlock(&ctx->mutex);
+	put_ctx(ctx);
+}
+
+
 static void unclone_ctx(struct perf_event_context *ctx)
 {
 	if (ctx->parent_ctx) {
@@ -1583,7 +1655,7 @@
  * is the current context on this CPU and preemption is disabled,
  * hence we can't get into perf_event_task_sched_out for this context.
  */
-void perf_event_disable(struct perf_event *event)
+static void _perf_event_disable(struct perf_event *event)
 {
 	struct perf_event_context *ctx = event->ctx;
 	struct task_struct *task = ctx->task;
@@ -1623,6 +1695,19 @@
 		event->state = PERF_EVENT_STATE_OFF;
 	}
 	raw_spin_unlock_irq(&ctx->lock);
+}
+
+/*
+ * Strictly speaking kernel users cannot create groups and therefore this
+ * interface does not need the perf_event_ctx_lock() magic.
+ */
+void perf_event_disable(struct perf_event *event)
+{
+	struct perf_event_context *ctx;
+
+	ctx = perf_event_ctx_lock(event);
+	_perf_event_disable(event);
+	perf_event_ctx_unlock(event, ctx);
 }
 EXPORT_SYMBOL_GPL(perf_event_disable);
 
@@ -2077,7 +2162,7 @@
  * perf_event_for_each_child or perf_event_for_each as described
  * for perf_event_disable.
  */
-void perf_event_enable(struct perf_event *event)
+static void _perf_event_enable(struct perf_event *event)
 {
 	struct perf_event_context *ctx = event->ctx;
 	struct task_struct *task = ctx->task;
@@ -2133,9 +2218,21 @@
 out:
 	raw_spin_unlock_irq(&ctx->lock);
 }
+
+/*
+ * See perf_event_disable();
+ */
+void perf_event_enable(struct perf_event *event)
+{
+	struct perf_event_context *ctx;
+
+	ctx = perf_event_ctx_lock(event);
+	_perf_event_enable(event);
+	perf_event_ctx_unlock(event, ctx);
+}
 EXPORT_SYMBOL_GPL(perf_event_enable);
 
-int perf_event_refresh(struct perf_event *event, int refresh)
+static int _perf_event_refresh(struct perf_event *event, int refresh)
 {
 	/*
 	 * not supported on inherited events
@@ -2144,9 +2241,24 @@
 		return -EINVAL;
 
 	atomic_add(refresh, &event->event_limit);
-	perf_event_enable(event);
+	_perf_event_enable(event);
 
 	return 0;
+}
+
+/*
+ * See perf_event_disable()
+ */
+int perf_event_refresh(struct perf_event *event, int refresh)
+{
+	struct perf_event_context *ctx;
+	int ret;
+
+	ctx = perf_event_ctx_lock(event);
+	ret = _perf_event_refresh(event, refresh);
+	perf_event_ctx_unlock(event, ctx);
+
+	return ret;
 }
 EXPORT_SYMBOL_GPL(perf_event_refresh);
 
@@ -3332,7 +3444,16 @@
 	rcu_read_unlock();
 
 	if (owner) {
-		mutex_lock(&owner->perf_event_mutex);
+		/*
+		 * If we're here through perf_event_exit_task() we're already
+		 * holding ctx->mutex which would be an inversion wrt. the
+		 * normal lock order.
+		 *
+		 * However we can safely take this lock because its the child
+		 * ctx->mutex.
+		 */
+		mutex_lock_nested(&owner->perf_event_mutex, SINGLE_DEPTH_NESTING);
+
 		/*
 		 * We have to re-check the event->owner field, if it is cleared
 		 * we raced with perf_event_exit_task(), acquiring the mutex
@@ -3384,12 +3505,13 @@
 				   u64 read_format, char __user *buf)
 {
 	struct perf_event *leader = event->group_leader, *sub;
-	int n = 0, size = 0, ret = -EFAULT;
 	struct perf_event_context *ctx = leader->ctx;
+	int n = 0, size = 0, ret;
+	u64 count, enabled, running;
 	u64 values[5];
-	u64 count, enabled, running;
-
-	mutex_lock(&ctx->mutex);
+
+	lockdep_assert_held(&ctx->mutex);
+
 	count = perf_event_read_value(leader, &enabled, &running);
 
 	values[n++] = 1 + leader->nr_siblings;
@@ -3404,7 +3526,7 @@
 	size = n * sizeof(u64);
 
 	if (copy_to_user(buf, values, size))
-		goto unlock;
+		return -EFAULT;
 
 	ret = size;
 
@@ -3418,14 +3540,11 @@
 		size = n * sizeof(u64);
 
 		if (copy_to_user(buf + ret, values, size)) {
-			ret = -EFAULT;
-			goto unlock;
+			return -EFAULT;
 		}
 
 		ret += size;
 	}
-unlock:
-	mutex_unlock(&ctx->mutex);
 
 	return ret;
 }
@@ -3484,8 +3603,14 @@
 perf_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
 {
 	struct perf_event *event = file->private_data;
-
-	return perf_read_hw(event, buf, count);
+	struct perf_event_context *ctx;
+	int ret;
+
+	ctx = perf_event_ctx_lock(event);
+	ret = perf_read_hw(event, buf, count);
+	perf_event_ctx_unlock(event, ctx);
+
+	return ret;
 }
 
 static unsigned int perf_poll(struct file *file, poll_table *wait)
@@ -3509,7 +3634,7 @@
 	return events;
 }
 
-static void perf_event_reset(struct perf_event *event)
+static void _perf_event_reset(struct perf_event *event)
 {
 	(void)perf_event_read(event);
 	local64_set(&event->count, 0);
@@ -3541,14 +3666,13 @@
 	struct perf_event_context *ctx = event->ctx;
 	struct perf_event *sibling;
 
-	WARN_ON_ONCE(ctx->parent_ctx);
-	mutex_lock(&ctx->mutex);
+	lockdep_assert_held(&ctx->mutex);
+
 	event = event->group_leader;
 
 	perf_event_for_each_child(event, func);
 	list_for_each_entry(sibling, &event->sibling_list, group_entry)
 		perf_event_for_each_child(sibling, func);
-	mutex_unlock(&ctx->mutex);
 }
 
 static int perf_event_period(struct perf_event *event, u64 __user *arg)
@@ -3618,25 +3742,24 @@
 				 struct perf_event *output_event);
 static int perf_event_set_filter(struct perf_event *event, void __user *arg);
 
-static long perf_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
-{
-	struct perf_event *event = file->private_data;
+static long _perf_ioctl(struct perf_event *event, unsigned int cmd, unsigned long arg)
+{
 	void (*func)(struct perf_event *);
 	u32 flags = arg;
 
 	switch (cmd) {
 	case PERF_EVENT_IOC_ENABLE:
-		func = perf_event_enable;
+		func = _perf_event_enable;
 		break;
 	case PERF_EVENT_IOC_DISABLE:
-		func = perf_event_disable;
+		func = _perf_event_disable;
 		break;
 	case PERF_EVENT_IOC_RESET:
-		func = perf_event_reset;
+		func = _perf_event_reset;
 		break;
 
 	case PERF_EVENT_IOC_REFRESH:
-		return perf_event_refresh(event, arg);
+		return _perf_event_refresh(event, arg);
 
 	case PERF_EVENT_IOC_PERIOD:
 		return perf_event_period(event, (u64 __user *)arg);
@@ -3683,13 +3806,30 @@
 	return 0;
 }
 
+static long perf_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
+{
+	struct perf_event *event = file->private_data;
+	struct perf_event_context *ctx;
+	long ret;
+
+	ctx = perf_event_ctx_lock(event);
+	ret = _perf_ioctl(event, cmd, arg);
+	perf_event_ctx_unlock(event, ctx);
+
+	return ret;
+}
+
 int perf_event_task_enable(void)
 {
+	struct perf_event_context *ctx;
 	struct perf_event *event;
 
 	mutex_lock(&current->perf_event_mutex);
-	list_for_each_entry(event, &current->perf_event_list, owner_entry)
-		perf_event_for_each_child(event, perf_event_enable);
+	list_for_each_entry(event, &current->perf_event_list, owner_entry) {
+		ctx = perf_event_ctx_lock(event);
+		perf_event_for_each_child(event, _perf_event_enable);
+		perf_event_ctx_unlock(event, ctx);
+	}
 	mutex_unlock(&current->perf_event_mutex);
 
 	return 0;
@@ -3697,11 +3837,15 @@
 
 int perf_event_task_disable(void)
 {
+	struct perf_event_context *ctx;
 	struct perf_event *event;
 
 	mutex_lock(&current->perf_event_mutex);
-	list_for_each_entry(event, &current->perf_event_list, owner_entry)
-		perf_event_for_each_child(event, perf_event_disable);
+	list_for_each_entry(event, &current->perf_event_list, owner_entry) {
+		ctx = perf_event_ctx_lock(event);
+		perf_event_for_each_child(event, _perf_event_disable);
+		perf_event_ctx_unlock(event, ctx);
+	}
 	mutex_unlock(&current->perf_event_mutex);
 
 	return 0;
@@ -6995,6 +7139,15 @@
 	return ret;
 }
 
+static void mutex_lock_double(struct mutex *a, struct mutex *b)
+{
+	if (b < a)
+		swap(a, b);
+
+	mutex_lock(a);
+	mutex_lock_nested(b, SINGLE_DEPTH_NESTING);
+}
+
 /**
  * sys_perf_event_open - open a performance event, associate it to a task/cpu
  *
@@ -7010,7 +7163,7 @@
 	struct perf_event *group_leader = NULL, *output_event = NULL;
 	struct perf_event *event, *sibling;
 	struct perf_event_attr attr;
-	struct perf_event_context *ctx;
+	struct perf_event_context *ctx, *uninitialized_var(gctx);
 	struct file *event_file = NULL;
 	struct fd group = {NULL, 0};
 	struct task_struct *task = NULL;
@@ -7183,9 +7336,14 @@
 	}
 
 	if (move_group) {
-		struct perf_event_context *gctx = group_leader->ctx;
-
-		mutex_lock(&gctx->mutex);
+		gctx = group_leader->ctx;
+
+		/*
+		 * See perf_event_ctx_lock() for comments on the details
+		 * of swizzling perf_event::ctx.
+		 */
+		mutex_lock_double(&gctx->mutex, &ctx->mutex);
+
 		perf_remove_from_context(group_leader, false);
 
 		/*
@@ -7200,14 +7358,17 @@
 			perf_event__state_init(sibling);
 			put_ctx(gctx);
 		}
-		mutex_unlock(&gctx->mutex);
-		put_ctx(gctx);
+	} else {
+		mutex_lock(&ctx->mutex);
 	}
 
 	WARN_ON_ONCE(ctx->parent_ctx);
-	mutex_lock(&ctx->mutex);
 
 	if (move_group) {
+		/*
+		 * Wait for everybody to stop referencing the events through
+		 * the old lists, before installing it on new lists.
+		 */
 		synchronize_rcu();
 		perf_install_in_context(ctx, group_leader, event->cpu);
 		get_ctx(ctx);
@@ -7220,6 +7381,11 @@
 
 	perf_install_in_context(ctx, event, event->cpu);
 	perf_unpin_context(ctx);
+
+	if (move_group) {
+		mutex_unlock(&gctx->mutex);
+		put_ctx(gctx);
+	}
 	mutex_unlock(&ctx->mutex);
 
 	put_online_cpus();
@@ -7323,7 +7489,11 @@
 	src_ctx = &per_cpu_ptr(pmu->pmu_cpu_context, src_cpu)->ctx;
 	dst_ctx = &per_cpu_ptr(pmu->pmu_cpu_context, dst_cpu)->ctx;
 
-	mutex_lock(&src_ctx->mutex);
+	/*
+	 * See perf_event_ctx_lock() for comments on the details
+	 * of swizzling perf_event::ctx.
+	 */
+	mutex_lock_double(&src_ctx->mutex, &dst_ctx->mutex);
 	list_for_each_entry_safe(event, tmp, &src_ctx->event_list,
 				 event_entry) {
 		perf_remove_from_context(event, false);
@@ -7331,11 +7501,9 @@
 		put_ctx(src_ctx);
 		list_add(&event->migrate_entry, &events);
 	}
-	mutex_unlock(&src_ctx->mutex);
 
 	synchronize_rcu();
 
-	mutex_lock(&dst_ctx->mutex);
 	list_for_each_entry_safe(event, tmp, &events, migrate_entry) {
 		list_del(&event->migrate_entry);
 		if (event->state >= PERF_EVENT_STATE_OFF)
@@ -7345,6 +7513,7 @@
 		get_ctx(dst_ctx);
 	}
 	mutex_unlock(&dst_ctx->mutex);
+	mutex_unlock(&src_ctx->mutex);
 }
 EXPORT_SYMBOL_GPL(perf_pmu_migrate_context);
 
