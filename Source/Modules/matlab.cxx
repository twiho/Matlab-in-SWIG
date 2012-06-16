#include "swigmod.h"
#define DEBUG

#define MEXACT 1
#define MCOMPATIBLE 2
#define MFREE 3
#define MCLASS 4

class MATLAB : public Language {

protected:

  struct {
      bool isCpp;
      bool inClass;
  } flags;

  void generateCppBaseClass(String *filePath);
  void generateCppPointerClass(String *filePath);
  void generateCppDummyPointerClass(String *filePath);

  String *getMatlabType(Parm *p, int *pointerCount, int *referenceCount, int typeType);

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

String *MATLAB::getMatlabType(Parm *p, int *pointerCount, int *referenceCount, int typeType) {
  SwigType *type = Getattr(p,"type");
  String *name = Getattr(p,"name");
  if(pointerCount != 0)
    *pointerCount = 0;
  while (SwigType_ispointer(type)) {
    SwigType_del_pointer(type);
    if(pointerCount != 0)
      *pointerCount += 1;
  }
  if(referenceCount != 0)
    *referenceCount = 0;
  while (SwigType_isreference(type)) {
    SwigType_del_reference(type);
    if(referenceCount != 0)
      *referenceCount += 1;
  }
  Parm *p2 = NewParm(type,name,p);
  String *mClass = Swig_typemap_lookup("mclass",p2,"",0);
  if(mClass)
      return mClass;
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

}



int MATLAB::classHandler(Node* n) {
  String* kind = Getattr(n,"kind");
  if(!Strcmp(kind,"struct")) {
    //TODO copy the declaration to header file
  }
  if(!Strcmp(kind,"class")) {
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
    Swig_typemap_register("mclass",classParm,matlabFullClassName,0,0);
    //delete(classParm);
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
        Parm* classParm = NewParm(cppSuperClassName,0,n);
        String *matlabFullSuperClassName = getMatlabType(classParm,0,0,MCLASS);
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
    Language::classHandler(n); //poresi nam ruzne gety a sety a konstruktory atp

    Dump(mClass_content, mClass_file);
    Close(mClass_file);
    Delete(mClass_file);
    Delete(mClass_fileName);

    flags.inClass = false;
  }
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
