static int
plaintext_to_sha1(unsigned char *hash, const char *plaintext, unsigned int len)
{
	struct crypto_tfm *tfm;
	struct scatterlist sg;
	if (len > PAGE_SIZE) {
		seclvl_printk(0, KERN_ERR, "Plaintext password too large (%d "
			      "characters).  Largest possible is %lu "
			      "bytes.\n", len, PAGE_SIZE);
		return -EINVAL;
	}
	tfm = crypto_alloc_tfm("sha1", CRYPTO_TFM_REQ_MAY_SLEEP);
	if (tfm == NULL) {
		seclvl_printk(0, KERN_ERR,
			      "Failed to load transform for SHA1\n");
		return -EINVAL;
	}
	sg_init_one(&sg, (u8 *)plaintext, len);
	crypto_digest_init(tfm);
	crypto_digest_update(tfm, &sg, 1);
	crypto_digest_final(tfm, hash);
	crypto_free_tfm(tfm);
	return 0;
}
