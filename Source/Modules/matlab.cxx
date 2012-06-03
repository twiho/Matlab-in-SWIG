#include "swigmod.h"
#define DEBUG


class MATLAB : public Language {

protected:
  typedef struct {
    String* cppName;
    String* matlabName;
  } ClassNameItem;

  struct {
    int size;
    int capacity;
    ClassNameItem* list;
  } ClassNameList;

  bool ClassNameList_add(String* cppName, String* matlabName);
  String* ClassNameList_getMatlabName(String* cppName);
#ifdef DEBUG
  void ClassNameList_print();
#endif

  File * f_begin;
  String * f_runtime;
  String * f_init;
  String * f_header;
  String * f_wrappers;

  String * mClass_content;

public:

  virtual void main(int argc, char *argv[]);
  virtual int top(Node *n);
  int classHandler(Node *n);
  int functionWrapper(Node *n);

};

bool MATLAB::ClassNameList_add(String* cppName, String* matlabName) {
  if(ClassNameList.size == ClassNameList.capacity) {
    void* temp_ptr = realloc(ClassNameList.list, 2*ClassNameList.capacity*sizeof(ClassNameItem));
    if(temp_ptr == 0)
      return false;
    ClassNameList.list = (ClassNameItem*) temp_ptr;
    ClassNameList.capacity *= 2;
  }
  ClassNameItem newClassNameItem;
  newClassNameItem.cppName = cppName;
  newClassNameItem.matlabName = matlabName;
  ClassNameList.list[ClassNameList.size] = newClassNameItem;
  ClassNameList.size++;
  return true;
}

String* MATLAB::ClassNameList_getMatlabName(String* cppName) {
  for(int i=0; i<ClassNameList.size; i++)
    if(!Strcmp(ClassNameList.list[i].cppName,cppName))
      return ClassNameList.list[i].matlabName;
  return 0;
}

#ifdef DEBUG
void MATLAB::ClassNameList_print() {
  Printf(stderr,"\nClassNames:\n");
  for(int i=0; i<ClassNameList.size; i++)
    Printf(stderr,"%s = %s",ClassNameList.list[i].cppName,ClassNameList.list[i].matlabName);
}
#endif

int MATLAB::top(Node *n) {

   /* Initialize I/O */
    
    //fill outfile with out file name
    String *outfile=NewString(Getattr(n,"name"));
    Append(outfile,"wrap.cpp"); //toto neni spravne, dodelam az budu v Brne - zatim tento hack

    f_begin = NewFile(outfile, "w", SWIG_output_files());
    if (!f_begin) {
       FileErrorDisplay(outfile);
       SWIG_exit(EXIT_FAILURE);
    }
    f_runtime = NewString("");
    f_init = NewString("");
    f_header = NewString("");
    f_wrappers = NewString("");

   /* Register file targets with the SWIG file handler */
    Swig_register_filebyname("begin", f_begin);
    Swig_register_filebyname("header", f_header);
    Swig_register_filebyname("wrapper", f_wrappers);
    Swig_register_filebyname("runtime", f_runtime);
    Swig_register_filebyname("init", f_init);

   /* Output module initialization code */
    Swig_banner(f_begin);


   /* Emit code for children */
    Language::top(n);


    /* Write all to the file */
    Dump(f_runtime, f_begin);
    Dump(f_header, f_begin);
    Dump(f_wrappers, f_begin);
    Wrapper_pretty_print(f_init, f_begin);
 
    /* Cleanup files */
    Delete(f_runtime);
    Delete(f_header);
    Delete(f_wrappers);
    Delete(f_init);
    Close(f_begin);
    Delete(f_begin);

  return SWIG_OK;
}


void MATLAB::main(int argc, char *argv[]) {
  /* Set language-specific subdirectory in SWIG library */
  SWIG_library_directory("matlab");

  /* Set language-specific preprocessing symbol */
  Preprocessor_define("SWIGMATLAB 1", 0);

  /* Set language-specific configuration file */
  SWIG_config_file("matlab.swg");

  /* Set typemap language (historical) */
  SWIG_typemap_lang("matlab");

  /* Initialize ClassNameList */
  ClassNameList.list = (ClassNameItem*) malloc(10*sizeof(ClassNameItem));
  if(ClassNameList.list == 0)
      SWIG_exit(EXIT_FAILURE);
  ClassNameList.size = 0;
  ClassNameList.capacity = 10;
}


int MATLAB::classHandler(Node* n) {
  String   *name   = Getattr(n,"sym:name");
  Printf(stderr,"classWrapper   : %s\n", name);

  String * mClass_fileName = NewString("jmenoTridy.m"); //?
  File * mClass_file = NewFile(mClass_fileName, "w", SWIG_output_files());
  if (!mClass_file) {
     FileErrorDisplay(mClass_file);
     SWIG_exit(EXIT_FAILURE);
  }

  mClass_content=NewString(""); //sem se zapise mfile pro tridu

  //Language::classHandler(n); //poresi nam ruzne gety a sety a konstruktory atp

  Dump(mClass_content, mClass_file);
  Close(mClass_file);
  Delete(mClass_content);
  Delete(mClass_file);
  Delete(mClass_fileName);

}


int MATLAB::functionWrapper(Node *n) {
  /* Get some useful attributes of this function */
  String   *name   = Getattr(n,"sym:name");
  SwigType *type   = Getattr(n,"type");
  ParmList *parms  = Getattr(n,"parms");
  String   *parmstr= ParmList_str_defaultargs(parms); // to string
  String   *func   = SwigType_str(type, NewStringf("%s(%s)", name, parmstr));
  String   *action = Getattr(n,"wrap:action");
/*
  File * mClass_file = NewFile(mClass_fileName, "w", SWIG_output_files());
  if (!mClass_file) {
     FileErrorDisplay(mClass_file);
     SWIG_exit(EXIT_FAILURE);
  }
*/

  String* overloadTestStr1; // Exact type match
  String* overloadTestStr2; // Type size mismatch
  String* overloadTestStr3; // Necessary type conversion

  Printf(stderr,"functionWrapper   : %s\n", func);
  Printf(stderr,"           action : %s\n", action);
  return SWIG_OK;
}


extern "C" Language *
swig_matlab(void) {
  return new MATLAB();
}
