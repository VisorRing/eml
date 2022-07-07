#ifndef WIKILINE_H
#define WIKILINE_H

class  MotorOutput;
class  WikiFormat;
class  WikiMotorObjVec;
class  WikiMotorObjVecVec;

bool  wl_getvar (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_vector (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_car (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_cdr (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_nth (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_join (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_table (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_eval (WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_italic (WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_bold (WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_bolditalic (WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_sup (WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_sub (WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_http (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_http_new (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_https (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_https_new (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_link (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_link_new (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_image (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_color (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_anchor (WikiMotorObjVec* arg, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_span (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_input (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_input_number (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_password (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_hidden (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_file (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_submit (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_button (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_radio (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_checkbox (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_textarea (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_pad0 (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_c3 (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_date (WikiMotorObjVecVec* args, WikiMotorObjVec* arg2, WikiMotorObjVec& out, WikiFormat* wiki);
bool  wl_q (WikiMotorObjVecVec* args, WikiMotorObjVec& out, WikiFormat *wiki);

#endif /* WIKILINE_H */
