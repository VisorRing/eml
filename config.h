#ifndef CONFIG_H
#define CONFIG_H

#define  cDataTop		"/export/web/htvar"
#define  cDEFAULTTIMELIMIT	300
#define  cLOGSHORTSIZE		500
#define  cPOSTLIMITDEFAULT	(1LL*1024*1024)
#define  cPOSTLIMITHARD		(10LL*1024*1024)
#define  cPOSTFILELIMITDEFAULT	(3LL*1024*1024)
#define  cPOSTFILELIMITHARD	(500LL*1024*1024)
#define  cResourceFileMax	(5LL*1024*1024)
#define  cmd_qmailinject	"/export/mail/qmail/bin/qmail-inject"
#define  cmd_rm			"/bin/rm"
#define  path_devnull		"/dev/null"
#define  memcached_ip		"127.0.0.1"
#define  memcached_port		11211
#define  memcached_socket	"memcached/cache.sock"
#define  kCERTFILE		"/usr/local/share/certs/ca-root-nss.crt"

#define  SHARE_AUTHDB		1
//#define  WIKICOMPAT		1
//#define  WIKITABLEATTRIB	1
//#define  WIKITABLECOMPATFLAG	1
//#define  WIKIMOTORPRE		1
#define  BOOTSTRAPHACK		1
//#define  COMPAT_STORAGE_CREATE	1
//#define  HTTPS_NOCACHE	1
//#define  QUERYENCODEALT		1
#define  STANDALONE		1
//#define FreeBSDPortsIConv	1
#define  INSERTTABHACK		1

#define k_crypto_bf_key "\xf4\x06\x0b\x42\xca\x52\xba\x1c\xe3\x77\x74\xb6\xcf\x03\x67\xff"      /* blowfish key: 32bits - 448bits long */
#define k_crypto_bf_iv  "\x00\x00\x00\x00\x00\x00\x00\x00"      /* blowfish iv: 64bits long */

#endif /* CONFIG_H */
