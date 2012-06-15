#include "swigmod.h"
#define DEBUG

#define MEXACT 1
#define MCOMPATIBLE 2
#define MFREE 3

class MATLAB : public Language {

protected:
  typedef struct {
    String* cppName;
    String* matlabFullName;
  } ClassNameItem;

  struct {
    int size;
    int capacity;
    ClassNameItem* list;
  } ClassNameList;

  bool ClassNameList_add(String* cppName, String* matlabName);
  String* ClassNameList_getMatlabFullName(String* cppName);
#ifdef DEBUG
  void ClassNameList_print();
#endif

  void generateCppBaseClass(String *filePath);
  void generateCppPointerClass(String *filePath);
  void generateCppDummyPointerClass(String *filePath);

  String *getMatlabType(Parm *p, int typeType);
  bool isClassType(Parm *p);

  struct {
      bool isCpp;
      bool inClass;
  } flags;

  File * f_begin;
  String * f_runtime;
  String * f_init;
  String * f_header;
  String * f_wrappers;

  String *module;
  String *packageDirName;

  String * mClass_content;

public:

  virtual void main(int argc, char *argv[]);
  virtual int top(Node *n);
  int classHandler(Node *n);
  int functionWrapper(Node *n);

};

bool MATLAB::ClassNameList_add(String* cppName, String* matlabFullName) {
  if(ClassNameList.size == ClassNameList.capacity) {
    void* temp_ptr = realloc(ClassNameList.list, 2*ClassNameList.capacity*sizeof(ClassNameItem));
    if(temp_ptr == 0)
      return false;
    ClassNameList.list = (ClassNameItem*) temp_ptr;
    ClassNameList.capacity *= 2;
  }
  ClassNameItem newClassNameItem;
  newClassNameItem.cppName = cppName;
  newClassNameItem.matlabFullName = matlabFullName;
  ClassNameList.list[ClassNameList.size] = newClassNameItem;
  ClassNameList.size++;
  return true;
}

String* MATLAB::ClassNameList_getMatlabFullName(String* cppName) {
  for(int i=0; i<ClassNameList.size; i++)
    if(!Strcmp(ClassNameList.list[i].cppName,cppName))
      return ClassNameList.list[i].matlabFullName;
  return 0;
}

#ifdef DEBUG
void MATLAB::ClassNameList_print() {
  Printf(stderr,"\nClassNames:\n");
  for(int i=0; i<ClassNameList.size; i++)
    Printf(stderr,"%s = %s",ClassNameList.list[i].cppName,ClassNameList.list[i].matlabFullName);
}
#endif

void MATLAB::generateCppBaseClass(String *filePath) {
  String *fileName = NewStringf("%sCppBaseClass.m",filePath?filePath:"");
  String* code = NewString("");
  Printf(code,"classdef CppBaseClass < handle\n");
  Printf(code,"    properties (GetAccess = public, SetAccess = protected)\n");
  Printf(code,"        pointer;\n");
  Printf(code,"    end\n");
  Printf(code,"end\n");
  File *mFile = NewFile(fileName,"w",SWIG_output_files());
  if (!mFile) {
    FileErrorDisplay(mFile);
    SWIG_exit(EXIT_FAILURE);
  }
  Dump(code,mFile);
  Close(mFile);
  Delete(code);
  Delete(fileName);
}

void MATLAB::generateCppPointerClass(String *filePath) {
  String *fileName = NewStringf("%sCppPointerClass.m",filePath?filePath:"");
  String* code = NewString("");
  Printf(code,"classdef CppPointerClass\n");
  Printf(code,"    properties (GetAccess = public, SetAccess = private)\n");
  Printf(code,"        pointer;\n");
  Printf(code,"    end\n");
  Printf(code,"\n");
  Printf(code,"    methods\n");
  Printf(code,"        function this = CppPointerClass(p)\n");
  Printf(code,"            if nargin ~= 1 || ~isa(p,'lib.pointer')\n");
  Printf(code,"                error('Illegal constructor call');\n");
  Printf(code,"            end\n");
  Printf(code,"            this.pointer = p;\n");
  Printf(code,"        end\n");
  Printf(code,"    end\n");
  Printf(code,"end\n");
  File *mFile = NewFile(fileName,"w",SWIG_output_files());
  if (!mFile) {
    FileErrorDisplay(mFile);
    SWIG_exit(EXIT_FAILURE);
  }
  Dump(code,mFile);
  Close(mFile);
  Delete(code);
  Delete(fileName);
}

void MATLAB::generateCppDummyPointerClass(String *filePath) {
  String *fileName = NewStringf("%sCppDummyPointerClass.m",filePath?filePath:"");
  String *code = NewString("classdef CppDummyPointerClass\nend\n");
  File *mFile = NewFile(fileName,"w",SWIG_output_files());
  if (!mFile) {
    FileErrorDisplay(mFile);
    SWIG_exit(EXIT_FAILURE);
  }
  Dump(code,mFile);
  Close(mFile);
  Delete(code);
  Delete(fileName);
}

String *MATLAB::getMatlabType(Parm *p, int typeType) {
  SwigType *type = Getattr(p,"type");
  String *name = Getattr(p,"name");
  while (SwigType_isreference(type))
    SwigType_del_reference(type);
  while (SwigType_ispointer(type))
    SwigType_del_pointer(type);
  Parm *p2 = NewParm(type,name,p);
  switch(typeType) {
      case MEXACT:
        return Swig_typemap_lookup("mexact",p2,"",0);
      case MCOMPATIBLE:
        return Swig_typemap_lookup("mcompatible",p2,"",0);
      case MFREE:
        return Swig_typemap_lookup("mfree",p2,"",0);
  }
  return 0;
}

bool MATLAB::isClassType(Parm *p) {
  SwigType *type = Getattr(p,"type");
  String *name = Getattr(p,"name");
  while (SwigType_isreference(type))
    SwigType_del_reference(type);
  while (SwigType_ispointer(type))
    SwigType_del_pointer(type);
  Parm *p2 = NewParm(type,name,p);
  return Swig_typemap_lookup("isclass",p2,"",0)?true:false;
}

int MATLAB::top(Node *n) {
  module = Getattr(n,"name");
  packageDirName = NewStringf("+%s/",module);
  Swig_new_subdirectory(NewStringEmpty(),packageDirName);
  
  // Generates matlab base classes
  if(flags.isCpp) {
      generateCppBaseClass(0);
      generateCppPointerClass(0);
      generateCppDummyPointerClass(0);
  }
  
  /* Initialize flags */
  flags.inClass = false;

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
  //this block serves argument tagging and processing
  flags.isCpp = false;
  for (int i = 1; i < argc; i++) {
    if (argv[i]) {
      if(strcmp(argv[i],"-c++") == 0) {
          flags.isCpp = true;
      }
    }
  }
    
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
  flags.inClass = true;
  /* Getting class names and file path */
  String *mClass_fileName = NewString(packageDirName);
  String *cppClassName = Getattr(n,"classtype");
  String *matlabClassName = Getattr(n,"sym:name");
  String *matlabFullClassName = NewStringf("%s.",module);
  // Resolve namespaces
  if (Getattr(n,"feature:nspace")) {
    String* cppFullClassName = Getattr(n,"name");
    String* namespaces = Swig_scopename_prefix(cppFullClassName);
    List* namespaceList = Split(namespaces,':',-1);
    for (Iterator i = First(namespaceList); i.item; i = Next(i)) {
      Printf(mClass_fileName,"+%s/",i.item);
      Printf(matlabFullClassName,"%s.",i.item);
      // creates directories for namespace packages
      Swig_new_subdirectory(NewStringEmpty(),mClass_fileName);
    }
    Delete(namespaceList);
    Delete(namespaces);
  }
  Printf(mClass_fileName,"%s.m",matlabClassName);
  Printf(matlabFullClassName,"%s",matlabClassName);
  Parm *classParm = NewParm(cppClassName,0,n);
  Swig_typemap_register("mexact",classParm,matlabFullClassName,0,0);
  Swig_typemap_register("mcompatible",classParm,matlabFullClassName,0,0);
  Swig_typemap_register("mfree",classParm,matlabFullClassName,0,0);
  Swig_typemap_register("isclass",classParm,NewStringEmpty(),0,0);
  //delete(classParm);
  ClassNameList_add(cppClassName,matlabFullClassName);
#ifdef DEBUG
  Printf(stderr,"PARSING CLASS: %s -> %s\n",cppClassName,matlabFullClassName);
#endif

  /* Creating file for Matlab class */
  File *mClass_file = NewFile(mClass_fileName,"w",SWIG_output_files());
  if (!mClass_file) {
    FileErrorDisplay(mClass_file);
    SWIG_exit(EXIT_FAILURE);
  }

  /* Matlab class header */
  mClass_content = NewStringf("classdef %s", matlabClassName);
  // Resolving inheritance
  List *superClassList = Getattr(n, "allbases");
  int superClassCount = 0;
  if (superClassList) {
    for (Iterator i = First(superClassList); i.item; i = Next(i)) {
      String *cppSuperClassName = Getattr(i.item, "classtype");
      String *matlabFullSuperClassName = ClassNameList_getMatlabFullName(cppSuperClassName);
      Printf(mClass_content," %s %s",(superClassCount?"&":"<"),matlabFullSuperClassName);
      superClassCount++;
#ifdef DEBUG
      Printf(stderr,"Inherites: %s\n",matlabFullSuperClassName);
#endif
    }
  } else {
    Printf(mClass_content," < CppBaseClass");
  }
  Delete(superClassList);
  
  Printf(mClass_content, "\n");
  // Property for pointer to C++ object
  Printf(mClass_content, "    properties (GetAccess = private, SetAccess = private)\n");
  Printf(mClass_content, "        callDestructor = false;\n");
  Printf(mClass_content, "    end\n\n");
  //Language::classHandler(n); //poresi nam ruzne gety a sety a konstruktory atp

  Dump(mClass_content, mClass_file);
  Close(mClass_file);
  Delete(mClass_file);
  Delete(mClass_fileName);

  flags.inClass = false;

  return SWIG_OK;
}


int MATLAB::functionWrapper(Node *n) {

  if (!Getattr(n, "sym:nextSibling")) {
    /* Get some useful attributes of this function */
    String   *name   = Getattr(n,"sym:name");
    SwigType *type   = Getattr(n,"type");
    ParmList *parms  = Getattr(n,"parms");
    String   *parmstr= ParmList_str_defaultargs(parms); // to string
    String   *func   = SwigType_str(type, NewStringf("%s(%s)", name, parmstr));
    String   *action = Getattr(n,"wrap:action");

    String *matlabFunctionName = Getattr(n,"sym:name");
    
    String *mFunction_content = NewString("");
  
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
  
    if (flags.inClass) {
      // TODO add indentation to mFunction_content
      Append(mClass_content,mFunction_content);
    } else {
      String *mFunction_fileName = NewString(packageDirName);
      if(Getattr(n,"feature:nspace")) {
        String* cppFullFunctionName = Getattr(n,"name");
        String* namespaces = Swig_scopename_prefix(cppFullFunctionName);
        List* namespaceList = Split(cppFullFunctionName,':',-1);
        for (Iterator i = First(namespaceList); i.item; i = Next(i)) {
          Printf(mFunction_fileName,"+%s/",i.item);
          Swig_new_subdirectory(NewStringEmpty(),mFunction_fileName);
        }
        Delete(namespaceList);
        Delete(namespaces);
      }
      Printf(mFunction_fileName,"%s.m",matlabFunctionName);
      File *mFunction_file = NewFile(mFunction_fileName,"w",SWIG_output_files());
      if (!mFunction_file) {
        FileErrorDisplay(mFunction_fileName);
        SWIG_exit(EXIT_FAILURE);
      }
      Dump(mFunction_content,mFunction_file);
      Close(mFunction_file);
      Delete(mFunction_fileName);
    }
    Delete(mFunction_content);
  }
  return SWIG_OK;
}


extern "C" Language *
swig_matlab(void) {
  return new MATLAB();
}
