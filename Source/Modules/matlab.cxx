#include "swigmod.h"

class MATLAB : public Language {
public:

  virtual void main(int argc, char *argv[]);
  virtual int top(Node *n);
  int functionWrapper(Node *n);

};

int MATLAB::top(Node *n) {

   /* Initialize I/O */
//   f_begin = NewFile(outfile, "w", SWIG_output_files());
//   if (!f_begin) {
//      FileErrorDisplay(outfile);
//      SWIG_exit(EXIT_FAILURE);
//   }
//   f_runtime = NewString("");
//   f_init = NewString("");
//   f_header = NewString("");
//   f_wrappers = NewString("");
//
//   /* Register file targets with the SWIG file handler */
//   Swig_register_filebyname("begin", f_begin);
//   Swig_register_filebyname("header", f_header);
//   Swig_register_filebyname("wrapper", f_wrappers);
//   Swig_register_filebyname("runtime", f_runtime);
//   Swig_register_filebyname("init", f_init);
//
//   /* Output module initialization code */
//   Swig_banner(f_begin);


   /* Emit code for children */
   Language::top(n);


//   /* Write all to the file */
//   Dump(f_runtime, f_begin);
//   Dump(f_header, f_begin);
//   Dump(f_wrappers, f_begin);
//   Wrapper_pretty_print(f_init, f_begin);
//
//   /* Cleanup files */
//   Delete(f_runtime);
//   Delete(f_header);
//   Delete(f_wrappers);
//   Delete(f_init);
//   Close(f_begin);
//   Delete(f_begin);

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
  }


int MATLAB::functionWrapper(Node *n) {
  /* Get some useful attributes of this function */
  String   *name   = Getattr(n,"sym:name");
  SwigType *type   = Getattr(n,"type");
  ParmList *parms  = Getattr(n,"parms");
  String   *parmstr= ParmList_str_defaultargs(parms); // to string
  String   *func   = SwigType_str(type, NewStringf("%s(%s)", name, parmstr));
  String   *action = Getattr(n,"wrap:action");

  Printf(stderr,"functionWrapper   : %s\n", func);
  Printf(stderr,"           action : %s\n", action);
  return SWIG_OK;
}


extern "C" Language *
swig_matlab(void) {
  return new MATLAB();
}
