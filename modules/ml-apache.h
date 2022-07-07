#ifndef ML_APACHE_H
#define ML_APACHE_H

class  MNode;
class  MlEnv;

MNode*  ml_server_name (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_server_port (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_remote_user (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_remote_referer (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_remote_user_agent (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_extra_path (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_apache_query_string (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_script_path (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_apache_redirect_url (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_remote_ip (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_request_method (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_request_type (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_apache_https (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_ssl_client_m_serial (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_ssl_client_s_dn (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_ssl_client_i_dn (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_is_https (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_request_header (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_get_ssl_env (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_is_get_method (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_is_post_method (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_absolute_url (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_request_cookie (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_APACHE_H */
