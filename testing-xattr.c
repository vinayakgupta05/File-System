dxattr_t *dxattr;
int i, dxattr_offset = 0;
dblock_t *dblk;
xcb_t *xcb;
fcb_t *fcb;
unsigned int block;
int num_xattrs = 0;

int tot_size_names = 0;
fcb = fs->proc->fstat_table[fd]->file->diskfile;
block = fcb->attr_block;
if(block != BLK_INVALID)
{
        dblk = (dblock_t *)disk2addr( fs->base, (block2offset( block )));
        xcb = (xcb_t *)&dblk->data;
        num_xattrs= xcb->no_xattrs;
        for ( i = 0; i < xcb->no_xattrs; i++ ) {
                dxattr = (dxattr_t *)(((char *)xcb->xattrs) + dxattr_offset);
                tot_size_names += dxattr->name_size;
                dxattr_offset += sizeof(dxattr_t) + dxattr->name_size;
        }
}

printf("XATTR for %s: NUM_XATTR = %d \t  TOTAL_SUM_OF_XATTR_NAMES = %d\n", name,num_xattrs, tot_size_names);
