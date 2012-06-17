#include "swigmod.h"

#define MEXACT 1
#define MCOMPATIBLE 2
#define MFREE 3
#define MCLASS 4

class MATLAB : public Language {

protected:

  struct {
      bool isCpp;
      bool isDebugging;
      bool inClass;
      bool inConstructor;
      bool inDestructor;
  } flags;

  void generateCppBaseClass(String *filePath);
  void generateCppPointerClass(String *filePath);
  void generateCppDummyPointerClass(String *filePath);

  String *generateLibisloadedTest();
  String *generateMFunctionContent(Node *n);

  String *getMatlabType(Parm *p, int *pointerCount, int *referenceCount, int typeType);

  File * f_begin;
  String * f_runtime;
  String * f_init;
  String * f_header;
  String * f_wrappers;

  String *module;
  String *packageDirName;
  String *libraryFileName;

  String *mClass_content;
  String *superClassConstructorCalls;

public:

  virtual void main(int argc, char *argv[]);
  virtual int top(Node *n);
  int classHandler(Node *n);
  int functionHandler(Node *n);
  int constructorHandler(Node *n);
  int destructorHandler(Node *n);
  int functionWrapper(Node *n);

};

/*** Matlab module function ***/
void MATLAB::generateCppBaseClass(String *filePath) {
  if (flags.isDebugging)
      Printf(stdout,"Generating CppBaseClass\n");
  String *fileName = NewStringf("%sCppBaseClass.m",filePath?filePath:"");
  String* code = NewString("");
  Printf(code,"classdef CppBaseClass < handle\n\n");
  Printf(code,"properties (GetAccess = public, SetAccess = protected)\n");
  Printf(code,"    pointer;\n");
  Printf(code,"end\n");
  Printf(code,"\n");
  Printf(code,"end %%classdef\n");
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
  if (flags.isDebugging)
      Printf(stdout,"Generating CppPointerClass\n");
  String *fileName = NewStringf("%sCppPointerClass.m",filePath?filePath:"");
  String* code = NewString("");
  Printf(code,"classdef CppPointerClass\n\n");
  Printf(code,"properties (GetAccess = public, SetAccess = private)\n");
  Printf(code,"    pointer;\n");
  Printf(code,"end\n");
  Printf(code,"\n");
  Printf(code,"methods\n");
  Printf(code,"function this = CppPointerClass(p)\n");
  Printf(code,"    if nargin ~= 1 || ~isa(p,'lib.pointer')\n");
  Printf(code,"        error('Illegal constructor call');\n");
  Printf(code,"    end\n");
  Printf(code,"    this.pointer = p;\n");
  Printf(code,"end\n");
  Printf(code,"end\n");
  Printf(code,"\n");
  Printf(code,"end %%classdef\n");
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
  if (flags.isDebugging)
      Printf(stdout,"Generating CppDummyPointerClass\n");
  String *fileName = NewStringf("%sCppDummyPointerClass.m",filePath?filePath:"");
  String *code = NewString("classdef CppDummyPointerClass\nend %classdef\n");
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

String *MATLAB::generateLibisloadedTest() {
  String *code = NewStringf("    if ~libisloaded('%s')\n",libraryFileName);
  Printf(code,"        error('Library %s is not loaded. Call %s.load()');\n",libraryFileName,module);
  Printf(code,"    end\n");
  return code;
}

String *MATLAB::generateMFunctionContent(Node *n) {
  /* Generating wrapper after parsing all overloaded functions */
  if (!Getattr(n, "sym:nextSibling")) {
    String *mFunction_content = NewString("");
    if (flags.inDestructor) {
      Printf(mFunction_content,"function delete(this)\n");
      Append(mFunction_content,generateLibisloadedTest());
      Printf(mFunction_content,"    if callDestructor\n");
      //Printf(mFunction_content,"        calllib()\n"); //TODO
      Printf(mFunction_content,"    end\n");
      Printf(mFunction_content,"end\n");
      return mFunction_content;
    }
    Printf(mFunction_content,"function %s(varargin)\n",Getattr(n,"matlab:name"));
    Append(mFunction_content,generateLibisloadedTest());
    if (flags.inConstructor) {
      Append(mFunction_content,superClassConstructorCalls);
      Printf(mFunction_content,"    if nargin == 1 && isa(varargin{1},'CppDummyPointerClass')\n");
      Printf(mFunction_content,"        return;\n");
      Printf(mFunction_content,"    end\n");
      Printf(mFunction_content,"    if nargin == 1 && isa(varargin{1},'CppPointerClass')\n");
      Printf(mFunction_content,"        this.pointer = varargin{1}.pointer;\n");
      Printf(mFunction_content,"        callDestructor = true;\n");
      Printf(mFunction_content,"        return;\n");
      Printf(mFunction_content,"    end\n");
    }
    Printf(mFunction_content,"    error('Illegal function call');\n");
    Printf(mFunction_content,"end\n");
    return mFunction_content;
  }
  return 0;
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

/*** Main function ***/
void MATLAB::main(int argc, char *argv[]) {
  flags.isCpp = false;
  flags.isDebugging = false;
  flags.inClass = false;
  flags.inConstructor = false;
  flags.inDestructor = false;
  /* Parsing command line arguments */
  for (int i = 1; i < argc; i++) {
    if (argv[i]) {
      if(strcmp(argv[i],"-c++") == 0) {
          flags.isCpp = true;
      }
      if(strcmp(argv[i],"-debug-matlab") == 0) {
          flags.isDebugging = true;
          Swig_mark_arg(i);
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

/*** Function for tree parsing ***/
int MATLAB::top(Node *n) {
  module = Getattr(n,"name");
  packageDirName = NewStringf("+%s/",module);
  Swig_new_subdirectory(NewStringEmpty(),packageDirName);
  // TODO set libraryFileName
  libraryFileName = NewStringf("lib%s",module);
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
    /* Resolving namespaces */
    if (Getattr(n,"feature:nspace")) {
      String* cppFullClassName = Getattr(n,"name");
      String* namespaces = Swig_scopename_prefix(cppFullClassName);
      List* namespaceList = Split(namespaces,':',-1);
      for (Iterator i = First(namespaceList); i.item; i = Next(i)) {
        Printf(mClass_fileName,"+%s/",i.item);
        Printf(matlabFullClassName,"%s.",i.item);
        /* Creating directories for namespace packages */
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
    if(flags.isDebugging)
      Printf(stdout,"Parsing class: %s -> %s\n",cppClassName,matlabFullClassName);

    /* Creating file for Matlab class */
    File *mClass_file = NewFile(mClass_fileName,"w",SWIG_output_files());
    if (!mClass_file) {
      FileErrorDisplay(mClass_file);
      SWIG_exit(EXIT_FAILURE);
    }

    /* Matlab class header */
    mClass_content = NewStringf("classdef %s", matlabClassName);
    /* Resolving inheritance */
    List *superClassList = Getattr(n, "allbases");
    int superClassCount = 0;
    superClassConstructorCalls = NewString("");
    if (superClassList) {
      for (Iterator i = First(superClassList); i.item; i = Next(i)) {
        String *cppSuperClassName = Getattr(i.item, "classtype");
        Parm* classParm = NewParm(cppSuperClassName,0,n);
        String *matlabFullSuperClassName = getMatlabType(classParm,0,0,MCLASS);
        Printf(mClass_content," %s %s",(superClassCount?"&":"<"),matlabFullSuperClassName);
        Printf(superClassConstructorCalls,"    this = this@%s(Dummy);\n",matlabFullSuperClassName);
        superClassCount++;
        if(flags.isDebugging)
        Printf(stdout,"Inherites: %s\n",matlabFullSuperClassName);
      }
    } else {
      Printf(mClass_content," < CppBaseClass");
    }
    Delete(superClassList);
  
    Printf(mClass_content, "\n\n");
    // Property for pointer to C++ object
    Printf(mClass_content, "properties (GetAccess = private, SetAccess = private)\n");
    Printf(mClass_content, "    callDestructor = false;\n");
    Printf(mClass_content, "end\n\n");
    Language::classHandler(n); //poresi nam ruzne gety a sety a konstruktory atp
    Printf(mClass_content, "end %%classdef\n");

    Clear(superClassConstructorCalls);

    Dump(mClass_content,mClass_file);
    Close(mClass_file);
    Delete(mClass_file);
    Delete(mClass_fileName);

    flags.inClass = false;
  }
  return SWIG_OK;
}

int MATLAB::functionHandler(Node* n) {
#ifdef MATLABPRINTFUNCTIONENTRY
  Printf(stderr,"Entering functionHandler\n");
#endif
  if(Getattr(n,"feature:matlab:name")) {
    Setattr(n,"matlab:name",Getattr(n,"feature:matlab:name"));
  } else {
    Setattr(n,"matlab:name",Getattr(n,"sym:name"));
  }
  Language::functionHandler(n);
  return SWIG_OK;
}

int MATLAB::constructorHandler(Node *n) {
#ifdef MATLABPRINTFUNCTIONENTRY
  Printf(stderr,"Entering constructorHandler\n");
#endif
  Setattr(n,"matlab:name",Getattr(n,"sym:name"));
  flags.inConstructor = true;
  Language::constructorHandler(n);
  flags.inConstructor = false;
  return SWIG_OK;
}

int MATLAB::destructorHandler(Node *n) {
#ifdef MATLABPRINTFUNCTIONENTRY
  Printf(stderr,"Entering destructorHandler\n");
#endif
  flags.inDestructor = true;
  Language::destructorHandler(n);
  flags.inDestructor = false;
  return SWIG_OK;
}

int MATLAB::functionWrapper(Node *n) {
    // TODO generated setters and getters of global variables does not work
  
    /* Get some useful attributes of this function */
    String   *name   = Getattr(n,"sym:name");
    SwigType *type   = Getattr(n,"type");
    ParmList *parms  = Getattr(n,"parms");
    String   *parmstr= ParmList_str_defaultargs(parms); // to string
    String   *func   = SwigType_str(type, NewStringf("%s(%s)", name, parmstr));
    String   *action = Getattr(n,"wrap:action");
    
    String *matlabFunctionName = Getattr(n,"matlab:name");
    String *mFunction_content = generateMFunctionContent(n);
  
    if(flags.isDebugging)
      Printf(stdout,"Parsing function: %s\n",matlabFunctionName);

    
/*
    File * mClass_file = NewFile(mClass_fileName, "w", SWIG_output_files());
    if (!mClass_file) {
      FileErrorDisplay(mClass_file);
      SWIG_exit(EXIT_FAILURE);
  }
*/
#ifdef DEBUG
    Printf(stderr,"functionWrapper   : %s\n", func);
    Printf(stderr,"           action : %s\n", action);
#endif
  if(mFunction_content) {
    if (flags.inClass) {
      String *function_declarations = NewString("methods");
      // TODO solve various class functions - static, ...
      Append(function_declarations,"\n");
      Push(mFunction_content,function_declarations);
      Append(mFunction_content,"end\n\n");
      // TODO add indentation to mFunction_content
      Append(mClass_content,mFunction_content);
    } else {
      /* Resolving namespaces */
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
      /* Creating function M-file */
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
