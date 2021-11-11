/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __FAST_COMMIT_H__
#define __FAST_COMMIT_H__

/* Fast commit tags */
#define PXT4_FC_TAG_ADD_RANGE		0x0001
#define PXT4_FC_TAG_DEL_RANGE		0x0002
#define PXT4_FC_TAG_CREAT		0x0003
#define PXT4_FC_TAG_LINK		0x0004
#define PXT4_FC_TAG_UNLINK		0x0005
#define PXT4_FC_TAG_INODE		0x0006
#define PXT4_FC_TAG_PAD			0x0007
#define PXT4_FC_TAG_TAIL		0x0008
#define PXT4_FC_TAG_HEAD		0x0009

#define PXT4_FC_SUPPORTED_FEATURES	0x0

/* On disk fast commit tlv value structures */

/* Fast commit on disk tag length structure */
struct pxt4_fc_tl {
	__le16 fc_tag;
	__le16 fc_len;
};

/* Value structure for tag PXT4_FC_TAG_HEAD. */
struct pxt4_fc_head {
	__le32 fc_features;
	__le32 fc_tid;
};

/* Value structure for PXT4_FC_TAG_ADD_RANGE. */
struct pxt4_fc_add_range {
	__le32 fc_ino;
	__u8 fc_ex[12];
};

/* Value structure for tag PXT4_FC_TAG_DEL_RANGE. */
struct pxt4_fc_del_range {
	__le32 fc_ino;
	__le32 fc_lblk;
	__le32 fc_len;
};

/*
 * This is the value structure for tags PXT4_FC_TAG_CREAT, PXT4_FC_TAG_LINK
 * and PXT4_FC_TAG_UNLINK.
 */
struct pxt4_fc_dentry_info {
	__le32 fc_parent_ino;
	__le32 fc_ino;
	u8 fc_dname[0];
};

/* Value structure for PXT4_FC_TAG_INODE and PXT4_FC_TAG_INODE_PARTIAL. */
struct pxt4_fc_inode {
	__le32 fc_ino;
	__u8 fc_raw_inode[0];
};

/* Value structure for tag PXT4_FC_TAG_TAIL. */
struct pxt4_fc_tail {
	__le32 fc_tid;
	__le32 fc_crc;
};

/*
 * In memory list of dentry updates that are performed on the file
 * system used by fast commit code.
 */
struct pxt4_fc_dentry_update {
	int fcd_op;		/* Type of update create / unlink / link */
	int fcd_parent;		/* Parent inode number */
	int fcd_ino;		/* Inode number */
	struct qstr fcd_name;	/* Dirent name */
	unsigned char fcd_iname[DNAME_INLINE_LEN];	/* Dirent name string */
	struct list_head fcd_list;
};

/*
 * Fast commit reason codes
 */
enum {
	/*
	 * Commit status codes:
	 */
	PXT4_FC_REASON_OK = 0,
	PXT4_FC_REASON_INELIGIBLE,
	PXT4_FC_REASON_ALREADY_COMMITTED,
	PXT4_FC_REASON_FC_START_FAILED,
	PXT4_FC_REASON_FC_FAILED,

	/*
	 * Fast commit ineligiblity reasons:
	 */
	PXT4_FC_REASON_XATTR = 0,
	PXT4_FC_REASON_CROSS_RENAME,
	PXT4_FC_REASON_JOURNAL_FLAG_CHANGE,
	PXT4_FC_REASON_NOMEM,
	PXT4_FC_REASON_SWAP_BOOT,
	PXT4_FC_REASON_RESIZE,
	PXT4_FC_REASON_RENAME_DIR,
	PXT4_FC_REASON_FALLOC_RANGE,
	PXT4_FC_REASON_INODE_JOURNAL_DATA,
	PXT4_FC_COMMIT_FAILED,
	PXT4_FC_REASON_MAX
};

struct pxt4_fc_stats {
	unsigned int fc_ineligible_reason_count[PXT4_FC_REASON_MAX];
	unsigned long fc_num_commits;
	unsigned long fc_ineligible_commits;
	unsigned long fc_numblks;
};

#define PXT4_FC_REPLAY_REALLOC_INCREMENT	4

/*
 * Physical block regions added to different inodes due to fast commit
 * recovery. These are set during the SCAN phase. During the replay phase,
 * our allocator excludes these from its allocation. This ensures that
 * we don't accidentally allocating a block that is going to be used by
 * another inode.
 */
struct pxt4_fc_alloc_region {
	pxt4_lblk_t lblk;
	pxt4_fsblk_t pblk;
	int ino, len;
};

/*
 * Fast commit replay state.
 */
struct pxt4_fc_replay_state {
	int fc_replay_num_tags;
	int fc_replay_expected_off;
	int fc_current_pass;
	int fc_cur_tag;
	int fc_crc;
	struct pxt4_fc_alloc_region *fc_regions;
	int fc_regions_size, fc_regions_used, fc_regions_valid;
	int *fc_modified_inodes;
	int fc_modified_inodes_used, fc_modified_inodes_size;
};

#define region_last(__region) (((__region)->lblk) + ((__region)->len) - 1)


#endif /* __FAST_COMMIT_H__ */
