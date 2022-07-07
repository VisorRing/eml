#ifndef ML_STRING_H
#define ML_STRING_H

class  MNode;
class  MlEnv;

MNode*  ml_string_eq (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_string_ne (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_string_lt (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_string_le (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_string_gt (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_string_ge (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_emptyp (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_not_emptyp (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_concat (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_megabyte (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_c3 (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_string_join (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_password_match (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_password_crypt (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_bcrypt_hash (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_substring (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_tail_substring (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_length (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_byte_length (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_pad0 (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_ellipsis (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_string_format (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_random_key (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_generate_uuid (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_to_string (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_to_symbol (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_dump_to_texp (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_read_texp (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_read_texps (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_is_ascii63 (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_sort_string (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_to_upper (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_to_lower (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_dirname (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_string_duplicate (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_string_flatten (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_STRING_H */
