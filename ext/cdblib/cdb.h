#ifndef CDB_LIB_H
#define CDB_LIB_H

struct cdb {
  char *map; /* 0 if no map is available */
  int fd;
  uint32_t size; /* initialized if map is nonzero */
  uint32_t loop; /* number of hash slots searched under this key */
  uint32_t khash; /* initialized if loop is nonzero */
  uint32_t kpos; /* initialized if loop is nonzero */
  uint32_t hpos; /* initialized if loop is nonzero */
  uint32_t hslots; /* initialized if loop is nonzero */
  uint32_t dpos; /* initialized if cdb_findnext() returns 1 */
  uint32_t dlen; /* initialized if cdb_findnext() returns 1 */
} ;

extern void cdb_free(struct cdb *);
extern void cdb_init(struct cdb *,int fd);

extern int cdb_read(struct cdb *,char *,unsigned int,uint32_t);

extern void cdb_findstart(struct cdb *);
extern int cdb_findnext(struct cdb *,char *,unsigned int);
extern int cdb_find(struct cdb *,char *,unsigned int);

#define cdb_datapos(c) ((c)->dpos)
#define cdb_datalen(c) ((c)->dlen)

extern void cdb_free_mem (struct cdb* c);
extern void cdb_init_mem (struct cdb *c, unsigned char* data, uint32_t size);

#endif /* CDB_LIB_H */
