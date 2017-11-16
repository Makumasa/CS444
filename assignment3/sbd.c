/*
* A sample, extra-simple block driver.
*
* Copyright 2003 Eklektix, Inc.  Redistributable under the terms
* of the GNU GPL.
*/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>

#include <linux/kernel.h> /* printk() */
#include <linux/fs.h>     /* everything... */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>

#include <linux/crypto.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/random.h>

#include <linux/mm.h>
#include <linux/sysinfo.h>

#define SBD_ENCRYPTED 0
#define SBD_PLAINTEXT 1

MODULE_LICENSE("Dual BSD/GPL");
static char *Version = "1.3";

static int major_num = 0;
static int logical_block_size = 512;
static int nsectors = 1024;  /* How big the drive is */
static u8 default_key[] = {0x26, 0xe7, 0x0a, 0x0c, 0xbc, 0xce, 0x4a, 0x5c, 0x0b,
        0x0b, 0xd1, 0xd8, 0xf7, 0xce, 0x2d, 0xf7};
static const unsigned int default_keylen = 16;
static u8 encrypt_key[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static unsigned int ekeylen = 0;
module_param_array(encrypt_key, byte, &ekeylen, 0);
static u8 decrypt_key[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static unsigned int dkeylen = 0;
module_param_array(decrypt_key, byte, &dkeylen, 0);


/*
* We can tweak our hardware sector size, but the kernel talks to us
* in terms of small sectors, always.
*/
#define KERNEL_SECTOR_SIZE 512

/*
* Our request queue.
*/
static struct request_queue *Queue;

/*
* The internal representation of our device.
*/
static struct sbd_device {
        unsigned long size;
        spinlock_t lock;
        u8 *data;
        u8 *written;
        struct gendisk *gd;
        struct crypto_cipher *encrypt;
        struct crypto_cipher *decrypt;
} Device;

static void sbd_print_buffer(u8 *buffer,
                             unsigned long len,
                             unsigned long offset,
                             int type)
{
        int i;
        for (i = 0; i < len; ++i) {
                if ((i % 16) == 0) {
                        if (type == SBD_PLAINTEXT)
                                printk("\nPLAINTEXT, BYTES %06lu-%06lu: ",
                                       i + offset, i + offset + 15);
                        else
                                printk("\nENCRYPTED, BYTES %06lu-%06lu: ",
                                       i + offset, i + offset + 15);
                }
                printk("%02x ", (unsigned int) *(buffer + i));
        }
        printk("\n");
}

/*
* Handle an I/O request.
*/
static void sbd_transfer(struct sbd_device *dev, unsigned long sector,
                         unsigned long nsect, char *buffer, int write)
{
        int i;
        unsigned long offset = sector * logical_block_size;
        unsigned long nbytes = nsect  * logical_block_size;
        // Encrypt and decrypt have the same block size
        unsigned int crypt_blksize = crypto_cipher_blocksize(dev->encrypt);
        u8 *memloc = dev->data + offset;

        printk(KERN_NOTICE "sbd: Starting sbd_transfer()...\n");
        printk(KERN_NOTICE "sbd: offset        = %lu\n", offset);
        printk(KERN_NOTICE "sbd: nbytes        = %lu\n", nbytes);
        printk(KERN_NOTICE "sbd: crypt_blksize = %u\n", crypt_blksize);

        if ((offset % crypt_blksize) != 0) {
                printk(KERN_ERR
                       "sbd: Offset not aligned to crypto block size\n");
                return;
        } else if (nbytes % crypt_blksize != 0) {
                printk(KERN_ERR
                       "sbd: Nbytes not aligned to crypto block size\n");
                return;
        } else if ((offset + nbytes) > dev->size) {
                printk(KERN_ERR "sbd: Beyond-end write (%ld %ld)\n",
                                    offset, nbytes);
                return;
        }

        if (write) {
                int i;

                printk(KERN_NOTICE "sbd: Operation type: write\n");

                for (i = 0; i < nbytes; i += crypt_blksize)
                        crypto_cipher_encrypt_one(dev->encrypt, memloc + i,
                                                  buffer + i);

                for (i = (offset / crypt_blksize);
                     i < ((offset + nbytes) / crypt_blksize);
                     ++i)
                        (dev->written)[i] = 1;

                printk(KERN_NOTICE "sbd: Data before encryption:");
                sbd_print_buffer(buffer, nbytes, offset, SBD_PLAINTEXT);
                printk(KERN_NOTICE "sbd: Data after encryption:");
                sbd_print_buffer(memloc, nbytes, offset, SBD_ENCRYPTED);
        } else {
                printk(KERN_NOTICE "sbd: Operation type: read\n");

                for (i = 0; i < nbytes; i += crypt_blksize) {
                        if ((dev->written)[(offset + i) / crypt_blksize]) {
                                crypto_cipher_decrypt_one(dev->decrypt,
                                                          buffer + i,
                                                          memloc + i);
                        } else {
                                memcpy(buffer + i, memloc + i,
                                       crypt_blksize);
                        }
                }
                printk(KERN_NOTICE "sbd: Data before decryption:");
                sbd_print_buffer(memloc, nbytes, offset, SBD_ENCRYPTED);
                printk(KERN_NOTICE "sbd: Data after decryption:");
                sbd_print_buffer(buffer, nbytes, offset, SBD_PLAINTEXT);
        }

        printk(KERN_NOTICE "sbd: Ending sbd_transfer()...\n");
}

static void sbd_request(struct request_queue *q)
{
        struct request *req;
        printk(KERN_NOTICE "sbd: Starting sbd_request()...\n");
        req = blk_fetch_request(q);
        while (req != NULL) {
                if (req == NULL || (req->cmd_type != REQ_TYPE_FS)) {
                        printk(KERN_NOTICE "Skip non-CMD request\n");
                        __blk_end_request_all(req, -EIO);
                        continue;
                }
                sbd_transfer(&Device, blk_rq_pos(req), blk_rq_cur_sectors(req),
                             bio_data(req->bio), rq_data_dir(req));
                if (!__blk_end_request_cur(req, 0)) {
                        req = blk_fetch_request(q);
                }
        }
        printk(KERN_NOTICE "sbd: Ending sbd_request()...\n");
}

/*
 * The HDIO_GETGEO ioctl is handled in blkdev_ioctl(), which
 * calls this. We need to implement getgeo, since we can't
 * use tools such as fdisk to partition the drive otherwise.
 */
int sbd_getgeo(struct block_device * block_device, struct hd_geometry * geo)
{
        long size;

        printk(KERN_NOTICE "sbd: Starting sbd_getgeo()...\n");

        /* We have no real geometry, of course, so make something up. */
        size = Device.size * (logical_block_size / KERNEL_SECTOR_SIZE);
        geo->cylinders = (size & ~0x3f) >> 6;
        geo->heads = 4;
        geo->sectors = 16;
        geo->start = 0;

        printk(KERN_NOTICE "sbd: Ending sbd_getgeo()...\n");

        return 0;
}

/*
* The device operations structure.
*/
static struct block_device_operations sbd_ops = {
        .owner = THIS_MODULE,
        .getgeo = sbd_getgeo
};

static int __init sbd_init(void)
{
        struct crypto_cipher *encrypt, *decrypt;
        unsigned int encrypt_blksize, decrypt_blksize;
        unsigned int ret;

        printk(KERN_NOTICE "sbd: Starting sbd_init()...\n");

        /*
         * First, init our crypto.
         */

        // If both keys are given, use them; if one is given, it is used for
        // both; if no keys are given, use the default for both

        if (ekeylen != 0 && dkeylen != 0 && ekeylen != dkeylen) {
                printk(KERN_WARNING "sbd: keys must be the same length\n");
                goto out;
        }

        encrypt = crypto_alloc_cipher("aes", 0, 0);
        if (IS_ERR(encrypt)) {
                printk(KERN_WARNING "sbd: unable to allocate cipher\n");
                goto out;
        }
        encrypt_blksize = crypto_cipher_blocksize(encrypt);

        if (ekeylen == 0 && dkeylen == 0) {
                ret = crypto_cipher_setkey(encrypt, default_key, default_keylen);
        } else if (ekeylen == 0) {
                ret = crypto_cipher_setkey(encrypt, decrypt_key, dkeylen);
        } else {
                ret = crypto_cipher_setkey(encrypt, encrypt_key, ekeylen);
        }
        if (ret) {
                printk(KERN_NOTICE "sbd: crypto_blkcipher_setkey() failed\n");
                goto out;
        }

        Device.encrypt = encrypt;

        decrypt = crypto_alloc_cipher("aes", 0, 0);
        if (IS_ERR(decrypt)) {
                printk(KERN_WARNING "sbd: unable to allocate cipher\n");
                goto out;
        }
        decrypt_blksize = crypto_cipher_blocksize(decrypt);

        if (ekeylen == 0 && dkeylen == 0) {
                ret = crypto_cipher_setkey(decrypt, default_key, default_keylen);
        } else if (dkeylen == 0) {
                ret = crypto_cipher_setkey(decrypt, encrypt_key, ekeylen);
        } else {
                ret = crypto_cipher_setkey(decrypt, decrypt_key, dkeylen);
        }
        if (ret) {
                printk(KERN_NOTICE "sbd: crypto_blkcipher_setkey() failed\n");
                goto out;
        }

        Device.decrypt = decrypt;

        if (encrypt_blksize != decrypt_blksize) {
                printk(KERN_WARNING "sbd: block lengths must be the same\n");
                goto out;
        }

        /*
        * Set up our internal device.
        */
        Device.size = nsectors * logical_block_size;
        spin_lock_init(&Device.lock);
        Device.data    = vzalloc(Device.size);
        Device.written = vzalloc(Device.size / encrypt_blksize);
        if (Device.data == NULL)
                return -ENOMEM;
        /*
        * Get a request queue.
        */
        Queue = blk_init_queue(sbd_request, &Device.lock);
        if (Queue == NULL)
                goto out;
        blk_queue_logical_block_size(Queue, logical_block_size);
        /*
        * Get registered.
        */
        major_num = register_blkdev(major_num, "sbd");
        if (major_num <= 0) {
                printk(KERN_WARNING "sbd: unable to get major number\n");
                goto out;
        }
        /*
        * And the gendisk structure.
        */
        Device.gd = alloc_disk(16);
        if (!Device.gd)
                goto out_unregister;
        Device.gd->major        = major_num;
        Device.gd->first_minor  = 0;
        Device.gd->fops         = &sbd_ops;
        Device.gd->private_data = &Device;
        strcpy(Device.gd->disk_name, "sbd0");
        set_capacity(Device.gd, nsectors);
        Device.gd->queue        = Queue;
        add_disk(Device.gd);

        printk(KERN_NOTICE "sbd: Ending sbd_init()...\n");

        return 0;

out_unregister:
        unregister_blkdev(major_num, "sbd");
out:
        vfree(Device.written);
        vfree(Device.data);
        crypto_free_cipher(Device.encrypt);
        crypto_free_cipher(Device.decrypt);
        printk(KERN_NOTICE "sbd: Ending sbd_init()...\n");

        return -ENOMEM;
}

static void __exit sbd_exit(void)
{
        printk(KERN_NOTICE "sbd: Starting sbd_exit()...\n");

        del_gendisk(Device.gd);
        put_disk(Device.gd);
        unregister_blkdev(major_num, "sbd");
        blk_cleanup_queue(Queue);
        vfree(Device.written);
        vfree(Device.data);
        crypto_free_cipher(Device.encrypt);
        crypto_free_cipher(Device.decrypt);

        printk(KERN_NOTICE "sbd: Ending sbd_exit()...\n");
}

module_init(sbd_init);
module_exit(sbd_exit);
