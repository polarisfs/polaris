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
