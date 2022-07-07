#ifndef ML_XML_H
#define ML_XML_H

class  MNode;
class  MlEnv;

MNode*  ml_xml_read (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_xml_read_file (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_xml_output (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_xml_output_string (bool fev, MNode* cell, MlEnv* mlenv);
MNode*  ml_input_xml (bool fev, MNode* cell, MlEnv* mlenv);

#endif /* ML_XML_H */
