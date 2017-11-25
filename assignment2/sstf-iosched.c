/*
* elevator C-LOOK
*/
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

#define get_sector(X) blk_rq_pos(list_entry(X, struct request, queuelist))

struct sstf_data {
        struct list_head queue;
        sector_t head_loc;
};

static void sstf_merged_requests(struct request_queue *q,
                                 struct request *rq,
                                 struct request *next)
{
        list_del_init(&next->queuelist);
}

static int sstf_dispatch(struct request_queue *q, int force)
{
        struct sstf_data *nd = q->elevator->elevator_data;

        printk("[SSTF] DISPATCHING REQUEST...\n");

        if (!list_empty(&(nd->queue))) {
                struct request *rq;
                rq = list_entry(nd->queue.next, struct request, queuelist);
                list_del_init(&(rq->queuelist));
                elv_dispatch_sort(q, rq);
                nd->head_loc = rq_end_sector(rq);

                printk("[SSTF] DISPATCHED REQUEST:\n");
                if (rq_data_dir(rq) == READ) {
                        printk("[SSTF]     TYPE:           READ\n");
                }
                else {
                        printk("[SSTF]     TYPE:           WRITE\n");
                }
                printk("[SSTF]     HEAD POS START: %lu\n", blk_rq_pos(rq));
                printk("[SSTF]     HEAD POS END:   %lu\n", rq_end_sector(rq));
                
                return 1;
        }

        printk("[SSTF] CANNOT DISPATCH, QUEUE EMPTY.\n");
        return 0;
}

static void sstf_add_request(struct request_queue *q, struct request *rq)
{
        struct sstf_data *nd = q->elevator->elevator_data;
        struct list_head *cur = &(nd->queue);

        printk("[SSTF] ADDING REQUEST...\n");

        if (!list_empty(&nd->queue)) {
                struct list_head *next = cur->next;
                sector_t cur_sect  = nd->head_loc;
                sector_t next_sect = get_sector(next);
                sector_t rq_sect   = blk_rq_pos(rq);
                if ((cur->next == cur->prev) && (cur_sect == next_sect)) {
                        cur = cur->next;
                } else if (rq_sect < nd->head_loc) {
                        while (rq_sect < cur_sect) {
                                if ((cur_sect > next_sect) &&
                                    (rq_sect < next_sect)) {
                                        break;
                                }
                                next = cur;
                                next_sect = cur_sect;
                                cur = cur->prev;
                                cur_sect = (cur == &(nd->queue)) ?
                                           nd->head_loc          :
                                           get_sector(cur);
                        }
                } else {
                        while (next_sect < rq_sect) {
                                if ((cur_sect > next_sect) && 
                                    (rq_sect > cur_sect)) {
                                        break;
                                }
                                cur = next;
                                cur_sect = next_sect;
                                next = next->next;
                                next_sect = (next == &(nd->queue)) ?
                                        nd->head_loc               :
                                        get_sector(next);
                        }
                }
                
        }
        list_add(&(rq->queuelist), cur);

        printk("[SSTF] ADDED REQUEST:\n");
        if (rq_data_dir(rq) == READ) {
                printk("[SSTF]     TYPE:           READ\n");
        }
        else {
                printk("[SSTF]     TYPE:           WRITE\n");
        }
        printk("[SSTF]     REQUEST LOC:    %lu\n", blk_rq_pos(rq));
        printk("[SSTF]     NUM SECTORS:    %lu\n", blk_rq_sectors(rq));
}

static struct request *
sstf_former_request(struct request_queue *q, struct request *rq)
{
        struct sstf_data *nd = q->elevator->elevator_data;

        if (list_empty(&nd->queue))
                return NULL;
        return list_entry(nd->queue.prev, struct request, queuelist);
}

static struct request *
sstf_latter_request(struct request_queue *q, struct request *rq)
{
        struct sstf_data *nd = q->elevator->elevator_data;

        if (list_empty(&nd->queue))
                return NULL;
        return list_entry(nd->queue.next, struct request, queuelist);
}

static int sstf_init_queue(struct request_queue *q, struct elevator_type *e)
{
        struct sstf_data *nd;
        struct elevator_queue *eq;

        printk("[SSTF] INITIALIZING...\n");
        
        eq = elevator_alloc(q, e);
        if (!eq)
                return -ENOMEM;

        nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
        if (!nd) {
                kobject_put(&eq->kobj);
                return -ENOMEM;
        }
        eq->elevator_data = nd;

        INIT_LIST_HEAD(&nd->queue);
        nd->head_loc = 0;

        spin_lock_irq(q->queue_lock);
        q->elevator = eq;
        spin_unlock_irq(q->queue_lock);

        printk("[SSTF] INITIALIZED\n");
        return 0;
}

static void sstf_exit_queue(struct elevator_queue *e)
{
        struct sstf_data *nd = e->elevator_data;

        printk("[SSTF] EXITING...\n");

        BUG_ON(!list_empty(&nd->queue));
        kfree(nd);

        printk("[SSTF] EXITED\n");
}

static struct elevator_type elevator_sstf = {
        .ops = {
                .elevator_merge_req_fn  = sstf_merged_requests,
                .elevator_dispatch_fn   = sstf_dispatch,
                .elevator_add_req_fn    = sstf_add_request,
                .elevator_former_req_fn = sstf_former_request,
                .elevator_latter_req_fn = sstf_latter_request,
                .elevator_init_fn       = sstf_init_queue,
                .elevator_exit_fn       = sstf_exit_queue,
         },
        .elevator_name = "sstf",
        .elevator_owner = THIS_MODULE,
};

static int __init sstf_init(void)
{
        return elv_register(&elevator_sstf);
}

static void __exit sstf_exit(void)
{
        elv_unregister(&elevator_sstf);
}

module_init(sstf_init);
module_exit(sstf_exit);

MODULE_AUTHOR("Morgan Patch / Mark Bereza");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SSTF IO scheduler");
