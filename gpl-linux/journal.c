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

Errno getCommitLsn { return commitLsn;}

Errno getMinBufLsn { return minBufLsn; }

Errno getMaxBufLsn { return maxBufLsn; }
 
struct journallogger{

  JournalFile * jfp;
  LSN minBufLsn;
  LSN maxBufLsn;
  JournalUpdate * nextJournalUpdate;
  JournalUpdate * prevJournalUpdate;

} JournalUpdate;  
