#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

//notes:
//need buffer of requests
//if scanner is going up, honors quests from low to high until reaches end and then
//switch directions
//if scanner is going down, honors requests from high to low until reaches end, and then
//switch directions


//holds queue
struct sstf_data {
	struct list_head queue;
}

//removes the queuelist that is next's head, and then reinitialize it, apparently?
static void sstf_merged_requests(struct request_queue *q, struct request *rq, struct request *next) {
	list_del_init(&next->queuelist);
}

//not sure exactly what it's doing
static int sstf_dispatch(struct request_queue *q, int force) {
	struct sstf_data *nd = q->elevator->elevator_data;
	
	if(!list_empty(&nd->queue)){
		struct request *rq;
		rq = list_entry(nd->queue.next, struct request, queuelist);
		list_del_init(&rq->queuelist);
		elv_dispatch_sort(q, rq);
		return 1;
	}
	return 0;
}

//adds request to the correct location of the queue
static void sstf_add_request(struct request_queue *q, struct request *rq) {
	struct sstf_data *nd = q->elevator->elevator_data;
	struct request *next_req, *prev_req;
	//list_add_tail(&rq->queuelist, &nd->queue);

	 if (list_empty(&nd->queue)) {
		printk("add_request to an empty list\n");

		// If empty list then just add the request
        list_add(&rq->queuelist, &nd->queue);
    }else{
    	printk("add_request to non empty list\n");
    	
    	// get the value for the next and prev
    	next_req = list_entry(nd->queue.next, struct request, queuelist);
        prev_req = list_entry(nd->queue.prev, struct request, queuelist);

        // go through the list until you find the right location
        while (blk_rq_pos(rq) < blk_rq_pos(next_req)) {
            next_req = list_entry(next_req->queuelist.next, struct request, queuelist);
            prev_req = list_entry(prev_req->queuelist.prev, struct request, queuelist);
        }

        // add the request to the list
        list_add(&rq->queuelist, &prev_rq->queuelist);

    }
}

//if the previous request in *rq is the new request? return null
static struct request * sstf_former_request(struct request_queue *q, struct request *rq) {
	struct sstf_data *nd = q->elevator->elevator_data;
	
	if(rq->queuelist.prev == &nd->queue) {
		return null;
	}
	return list_entry(rq->queuelist.prev, struct request, queuelist);
}

//reverse of the previous function?
static struct request * sstf_latter_request(struct request_queue *q, struct request *rq) {
	struct sstf_data *nd = q->elevator->elevator_data;
	if(rq->queuelist.next == &nd->queue) {
		return NULL;
	}
	return list_entry(rq->queuelist.next, struct request, queuelist);
}

//initializes the queue
static int sstf_init_queue(struct request_queue *q, struct elevator_type *e) {
	struct sstf_data *nd;
	struct elevator_queue *eq;
	
	eq = elevator_alloc(q, e);
	
	if(!eq)
		return -ENOMEM;
	
	nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
	
	if(!nd) {
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
	eq->elevator_data = nd;
	
	INIT_LIST_HEAD(&nd->queue);
	
	spin_lock_irq(q->queue_lock);	//locks memory from use to keep from having race conditions
	q->elevator = eq;
	spin_unlock_irq(q->queue->lock);
	return 0;
}

//replaces working/current queue with elevator queue?
static void sstf_exit_queue(struct elevator_queue *e) {
	struct sstf_data *nd = e->elevator_data;
	
	BUG_ON(!list_empty(&nd->queue));
	kfree(nd);
}

//some kind of struct that is binding stuff in the elevator to functions in here
static struct elevator_type elevator_sstf = {
	.ops = {
		.elevator_merge_req_fn		= sstf_merged_requests;
		.elevator_dispatch_fn		= sstf_dispatch,
		.elevator_add_req_fn		= sstf_add_request,
		.elevator_former_req_fn		= sstf_former_request,
		.elevator_latter_req_fn		= sstf_latter_request,
		.elevator_init_fn			= sstf_init_queue,
		.elevator_exit_fn			= sstf_exit_queue,
	},
	.elevator_name = "sstf",
	.elevator_owner = THIS_MODULE,
};

static int __init sstf_init(void) {
	elv_register(&elevator_sstf);
}

static void __exit sstf_exit(void) {
	elv_unregister(&elevator_sstf);
}

module_init(sstf_init);
module_exit(sstf_exit);

MODULE_AUTHOR("GinaPhipps_NawwafAlmutairi_BrandonThenell");
MODULE_LICENSE("");
MODULE_DESCRIPTION("sstf IO scheduler");