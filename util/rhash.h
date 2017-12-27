//
// Created by System Administrator on 12/23/17.
//

#ifndef RSOCK_RHASH_H
#define RSOCK_RHASH_H

#include <string>

#include "../ktype.h"
#include "../rstype.h"

bool ValidIp4(const std::string &ip);

bool hash_equal(const char *hashed_buf, const std::string &key, const char *data, int data_len);

bool EmptyIdBuf(const IdBufType &id);
IINT8 compute_hash(char *hash, const std::string &key, const char *data, int data_len);
std::string IdBuf2Str(const IdBufType &id);
std::string HashBuf2String(const HashBufType &hash);
void GenerateIdBuf(IdBufType &hash, const std::string &key);
#endif //RSOCK_RHASH_H
