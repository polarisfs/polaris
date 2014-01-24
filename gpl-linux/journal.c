 /* Jia Qiang Xu <jiaqiangxu.udel@gmail.com>*/

/*flags include whether this is a compound transaction log, whether it is to
  be written to a nvram */
Errno prepare_journal_update(JournalFile * jfP, JournalUpdate * jlogger, BlockType btype, DiskAddr * dap, int numSectors, int flags);  

Errno finish_journal_update(JournalUpdate * jlogger, LSN minLSN, LSN maxLsn, Boolean forceBeforeApply)

Errno undo_journal_update();

Errno prepare_journal_flush(JournalFile * jfP, JournalUpdate * jlogger, InodeNum inodeNum, BlockType btype, DiskAddr * dap, int numSectors, Boolean flushInPlace, int flags);

Errno finish_journal_flush(Errno flushErr, Boolean writeCommitRecord);

Errno updateCommitLog();

Errno reserveJournalArea(JournalFile * jfP, BlockType btype);

Errno releaseJouralArea(JournalFile * jfP, BlockType btype);

Errno getCommitLsn(JournalUpdate * jlogger) { return jlogger->commitLsn;}

Errno getMinBufLsn(JournalUpdate * jlogger) { return jlogger->minBufLsn; }

Errno getMaxBufLsn(JournalUpdate * jlogger) { return jlogger->maxBufLsn; }
 
struct journallogger{

  JournalFile * jfp;
  LSN minBufLsn;
  LSN maxBufLsn;
  LSN commitLsn;
  JournalUpdate * nextJournalUpdate;
  JournalUpdate * prevJournalUpdate;

} JournalUpdate;  



/*flags include whether this is a compound transaction log, whether it is to
  be written to a nvram */
Errno prepare_journal_update(JournalUpdate * jlogger, BlockType btype, DiskAddr * dap, int numSectors, int flags);  

Errno finish_journal_update(JournalUpdate * jlogger, LSN minLSN, LSN maxLsn, Boolean forceBeforeApply)

Errno undo_journal_update();

Errno file_journal_flush(JournalFile * jfP, JournalUpdate * jlogger, InodeNum inodeNum, BlockType btype, DiskAddr * dap, int numSectors, int flags);

Errno fs_journal_flush(po_fs *fs, size_t count, int flag, int log_type);

Errno finish_journal_flush(Errno flushErr, Boolean writeCommitRecord);

Errno updateCommitLog();

Errno reserve_log_space(int logType, size_t size, addr_t bno);

Errno release_log_space(int logType, size_t size, addr_t bno);

Errno add_Inode_Sublog(transaction *tranp, inode *ip);

Errno add_DentryDel_Sublog(transaction *tranp, inode *dip, po_buffer *bufp, addr_t bno);

Errno getCommitLsn(JournalUpdate * jlogger) { return jlogger->commitLsn;}

Errno getMinBufLsn(JournalUpdate * jlogger) { return jlogger->minBufLsn; }

Errno getMaxBufLsn(JournalUpdate * jlogger) { return jlogger->maxBufLsn; }
 
struct journallogger{

  JournalFile * jfp;
  LSN minBufLsn;
  LSN maxBufLsn;
  LSN commitLsn;
  JournalUpdate * nextJournalUpdate;
  JournalUpdate * prevJournalUpdate;
} JournalUpdate;  
