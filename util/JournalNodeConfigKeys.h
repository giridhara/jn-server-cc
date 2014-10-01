/*
 * JournalNodeConfigKeys.h
 *
 *  Created on: Sep 5, 2014
 *      Author: psarda
 */

#ifndef JOURNALNODECONFIGKEYS_H_
#define JOURNALNODECONFIGKEYS_H_

#include <string>

namespace JournalServiceServer{
using std::string;
const string  DFS_JOURNALNODE_EDITS_DIR_KEY = "dfs.journalnode.edits.dir";
  const string  DFS_JOURNALNODE_EDITS_DIR_DEFAULT = "/tmp/hadoop/dfs/journalnode/";
  const string  DFS_JOURNALNODE_RPC_ADDRESS_KEY = "dfs.journalnode.rpc-address";
  const string  DFS_JOURNALNODE_RPC_PORT_DEFAULT = "8485";
  const string  DFS_JOURNALNODE_RPC_ADDRESS_DEFAULT = "0.0.0.0:" + DFS_JOURNALNODE_RPC_PORT_DEFAULT;

  const string  DFS_JOURNALNODE_HTTP_ADDRESS_KEY = "dfs.journalnode.http-address";
  const string  DFS_JOURNALNODE_HTTP_PORT_DEFAULT = "8480";
  const string  DFS_JOURNALNODE_HTTP_ADDRESS_DEFAULT = "0.0.0.0:" + DFS_JOURNALNODE_HTTP_PORT_DEFAULT;

  const string  DFS_JOURNALNODE_KEYTAB_FILE_KEY = "dfs.journalnode.keytab.file";
  const string  DFS_JOURNALNODE_USER_NAME_KEY = "dfs.journalnode.kerberos.principal";
  const string  DFS_JOURNALNODE_INTERNAL_SPNEGO_USER_NAME_KEY = "dfs.journalnode.kerberos.internal.spnego.principal";
}

#endif /* JOURNALNODECONFIGKEYS_H_ */
