/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2015-2018 Jason A. Donenfeld <Jason@zx2c4.com>. All Rights Reserved.
 */

#include "queueing.h"

struct multicore_worker __percpu *packet_alloc_percpu_multicore_worker(work_func_t function, void *ptr)
{
	int cpu;
	struct multicore_worker __percpu *worker = alloc_percpu(struct multicore_worker);

	if (!worker)
		return NULL;

	for_each_possible_cpu(cpu) {
		per_cpu_ptr(worker, cpu)->ptr = ptr;
		INIT_WORK(&per_cpu_ptr(worker, cpu)->work, function);
	}
	return worker;
}

int packet_queue_init(struct crypt_queue *queue, work_func_t function, bool multicore, unsigned int len)
{
	/*int ret;*/

	memset(queue, 0, sizeof(*queue));
	/*ret = ptr_ring_init(&queue->ring, len, GFP_KERNEL);*/
	/*if (ret)*/
		/*return ret;*/
	ck_ring_init(&queue->ring, len);
	queue->ring_buffer.value = kcalloc(len, sizeof(void *), GFP_KERNEL);
	if (multicore) {
		queue->worker = packet_alloc_percpu_multicore_worker(function, queue);
		if (!queue->worker)
			return -ENOMEM;
	} else
		INIT_WORK(&queue->work, function);
	return 0;
}

void packet_queue_free(struct crypt_queue *queue, bool multicore)
{
	if (multicore)
		free_percpu(queue->worker);

	/* TODO: from the ck docs: It is possible for the function to return
	 * false even if ring is non-empty. See also
	 * http://concurrencykit.org/doc/ck_ring_trydequeue_spmc.html */
	/*WARN_ON(!ptr_ring_empty_bh(&queue->ring));*/
	/*ptr_ring_cleanup(&queue->ring, NULL);*/
	kfree(queue->ring_buffer.value);
}
