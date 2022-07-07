#ifndef ML_MEMCACHED_H
#define ML_MEMCACHED_H

#include "ml.h"
#include "ml-id.h"
#include "ustring.h"
#include <libmemcached/memcached.h>

class  MNode;
class  MlEnv;

class  MLMemcached: public MLFunc {
 public:
    memcached_st*  mem;

    MLMemcached (MlEnv* _mlenv): MLFunc (cMLMemcachedID, _mlenv) {
	mem = memcached_create (NULL);
    };
    virtual  ~MLMemcached () {
	if (mem) {
	    memcached_free (mem);
	    mem = NULL;
	}
    };

};

MNode*  ml_memcached (MNode* cell, MlEnv* mlenv);
MNode*  ml_memcached_cache_set (MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_memcached_cache_add (MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_memcached_cache_replace (MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_memcached_cache_get (MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_memcached_cache_delete (MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_memcached_cache_increment (MNode* cell, MlEnv* mlenv, MLFunc* mobj);
MNode*  ml_memcached_cache_decrement (MNode* cell, MlEnv* mlenv, MLFunc* mobj);

#endif /* ML_MEMCACHED_H */
