
#ifndef WIKICMD_H
#define WIKICMD_H

#include "ustring.h"

class  WikiLine;
class  WikiBlock;
class  WikiFormat;

void  wc_repeat (WikiLine* wl, WikiFormat* wiki);
//void  wc_doarray (WikiLine* wl, WikiFormat* wiki);
//void  wc_reversedoarray (WikiLine* wl, WikiFormat* wiki);
//void  wc_dotable (WikiLine* wl, WikiFormat* wiki);
void  wc_block (WikiLine* wl, WikiFormat* wiki);
void  wc_switch (WikiLine* wl, WikiFormat* wiki);
//void  wc_setvar (WikiLine* wl, WikiFormat* wiki);
void  wc_eval (WikiLine* wl, WikiFormat* wiki);
void  wc_evalblock (WikiLine* wl, WikiFormat* wiki);
void  wc_insert (WikiLine* wl, WikiFormat* wiki);
void  wc_data (WikiLine* wl, WikiFormat* wiki);
void  wc_local (WikiLine* wl, WikiFormat* wiki);
void  wc_macro (WikiLine* wl, WikiFormat* wiki);
void  wc_element (WikiLine* wl, WikiFormat* wiki);
void  wp_element (WikiLine* wl, WikiFormat* wiki);
void  wc_input_text (WikiLine* wl, WikiFormat* wiki);
void  wc_input_textarea (WikiLine* wl, WikiFormat* wiki);
void  wc_input_int (WikiLine* wl, WikiFormat* wiki);
void  wc_input_real (WikiLine* wl, WikiFormat* wiki);
void  wc_input_int_or_blank (WikiLine* wl, WikiFormat* wiki);
void  wc_input_real_or_blank (WikiLine* wl, WikiFormat* wiki);
void  wc_input_ascii (WikiLine* wl, WikiFormat* wiki);
void  wc_input_bool (WikiLine* wl, WikiFormat* wiki);
void  wc_call_defun (WikiLine* wl, WikiFormat* wiki);
void  wc_premode (WikiLine* wl, WikiFormat* wiki);
void  wikiOutput (const ustring& text, bool fsuper, WikiFormat* wiki);
void  wc_list_each (WikiLine* wl, WikiFormat* wiki);
void  wc_vector_each (WikiLine* wl, WikiFormat* wiki);
void  wc_dovector (WikiLine* wl, WikiFormat* wiki);
//void  wc_reversedovector (WikiLine* wl, WikiFormat* wiki);
void  wc_table_each (WikiLine* wl, WikiFormat* wiki);

#endif /* WIKICMD_H */
