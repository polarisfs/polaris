/* Jia Qiang Xu <jiaqiangxu.udel@gmail.com> */


struct POFSUnlinkParms{
	posuperblock * pofsP;
	FileGenNumber pdirGenNum;
	InodeNumber pdirInodeNum;
	
	char dentryNameP[MAX_DIRNAME+1];  // the file or directory's name to be removed
	FileGenNumber fgenNum;
	InodeNumber inodeNum;
	fscredential* fscredP;
}


/* unlink the file with the specified name in unlinkParms from the directory */
Errno POFSUnlink(posuperblock * posbP, POFSUnlinkParms * pounlinkParms)
{
  FileGenNumber pdirGen = pounlinkParms->pdirGenNum;
  
  InodeNumber pdirInodeN = pounlinkParms->pdirInodeNum;
  
  char *dentrynameP = pounlinkParms->dentryNameP;
  
  /* between the lookup of the dentry and unlink op in vfs, there could 
     be a rename op to the same file name and make the inode number 
     from the lookup obsolete. So we may only regard the provided inode number
     as a hint and recheck it in this code path. Posix does not explicitly define
     this detail. Hold this up in polaris v1 and currently treat the inodenum 
     from the lookup as the definite key to unlink */

  /* lock the file and directory */
  /* for the file to unlink we should obtain xw inode lock.
     for the directory as a fragment, rf is enough */
  /* we need to define a lock order to prevent deadlock. the key for the order
     here should be inode number. */
    lock_polaris_inode(fileid, lockMode, &lmObtainedForInode, lpiflags);
     
  /* provide the necessary metadata for the file. We may need to read the inode
     from the disk. */
    getInode(fileid, flags);
    
    lock_polaris_directory(dirid, lockMode, &lmObtainedForDir, lpdflags);
    
  /* Find the block of the directory with the name.  */
    dirblockNum = locate_dirlock(fileid, dirid, dentrynameP);
    
  /* Obtain the br lock for this block */
    lock_polaris_byteRange(fileid, lockMode, &lmObtained, start, end, flags);
  
  /* Prepare the entry for deletion. We need to do journalling based on 
     the DA of the found directory block later. So we should not allow the scenario that the 
     dirblock only reside in buffer and the disk block has invalid data before we log. 
     A flush here for this dirblock is needed in this scenario */
    prepareDeleteDentry(dentrynameP, dirblockNum);
    
  /* check for parent directory sticky bit. */
    
  /* if nlink is 1, mark the file as to-be-deleted in the inode allocmap. */
  nlink = getNlink(fileid, fileMetadata);
  
  /* maybe the rename has unlinked this dentry? */
  if (nlink == 0)
  {
  	err = E_NOENT;
  	goto exit;
  }
  
  if (nlink == 1)
  {
  	err = markToBeDeleted(fileid);  //mark in the inode bitmap
  	if (err) goto exit;
  }
  /* two journal records to be logged: DentryDelete, FileNlinkChange. 
     It should be made within one transaction. */
     
  /* 1. journal area reservation 2. prepare journalling to sync with log wrap, buffer flush and llmigrate
     3. spool the journal record 4. apply the change to the journal object 
     5. release the journal sync */
  /* Shoud we support if the journal is spooled on non-volatile storage like PCM, need flags to indicate the 
     journal disk space is volatile or not. */
  
  prepare_journal_sync();
  
  spool_journal();
  
  spool_journal();
     
  /* lock the buffer to avoid steal? */
  lockBuffer();
  
  /* dentry delete in buffer */
  dentryDelete();
  /* nlink decrement */
  nlink--;
  ChangeNlink(nlink);
  
  release_journal_sync();
  
  
  /* if nlink==0, we can do real file destroy and flag it */
  if (nlink == 0)
  {
  	flagFileDestroy(fileid);
  }
  /* release lock and if flagged, we can do real file destroy now */
  if(flagFileDestroy)
    FileDestroyOnDisk(fileid);
}







delete interface  (to be continue)
-------------------------------------------------------
struct POFSUnlinkParms{
	posuperblock * pofsP;
	FileGenNumber pdirGenNum;
	InodeNumber pdirInodeNum;
	
	char dentryNameP[MAX_DIRNAME+1];  // the file or directory's name to be removed
	FileGenNumber fgenNum;
	InodeNumber inodeNum;
	fscredential* fscredP;
}


po_unlink(target_vnode, dir_vnode, namep, dirop, cred):
{
      take_op_level(RM);

      ip=find_deleting_file(dip, namep);
      polaris_inode_lock(ip, SH);
      polaris_byteRange_lock(ip, EX, 0, MAX_OFFSET, flag);   //prevent access form other node ?  VX takes EX G and EX RW lock here  

      delete_check(dip, ip, namep);

      dnlc_remove(dip, ip, namep);         //linux, remove dnlc hard hold on entry cache

again:
      query_owner(&owner, ip);

      if (myid == owner)
      {

       }
       else if (owner != nobody)
       {
             /* commit any pending changes of the directory before sending RPC to the owner */
             inode_update_commit(dip);

             send_RPC(REMOVE, dip, ip, namep, dirop, owner);
             //response error parsing and handle......

             update_disk_inode(ip, RPC_response);
             update_disk_inode(dip, RPC_response);   //??

       }
       else
       {
            try_to_become_owner(ip);
            goto again;
       }
       release_op_level(RM);
}

recv_delete_RPC(rpc_msg):
{
       take_op_level(RM);
       get_inode(ip);
       try_to_become_owner(dip);     
       try_to_become_owner(ip);      //if failed return E_NOT_METANODE

       remove_transaction(dip, namep, ip, cred, dirop);

       relinquish_owner(ip);
       relinquish_owner(dip);
       release_op_level(RM)
}

po_iremove(ip, tranp)
{
      /* make sure indirects valid incore */
      if (error = indirect_validate(ip){
            return error;
     }

     subrecp = log_inode_subrec(ip);
     
     if (not in defer process){
          if (dir)
                nlink -= 2;
          else
                nlink --;
          if (nlink == 0)
                return 0;

         if (vcount == 1){            <<<<<< linux use hard hold DLNC, v_count>1 means multiple entry in Dcahe using the inode
                polaris_qtrunc(ip, subrecp);
        }
        if (! qtrunced){
             ip->i_gen = NEXT_IGEN(ip->i_gen);
             polaris_iflagset(ip, subrecp, PO_IREMOVE);
             return;
        }
    else{
                  if (ip->i_blocks)
                         polaris_qtrunc(ip, subrecp);
                  else
                         polaris_trunc_pages(ip, 0);
    }

    /* inode quick reuse queue, we can divide it into multiple sub queues by hash to reduce contention */
    if (need_to_put_reuse_queue(ip)){
            put_ireuse_queue(ip);
            return;
   }

   /* at this point, inode can only have IREMOVE set */

   /* quota stuff .... */

   /* zeroing indrect pointers and attribute area in inode, and reset other fields */

    /* mark vnode as to be quick trunced, linux DNLC need this flag to trunc vfs inode */
    vp->v_flag |= VQTRUNC

    /* change map,  extented map ?? */
    polaris_map_change(ip, tranp, BIT_SET);

    /* release the inode region hold */
    polaris_iregion_release(ip);
}


  /* we have become metanode at this point */    
Errno po_do_unlink(po_fs *fs, POFSUnlinkParms * pounlinkParms)
{
  po_buffer *bufp;
   po_inode *ip = pounlinkParms->deletingInodeP;
  
   po_inode *dip = pounlinkParms->parentDirP;
  
  char *dentrynameP = pounlinkParms->dentryNameP;

  /* between the lookup of the dentry and unlink op in vfs, there could 
     be a rename op to the same file name and make the inode number 
     from the lookup obsolete. So we may only regard the provided inode number
     as a hint and recheck it in this code path. Posix does not explicitly define
     this detail. Hold this up in polaris v1 and currently treat the inodenum 
     from the lookup as the definite key to unlink */

  
  /* lock the file and directory */
  /* for the file to unlink we should obtain xw inode lock.
     for the directory as a fragment, rf is enough */

  /* check for parent directory sticky bit. */
  
  /* we need to define a lock order to prevent deadlock. the key for the order
     here should be inode number. po_locktable(); */
    po_inode_lock(ip, XW, flag, &lockGet);
    po_inode_lock(dip, ww, flag, &lockGet);
    
  /* provide the necessary metadata for the file. We may need to read the inode
     from the disk. */
   po_getInode(ip, flags);

  /* Find the block of the directory with the name.  */
    dirblockNum = locate_dirlock(dip,  namep);
    
  /* Obtain the br lock for this block */
    po_byteRange_lock(dip, xw, start, end, flags);

 /* Prepare the entry for deletion. We need to do journalling based on 
     the DA of the found directory block later. So we should not allow the scenario that the 
     dirblock only reside in buffer and the disk block has invalid data before we log. 
     A flush here for this dirblock is needed in this scenario */
    prepareDeleteDentry(dentrynameP, dirblockNum);


   if(dip->data_in_inode)
   {
      //search dir block, find entry, reset and setup sub record ...
       update_dirblk_hash(subrecp, namep);
   }
   else 
   {
       bufp = dir_bread(dip, dirblockNum);
       //search dir block, find entry, reset and setup sub record ...e
       update_dirblk_hash(subrecp, namep);
   }

   /* lock the buffer to avoid steal? */
    lockBuffer();

  /* two journal records to be logged: DentryDelete, FileNlinkChange. 
     It should be made within one transaction. */
     
  /* 1. journal area reservation 
     2. prepare journalling to sync with log wrap, buffer flush and llmigrate
     3. spool the journal record 
     4. apply the change to the journal object 
     5. release the journal sync */
  /* Shoud we support if the journal is spooled on non-volatile storage like PCM, need flags to indicate the 
     journal disk space is volatile or not. */

   tranp = log_reserve_prepare(REMOVE, size_of(namep), dirblockNum);       // log size need to be confirmed, may need round up 
   add_Inode_Sublog(tranp, ip);
   add_Inode_Sublog(tranp, dip);
   add_dentryDel_Sublog(tranp, dip, bufp, dirblockNum);

   /* if nlink is 1, mark the file as to-be-deleted in the inode allocmap. */
   nlink = getNlink(ip);

   /* maybe the rename has unlinked this dentry? */
   if (nlink == 0)
   {
  	err = E_NOENT;
  	goto exit;
   }

   /* dentry delete in buffer */
    dentryDelete();

    ip->i_nlink--;
    
   //transaction retry and undo handle .......
  
   release_journal_sync();      
   dip->dirty = 1;

  /* if nlink==0, we can do real file destroy and flag it */
  if (ip->i_nlink == 0)
  {
  	err = markToBeDeleted(ip);          //mark in the inode bitmap
         err = po_iremove(ip, tranp);
  }

    log_commit(tranp);

out: 
    /* error processing and transaction undo .... */
 
｝

  
  
    





   

        


    
  
    
  
  
  


     




















B
A
A
A
B
B
B
B
B
B
}
B
A

