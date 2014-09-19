#ifndef _status_h__
#define _status_h__

#include "share/share.h"
#include "rbtree/rbtree.h"

GPublic void status_init(RBTreeP_t pTree);
GPublic void status_sample(String_t szContext);

#endif