/*
 * Constants.h
 *
 *  Created on: Sep 6, 2014
 *      Author: psarda
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <string>

using std::string;

namespace JournalServiceServer{
const long INVALID_TXID = -12345;
/* Journal service allows to log various operation types but we are using
   * only DisallowSnapshotOp to read/write logs as opaque strings. This is a
   * OPCODE for that operation.
   */
const char OPCODE = 30;

const string EDITS_INPROGRESS("edits_inprogress");
const string EDITS("edits");

enum StorageState {
       NON_EXISTENT=0,
       NOT_FORMATTED,
       COMPLETE_UPGRADE,
       RECOVER_UPGRADE,
       COMPLETE_FINALIZE,
       COMPLETE_ROLLBACK,
       RECOVER_ROLLBACK,
       COMPLETE_CHECKPOINT,
       RECOVER_CHECKPOINT,
       NORMAL
};

enum LogLevel
  {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
  };

}

#endif /* CONSTANTS_H_ */
