#include "mail.h"

//Content-Type: multipart/alternative; boundary="boundary"
//Content-Transfer-Encoding: 7bit
//--boundary
//Content-Type: text/plain; charset=iso-2022-jp
//Content-Transfer-Encoding: 7bit
//
//--boundary
//Content-Type: text/html; charset=iso-2022-jp
//Content-Transfer-Encoding: 7bit
//
//--boundary--


//Content-Type: multipart/mixed; boundary=boundary
//--boundary
//Content-Disposition: attachment;
//filename*=iso-2022-jp''%1B%24B%257%25k%257%21%3C%1B%28B.swf
//Content-Type: application/octet-stream;
//name="=?iso-2022-jp?B?GyRCJTclayU3ITwbKEIuc3dm?="
//Content-Transfer-Encoding: base64
//
//Q1dTCj0JAAB4nKWUCVATdxTG/wkJSjIqIBZRsDFUBDHZTQgQIqYcETkLRkQqUFmTBYJJNmTDZbEG
//
//--boundary
//Content-Transfer-Encoding: 7bit
//Content-Type: text/plain; charset=us-ascii
//
//asdf
//
//--boundary--
