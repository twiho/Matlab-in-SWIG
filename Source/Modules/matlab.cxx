#include "swigmod.h"

//#define MATLABDEBUG 231
#define MATLABDEBUGDT
//#define MATLABPRINTFUNCTIONENTRY

class MATLAB : public Language {

protected:
    /* General DOH objects used for holding the strings */
    File *f_wrap_h; // .h file for Matlab loadlibrary function (it's second parameter)
    File *f_begin;
    File *f_runtime;
    File *f_header;
    File *f_wrappers;
    File *f_init;
    File *f_loadm;
    File *f_unloadm;

    String *wrap_h_code;
    String *variable_name;
    String *module;
    String *packageDirname;
    String *hfile;

    String *mClass; // .m class string
    String *mFunction; // .m function string

    bool globalvariable_flag;
    bool constantWrapper_flag;
    bool constructor_flag;
    bool copyconstructor_flag;
    bool classhandler_flag;
    bool destructor_flag;
    bool memberconstant_flag;
    bool memberfunction_flag;
    bool membervariable_flag;
    bool staticmemberfunction_flag;
    bool staticmembervariable_flag;
    bool variable_flag;

    void marshalInputArgs(ParmList *parms, int numarg, Wrapper *wrapper);
    void marshalOutput(Node *n, String *actioncode, Wrapper *wrapper);
    void emitGetterSetter(Node *n, String *wname, ParmList *parms, Wrapper *wrapper, bool is_getter);
    const String *typemapLookup(Node *n, const_String_or_char_ptr tmap_method, SwigType *type, int warning, Node *typemap_attributes);
    String *getOverloadedName(Node *n);

    String *WriteMString(Node *n);

    String* getEquivalentType(String* st);
    String* testEquivalentType(int i, SwigType* st);
    String* testCompatibleType(int i, SwigType* st);



public:

    MATLAB():
        globalvariable_flag(false),
        constantWrapper_flag(false),
        constructor_flag(false),
        copyconstructor_flag(false),
        destructor_flag(false),
        memberconstant_flag(false),
        memberfunction_flag(false),
        membervariable_flag(false),
        staticmemberfunction_flag(false),
        staticmembervariable_flag(false),
        variable_flag(false),
        classhandler_flag(false) {
        module = NewString("");
    }

    virtual void main(int argc, char *argv[]) {
        //this block serves argument tagging and processing
        /*for (int i = 1; i < argc; i++) {
            if (argv[i]) {
                if(strcmp(argv[i],"-myswitch") == 0) {
                    if (argv[i+1]) {
                        interface = NewString(argv[i+1]);
                        Swig_mark_arg(i);
                        Swig_mark_arg(i+1);
                        i++;
                    } else {
                        Swig_arg_error();
                    }
                }
            }
        }*/


        /* Set language-specific subdirectory in SWIG library */
        SWIG_library_directory("matlab");

        /* Set language-specific preprocessing symbol */
        Preprocessor_define("SWIGMATLAB 1", 0);

        /* Set language-specific configuration file */
        SWIG_config_file("matlab.swg");

        /* Set typemap language (historical) */
        SWIG_typemap_lang("matlab");

        allow_overloading();
    }


    virtual int top(Node *n);

    /* Function handlers */

    virtual int functionHandler(Node *n);
    virtual int globalfunctionHandler(Node *n);
    virtual int memberfunctionHandler(Node *n);
    virtual int staticmemberfunctionHandler(Node *n);
    virtual int callbackfunctionHandler(Node *n);

    /* Variable handlers */

    virtual int variableHandler(Node *n);
    virtual int globalvariableHandler(Node *n);
    virtual int membervariableHandler(Node *n);
    virtual int staticmembervariableHandler(Node *n);

    /* C++ handlers */

    virtual int memberconstantHandler(Node *n);
    virtual int constructorHandler(Node *n);
    virtual int copyconstructorHandler(Node *n);
    virtual int destructorHandler(Node *n);
    virtual int classHandler(Node *n);

    /* Miscellaneous */

    //virtual int typedefHandler(Node *n);

    /* Low-level code generation */

    virtual int constantWrapper(Node *n);
    virtual int variableWrapper(Node *n);
    virtual int functionWrapper(Node *n);
    virtual int nativeWrapper(Node *n);

};

extern "C" Language *
swig_matlab(void) {
    return new MATLAB();
}

String* MATLAB::getEquivalentType(String* st) {
    if(!Strcmp(st,"void")) return NewString("");
    if(!Strcmp(st,"bool")) return NewString("logical");
    if(!Strcmp(st,"byte")) return NewString("int8");
    if(!Strcmp(st,"char") || !Strcmp(st,"signed char")) return NewString("int8");
    if(!Strcmp(st,"unsigned char")) return NewString("uint8");
    if(!Strcmp(st,"short")) return NewString("int16");
    if(!Strcmp(st,"unsigned short")) return NewString("uint16");
    if(!Strcmp(st,"int")) return NewString("int32");
    if(!Strcmp(st,"unsigned int")) return NewString("uint32");
    if(!Strcmp(st,"long")) return NewString("int32");
    if(!Strcmp(st,"unsigned long")) return NewString("uin32");
    if(!Strcmp(st,"long long")) return NewString("int64");
    if(!Strcmp(st,"unsigned long long")) return NewString("uint64");
    if(!Strcmp(st,"float")) return NewString("single");
    if(!Strcmp(st,"double")) return NewString("double");
    return NewString("");
}

String* MATLAB::testEquivalentType(int i, SwigType* st) {
    String* t = SwigType_lstr(st,0);
    String* retVal = NewString("");
    String* arg = NewString("");
    Printf(arg,"varargin{%d}",i);
    // No pointers
    if(!Strcmp(t,"char")) {
        Printf(retVal,"(isa(%s,'int8') || (isa(%s,'char')))",arg,arg);
        Delete(arg);
        Delete(t);
        return retVal; // jeste se pak musi odecist 0, pokud jde o chat
    }
    if(!Strcmp(t,"bool")) {
        Printf(retVal,"(isa(%s,'integer'))",arg);
        Delete(arg);
        Delete(t);
        return retVal;
    }
    if(!Strcmp(t,"byte") || !Strcmp(t,"signed char")|| !Strcmp(t,"unsigned char") || !Strcmp(t,"short") || !Strcmp(t,"unsigned short") ||
       !Strcmp(t,"int") || !Strcmp(t,"unsigned int") || !Strcmp(t,"long") || !Strcmp(t,"unsigned long") ||
       !Strcmp(t,"long long") || !Strcmp(t,"unsigned long long") ||
       !Strcmp(t,"float") || !Strcmp(t,"double")) {
        Printf(retVal,"(isa(%s,'%s') || (isa(%s,'lib.pointer') && strcmp(%s.DataType,'%s')))",arg, getEquivalentType(t),arg,arg,getEquivalentType(t));
        Delete(arg);
        Delete(t);
        return retVal;
    }

    // Get rid of a single possible pointers
    SwigType_del_pointer(st);
    t = SwigType_lstr(st,0);

    // Single pointer
    if(!Strcmp(t,"char")) {
        Printf(retVal,"(isa(%s,'char') || isa(%s,'int8') || (isa(%s,'lib.pointer') && strcmp(%s.DataType,'int8Ptr')))",arg,arg,arg,arg);
        Delete(arg);
        Delete(t);
        return retVal;
    }
    if(!Strcmp(t,"void")) {
        Printf(retVal,"(isa(%s,'lib.pointer') && strcmp(%s.DataType,'voidPtr'))",arg,arg);
        Delete(arg);
        Delete(t);
        return retVal;
    }
    if(!Strcmp(t,"byte") || !Strcmp(t,"signed char")||  !Strcmp(t,"unsigned char")|| !Strcmp(t,"short") || !Strcmp(t,"unsigned short") ||
       !Strcmp(t,"int") || !Strcmp(t,"unsigned int") || !Strcmp(t,"long") || !Strcmp(t,"unsigned long") ||
       !Strcmp(t,"long long") || !Strcmp(t,"unsigned long long") ||
       !Strcmp(t,"float") || !Strcmp(t,"double")) {
        Printf(retVal,"(isa(%s,'%s') || (isa(%s,'lib.pointer') && strcmp(%s.DataType,'%sPtr')))",arg, getEquivalentType(t),arg,arg,getEquivalentType(t));
        Delete(arg);
        Delete(t);
        return retVal;
    }

    // Get rid of all pointers
    while (SwigType_ispointer(st))
        SwigType_del_pointer(st);
    t = SwigType_lstr(st,0);


    // Multiple pointers
    if(!Strcmp(t,"char")) {
        Printf(retVal,"(isa(%s,'char') || isa(%s,'int8') || (isa(%s,'lib.pointer') && strcmp(%s.DataType,'int8PtrPtr')))",arg,arg,arg,arg);
        Delete(arg);
        Delete(t);
        return retVal; // jeste se pak musi odecist 0, pokud jde o chat
    }
    if(!Strcmp(t,"void")) {
        Printf(retVal,"(isa(%s,'lib.pointer') && strcmp(%s.DataType,'voidPtrPtr'))",arg,arg);
        Delete(arg);
        Delete(t);
        return retVal;
    }
 if(!Strcmp(t,"byte") || !Strcmp(t,"signed char")||  !Strcmp(t,"unsigned char")|| !Strcmp(t,"short") || !Strcmp(t,"unsigned short") ||
       !Strcmp(t,"int") || !Strcmp(t,"unsigned int") || !Strcmp(t,"long") || !Strcmp(t,"unsigned long") ||
       !Strcmp(t,"long long") || !Strcmp(t,"unsigned long long") ||
       !Strcmp(t,"float") || !Strcmp(t,"double")) {
        Printf(retVal,"(isa(%s,'%s') || (isa(%s,'lib.pointer') && strcmp(%s.DataType,'%sPtrPtr')))",arg, getEquivalentType(t),arg,arg,getEquivalentType(t));
        Delete(arg);
        Delete(t);
        return retVal;
    }

    // Test for class or struct
    Printf(retVal,"(isa(%s,'%s.%s') || isa(%s,'lib.%s'))",arg,module,t,arg,t);
    Delete(arg);
    Delete(t);
    return retVal;
}

String* MATLAB::testCompatibleType(int i, SwigType *st) {
    String *t = SwigType_lstr(st,0);
    String* retVal = NewString("");
    String* arg = NewString("");
    Printf(arg,"varargin{%d}",i);
    if(!Strcmp(t,"char") || !Strcmp(t,"byte") || !Strcmp(t,"signed char") || !Strcmp(t,"unsigned char") ||
       !Strcmp(t,"short") || !Strcmp(t,"unsigned short") || !Strcmp(t,"int") || !Strcmp(t,"unsigned int") ||
       !Strcmp(t,"long") || !Strcmp(t,"unsigned long") || !Strcmp(t,"long long") || !Strcmp(t,"unsigned long long")) {
        Printf(retVal,"(isa(%s,'integer'))",arg);
        Delete(arg);
        Delete(t);
        return retVal;
    }
    if(!Strcmp(t,"float") || !Strcmp(t,"double")) {
        Printf(retVal,"(isa(%s,'float'))",arg);
        Delete(arg);
        Delete(t);
        return retVal;
    }
    if(!Strcmp(t,"char *")) {
        Printf(retVal,"(isa(%s,'char') || isa(%s,'int8') || (isa(%s,'lib.pointer') && strcmp(%s.DataType,'int8Ptr')))",arg,arg,arg,arg);
        Delete(arg);
        Delete(t);
        return retVal;
    }

    // Get rid of possible pointers
    while (SwigType_ispointer(st))
        SwigType_del_pointer(st);
    t = SwigType_lstr(st,0);

    // Pointers
    // Only values allowed, no pointers with different sizes
    if(!Strcmp(t,"char") || !Strcmp(t,"byte") || !Strcmp(t,"signed char") || !Strcmp(t,"unsigned char") ||
       !Strcmp(t,"short") || !Strcmp(t,"unsigned short") || !Strcmp(t,"int") || !Strcmp(t,"unsigned int") ||
       !Strcmp(t,"long") || !Strcmp(t,"unsigned long") || !Strcmp(t,"long long") || !Strcmp(t,"unsigned long long")) {
        Printf(retVal,"(isa(%s,'integer'))",arg);
        Delete(arg);
        Delete(t);
        return retVal;
    }
    if(!Strcmp(t,"float") || !Strcmp(t,"double")) {
        Printf(retVal,"(isa(%s,'float'))",arg);
        Delete(arg);
        Delete(t);
        return retVal;
    }
    // Test for class or struct
    Printf(retVal,"(isa(%s,'%s.%s') || isa(%s,'lib.%s'))",arg,module,t,arg,t);
    Delete(arg);
    Delete(t);
    return retVal;
}

String *MATLAB::WriteMString(Node *n) {
    Node *helpNode=n;
    while(Getattr(helpNode,"sym:previousSibling"))
        helpNode=Getattr(helpNode,"sym:previousSibling");

    String *mCode=NewString("");

    //Printf(mCode,"\n\nname=%s\nsym:name=%s\nwrap:name=%s\n\n", Getattr(helpNode,"name"), Getattr(helpNode,"sym:name"), Getattr(helpNode,"wrap:name"));

    if(constructor_flag || copyconstructor_flag) {
        Printf(mCode,"function [this] = %s(varargin)\n",Getattr(helpNode,"sym:name"));
    } else if(destructor_flag) {
        Printf(mCode,"function delete(varargin)\n");
    } else {
        Printf(mCode,"function [retVal] = %s(varargin)\n",Getattr(helpNode,"sym:name"));
    }
    Printf(mCode,"    if ~libisloaded('lib%s')\n",module);
    Printf(mCode,"        error('Library lib%s is not loaded. Call %s.load()');\n",module, module);
    Printf(mCode,"    end\n",NIL);
    Printf(mCode,"    switch nargin\n",NIL);

    /*** Solving overloads ***/
    int lastNumargs = -1; // impossible -1 to test first case
    do {
        /*** Preparing library call ***/
        String* libraryCall = NewString("                ");

        /*** Getting return type ***/
        String *matlabRetType;
        bool isStructOrObjectRetType = false;
        // constructor
        if(constructor_flag || copyconstructor_flag) {
#ifdef MATLABDEBUGDT
            Printf(stderr,"DT: constructor\n");
#endif
            Printf(libraryCall,"this.pointer=");
        // destructor
        } else if(destructor_flag) {
#ifdef MATLABDEBUGDT
            Printf(stderr,"DT: destructor\n");
#endif
            // only call the library
        // function
        } else {
            SwigType* returnType = Getattr(helpNode,"type");
            // removing pointers
            bool isVoidRetType = !Strcmp(SwigType_lstr(returnType,0),"void");
            while (SwigType_ispointer(returnType))
                SwigType_del_pointer(returnType);
            String *retTypeStr = SwigType_lstr(returnType,0);
#ifdef MATLABDEBUGDT
            Printf(stderr,"DT OUT: %s\n",retTypeStr);
#endif
            matlabRetType = getEquivalentType(retTypeStr);
            if(!isVoidRetType) {
                if(Strcmp(matlabRetType,"")) {
                    Printf(libraryCall,"retVal=%s",matlabRetType);
                } else {
                    // struct or object
                    isStructOrObjectRetType = true;
                    Printf(libraryCall,"helpVal=");
                }
            //Delete(retTypeStr);
            }
        }
        /*** Getting input arguments types ***/
        ParmList *parms = Getattr(helpNode,"parms");
        int numargs = emit_num_arguments(parms);

        /*** Preparing library call***/
        Printf(libraryCall,"(calllib('lib%s','%s'",module,Getattr(helpNode,"wrap:name"));
        for(int i=1; i<=numargs; i++)
            Printf(libraryCall,",varargin{%d}%s",i,((i==1 && (memberfunction_flag || destructor_flag || copyconstructor_flag))?".pointer":""));
        Printf(libraryCall,"));\n"); //chtelo by to navazat na typemapy pro tu kontrolu typu
        if(isStructOrObjectRetType) {
            Printf(libraryCall,"                try\n");
            Printf(libraryCall,"                    if isa(helpVal.Value,'struct')\n");
            Printf(libraryCall,"                        retVal=helpVal.Value;\n");
            Printf(libraryCall,"                    end\n");
            Printf(libraryCall,"                catch\n");
            Printf(libraryCall,"                    retVal=%s(PointerForMatlabConstructor(helpVal));\n",matlabRetType);
            Printf(libraryCall,"                end\n");
        }
        /*** Testing input argumets ***/
        // special case without input argumets
        if (numargs == 0) {
            Printf(mCode,"        case 0\n");
            Printf(mCode,"%s",libraryCall);
            lastNumargs = numargs;
            continue;
        }

        // numargs >= 1
        // special class constructor
        if (constructor_flag && lastNumargs < 1) {
            Printf(mCode,"        case 1\n");
            Printf(mCode,"            if isa(varargin{1},'PointerForMatlabConstructor')\n");
            Printf(mCode,"                this.pointer = varargin{1}.pointer;\n");
            lastNumargs = 1;
        }

        if (lastNumargs != numargs) {
            if (lastNumargs != -1) {
                Printf(mCode,"            else\n");
                Printf(mCode,"                error('Illegal arguments');\n");
                Printf(mCode,"            end\n");
            }
            Printf(mCode,"        case %d\n",numargs);
            Printf(mCode,"            if ");
        } else {
            Printf(mCode,"            elseif ");
        }

        /*** Testing input argumets ***/
        Parm *p;
        int i = 1;
        String* charConversion = NewString("");
        String* testCompatible = NewString("            elseif ");
        for (i = 1, p = parms; i <= numargs; i++) {
            SwigType *pt = Getattr(p,"type");
            // resolve all typedefs
            SwigType *ptr = SwigType_typedef_resolve_all(pt);
            // get a C-like type
            String *typeStr = SwigType_lstr(ptr,0);
#ifdef MATLABDEBUGDT
            Printf(stderr,"DT IN: %s\n",typeStr);
#endif
            // generate a Matlab test for the type
            String* matlabTest = testEquivalentType(i, ptr);
            Printf(mCode,"%s%s",(i>1?" && ":""),matlabTest);
            String* matlabCompatibleTest = testCompatibleType(i, ptr);
            Printf(testCompatible,"%s%s",(i>1?" && ":""),matlabCompatibleTest);
            // Char conversion from characters to ASCII codes
            if(!Strcmp(typeStr,"char")) {
                Printf(charConversion,"                if isa(varargin{%d},'char')\n",i);
                Printf(charConversion,"                    varargin{%d} = int8(varargin{%d});\n",i,i);
                Printf(charConversion,"                end\n");
            }
            Delete(matlabCompatibleTest);
            Delete(matlabTest);
            //Delete(typeStr);
            p = nextSibling(p);
        }
        Printf(mCode,"\n");
        Printf(mCode,"%s",charConversion);
        Printf(mCode,"%s",libraryCall);
        Printf(mCode,"%s\n",testCompatible);
        Printf(mCode,"                warning('LibraryUI:TypesMismatched','Possible data loss.');\n");
        Printf(mCode,"%s",libraryCall);

        lastNumargs = numargs;
        Delete(testCompatible);
        Delete(charConversion);
        Delete(libraryCall);
        //Delete(matlabRetType);
    } while (helpNode=Getattr(helpNode,"sym:nextSibling"));

    if(lastNumargs != 0) {
        Printf(mCode,"            else\n");
        Printf(mCode,"                error('Illegal arguments.');\n");
        Printf(mCode,"            end\n");
    }
    Printf(mCode,"        otherwise\n",NIL);
    Printf(mCode,"            error('Wrong number of arguments.');\n",NIL);
    Printf(mCode,"    end\n",NIL);
    Printf(mCode,"end\n",NIL);

    return mCode;
}


String *MATLAB::getOverloadedName(Node *n) {
    String *overloaded_name = NewStringf("%s", Getattr(n, "sym:name"));
    if (Getattr(n, "sym:overloaded")) {
        Printv(overloaded_name, Getattr(n, "sym:overname"), NIL);
    }
    return overloaded_name;
}



void MATLAB::emitGetterSetter(Node *n, String *wname, ParmList *parms, Wrapper *wrapper, bool is_getter) {
    // Write the required arguments
    emit_parameter_variables(parms, wrapper);

    // Attach the standard typemaps
    emit_attach_parmmaps(parms, wrapper);
    Setattr(n, "wrap:parms", parms);

    // Get number of arguments
    int numarg = emit_num_arguments(parms);
    //int numreq = emit_num_required(parms);

    if(!is_getter) {  // marshal input argument
        String *tm;
        Parm *p;
        int i = 0;
        for (i = 0, p = parms; i < numarg; i++) {
            SwigType *pt = Getattr(p, "type");

            if ((tm = Getattr(p, "tmap:in"))) {   	// Get typemap for this argument
                Replaceall(tm, "$input", "value");
                Setattr(p, "emit:input", "value");
                Printf(wrapper->code, "%s\n", tm);
            } else {
                Swig_warning(WARN_TYPEMAP_IN_UNDEF, input_file, line_number, "Unable to use type %s as a function argument.\n", SwigType_str(pt, 0));
                p = nextSibling(p);
            }
        }
    }

    // Write constraints

    // Emit the action
    Setattr(n, "wrap:name", wname);
    String *actioncode = emit_action(n);


#ifdef MATLABDEBUG
    Printf(stderr,"-------------\nwrap:name=%s\naction=%s\n------------------\n",wname,actioncode);
#endif

    // Write typemaps(out)
    marshalOutput(n, actioncode, wrapper);
}

const String *MATLAB::typemapLookup(Node *n, const_String_or_char_ptr tmap_method, SwigType *type, int warning, Node *typemap_attributes=0) {
    Node *node = !typemap_attributes ? NewHash() : typemap_attributes;
    Setattr(node, "type", type);
    Setfile(node, Getfile(n));
    Setline(node, Getline(n));
    const String *tm = Swig_typemap_lookup(tmap_method, node, "", 0);
    if (!tm) {
        tm = NewString("");
        if (warning != WARN_NONE)
            Swig_warning(warning, Getfile(n), Getline(n), "No %s typemap defined for %s\n", tmap_method, SwigType_str(type, 0));
    }
    if (!typemap_attributes)
        Delete(node);
    return tm;
}


void MATLAB::marshalInputArgs(ParmList *parms, int numarg, Wrapper *wrapper) {
    String *tm;
    Parm *p;
    int i = 0;
    for (i = 0, p = parms; i < numarg; i++) {
        SwigType *pt = Getattr(p, "type");

        String *arg = NewString("");

        if (tm = Getattr(p, "tmap:in")) {   	// Get typemap for this argument
            Replaceall(tm, "$input", arg);
            Setattr(p, "emit:input", arg);
            Printf(wrapper->code, "%s\n", tm);
            p = Getattr(p, "tmap:in:next");
        } else {
            Swig_warning(WARN_TYPEMAP_IN_UNDEF, input_file, line_number, "Unable to use type %s as a function argument.\n", SwigType_str(pt, 0));
            p = nextSibling(p);
        }
        Delete(arg);
    }
}


void MATLAB::marshalOutput(Node *n, String *actioncode, Wrapper *wrapper) {
    SwigType *type = Getattr(n, "type");
    Setattr(n, "type", type);
    String *tm;
    if ((tm = Swig_typemap_lookup_out("out", n, "result", wrapper, actioncode))) {
        //   Replaceall(tm, "$result", "jsresult");
        // TODO: May not be the correct way
        Replaceall(tm, "$objecttype", Swig_scopename_last(SwigType_str(SwigType_strip_qualifiers(type), 0)));
        Printf(wrapper->code, "%s", tm);
        if (Len(tm))
            Printf(wrapper->code, "\n");
    } else {
        Swig_warning(WARN_TYPEMAP_OUT_UNDEF, input_file, line_number, "Unable to use return type %s in function %s.\n", SwigType_str(type, 0), Getattr(n, "name"));
    }
    emit_return_variable(n, type, wrapper);
}


int MATLAB::top(Node *n) {

    /* Get the module name */
    module = Getattr(n,"name");

    /* Get c++ wrapper output file name */
    String *outfile = Getattr(n,"outfile");

    /* Get .h wrapper header file name*/
    hfile = Getattr(n,"outfile_h");


    /* Creating package directory */
    String* empty=NewString("");
    packageDirname=NewString("+");
    Append(packageDirname,module);
    Swig_new_subdirectory(empty,packageDirname);

    /* Initialize I/O (see next section) */
    f_begin = NewFile(outfile, "w", SWIG_output_files());
    if (!f_begin) {
        FileErrorDisplay(outfile);
        SWIG_exit(EXIT_FAILURE);
    }

    /* init header file */
    f_wrap_h = NewFile(hfile, "w", SWIG_output_files());
    if (!f_wrap_h) {
        FileErrorDisplay(hfile);
        SWIG_exit(EXIT_FAILURE);
    }

    String *loadFileName = NewString(packageDirname);
    Append(loadFileName,"/load.m");

    f_loadm = NewFile(loadFileName, "w", SWIG_output_files());
    if (!f_loadm) {
        FileErrorDisplay(loadFileName);
        SWIG_exit(EXIT_FAILURE);
    }

    /* print the load.m file */
    String *loadmCode = NewString("") ;
    Printv(loadmCode, "function load()\n", NIL);
    Printv(loadmCode, "    if libisloaded('libLIBNAME')\n", NIL);
    Printv(loadmCode, "        unloadlibrary('libLIBNAME');\n", NIL);
    Printv(loadmCode, "    end\n", NIL);
    Printv(loadmCode, "    loadlibrary('libLIBNAME','HNAME');\n", NIL);
    Replaceall(loadmCode,"LIBNAME",module);
    Replaceall(loadmCode,"HNAME",hfile);
    Dump(loadmCode,f_loadm);
    Close(loadFileName);
    Delete(loadmCode);
    Delete(loadFileName);

    String *unloadFileName = NewString(packageDirname);
    Append(unloadFileName,"/unload.m");

    f_unloadm = NewFile(unloadFileName, "w", SWIG_output_files());
    if (!f_unloadm) {
        FileErrorDisplay(unloadFileName);
        SWIG_exit(EXIT_FAILURE);
    }

    /* print the unload.m file */
    String *unloadmCode = NewString("") ;
    Printv(unloadmCode, "function unload()\n", NIL);
    Printv(unloadmCode, "    if libisloaded('libLIBNAME')\n", NIL);
    Printv(unloadmCode, "        unloadlibrary('libLIBNAME');\n", NIL);
    Printv(unloadmCode, "    end;\n", NIL);
    Replaceall(unloadmCode,"LIBNAME",module);
    Dump(unloadmCode,f_unloadm);
    Close(unloadFileName);
    Delete(unloadmCode);
    Delete(unloadFileName);

    /* set function prefix */
    String *functionPrefix = NewString("");
    Printf(functionPrefix,"matlab_%s_",module);
    Append(functionPrefix,"%f");
    Swig_name_register("wrapper", functionPrefix);


    wrap_h_code = NewString("");

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

    /* write .h file */

    //no dump for header, need to redefine classes to void *
    //Dump(f_header,f_wrap_h); //add interface include code
    Replaceall(wrap_h_code,"&","*");
    Dump(wrap_h_code, f_wrap_h);

    /* write .c file */
    Dump(f_runtime, f_begin);
    Dump(f_header, f_begin);
    Dump(f_wrappers, f_begin);
    Wrapper_pretty_print(f_init, f_begin);

    /* Cleanup files */
    Close(f_wrap_h);
    Delete(f_wrap_h);
    Delete(f_runtime);
    Delete(f_header);
    Delete(f_wrappers);
    Delete(f_init);
    Close(f_begin);
    Delete(f_begin);

    return SWIG_OK;
}

/*
   this function names unnamed parameters

   Security by obscurity! If user chooses swig_par_name_2 as a parameter name
   for the first parameter and does not name the second one, boom.
*/
void nameUnnamedParams(ParmList *parms, bool all) {
    Parm *p;
    int i;
    for (p = parms, i=1; p; p = nextSibling(p),i++) {
        if(all || !Getattr(p,"name")) {
            String* parname=NewStringf("swig_par_name_%d", i);
            Setattr(p,"name",parname);
        }

    }
}

void renameParams(ParmList *parms) {
    nameUnnamedParams(parms, true);
}

int MATLAB::functionWrapper(Node *n) {
    /* get useful atributes */
    String   *name   = Getattr(n,"sym:name");
    SwigType *type   = Getattr(n,"type");
    ParmList *parms  = Getattr(n,"parms");

    /* handle nameless parameters */
    nameUnnamedParams(parms, false);

    //String *scopeNamePrefix = NewString(Swig_scopename_prefix(Getattr(n,"name")));
    //Printf(stderr,"name=%s :::: %s\n", name, scopeNamePrefix);
    //Append(scopeNamePrefix, "::");

    String *parmprotostr = ParmList_protostr(parms);

    /*do not wrap pure virtual*/
    String *value = Getattr(n, "value");
    String *storage = Getattr(n, "storage");
    bool pure_virtual = false;
    if (Cmp(storage, "virtual") == 0) {
        if (Cmp(value, "0") == 0) {
            pure_virtual = true;
        }
    }

    /* this treats overloaded name */
    String *overname = NewString("");

    Printf(overname, "%s", getOverloadedName(n));

    String *wname = Swig_name_wrapper(overname);

    Setattr(n, "wrap:name", wname);

    /* create the wrapper object */
    Wrapper *wrapper = NewWrapper();

    bool is_getter = (Cmp(wname, Swig_name_set(getNSpace(), variable_name)) != 0);

    // Do nothing for pure virtual functions
    if(pure_virtual) return SWIG_OK;

    if(copyconstructor_flag);//deal with constructor

    if (constructor_flag); //deal with constructor

    if (destructor_flag); //deal with destructor

    if(globalvariable_flag) { //deal with global variables

        //need to rename parms so that the parm has different name than the variable
        renameParams(parms);
    }

    if(memberconstant_flag);//deal with class constants

    if(memberfunction_flag);//deal with class functions

    if(constantWrapper_flag);//deal with constant

    if(membervariable_flag);//deal with membervariable

    if(staticmemberfunction_flag);//deal with static class function

    if(staticmembervariable_flag);//deal with static class variable

    //TODO how is this different from global variable
    if(variable_flag); //deal with variable

    /* write h-file declaration; Matlab requires .h file to load library */
    Printv(wrap_h_code, SwigType_str(type,0), " ", wname, "(", parmprotostr, ");\n",NIL);
    //Printv(wrap_h_code, Getattr(n,"type"), " ", wname, "(", parmprotostr, ");\n",NIL);

    String *parmstr  = ParmList_str_defaultargs(parms);

    /* write the wrapper function definition */
    Printv(wrapper->def, "\n#ifdef __cplusplus\nextern \"C\"\n#endif\nSWIGEXPORT ", SwigType_str(type,0), " ", wname, "(", parmstr, ") {",NIL);

    /* if any additional local variable needed, add them now */
    // TODO - do we need this?

    /* write the list of locals/arguments required */
    emit_parameter_variables(parms,wrapper);

    /* check arguments */

    /* attach parmmaps*/
    emit_attach_parmmaps(parms, wrapper);
    Setattr(n, "wrap:parms", parms);

    // Get number of arguments
    int numarg = emit_num_arguments(parms);

    /* write typemaps(in) */
    /* the same with this? */
    marshalInputArgs(parms, numarg, wrapper);

    /* write constriants */


    /* Emit the function call */
    String *actioncode = emit_action(n);
    //Printf(wrapper->code, "%s\n", actioncode);//TODO not really helpful
    //emit_action_code(n, actioncode, wrapper);

    marshalOutput(n, actioncode, wrapper);
    //emit_return_variable(n,type,wrapper);

    /* return value if necessary  */

    /* write typemaps(out) */


    /* add cleanup code */

    /* add the failure cleanup code */


    /* Close the function */
    Printv(wrapper->code, "}\n", NIL);

    /* final substititions if applicable */


    /* Dump the function out */
    Wrapper_print(wrapper,f_wrappers);

    /*generate .m files*/
    if(!Getattr(n, "sym:nextSibling")) {
        //Printf(stderr,"name=%s\n",wname);
        if(classhandler_flag) {
            String* mClassFunc = WriteMString(n);
            Append(mClass,mClassFunc);
        } else { //regular function

            mFunction=NewString("");
            mFunction=WriteMString(n);
            String *mFunction_fileName=NewString(packageDirname);
            Append(mFunction_fileName,"/");
            if(Getattr(n,"feature:nspace")) {
                String* functionFullName = NewString(Getattr(n,"name"));
                String* functionNamespaces = Swig_scopename_prefix(functionFullName);
                List* namespaceList = Split(functionNamespaces, ':', -1);
                String* empty = NewString("");
                Iterator nLit;
                for(nLit=First(namespaceList); nLit.item; nLit=Next(nLit)) {
                    String* singleNamespace = nLit.item;
                    Printf(mFunction_fileName,"+%s/",singleNamespace);
                    Swig_new_subdirectory(empty,mFunction_fileName);
                }
                Delete(empty);
                Delete(namespaceList);
                Delete(functionNamespaces);
                Delete(functionFullName);
            }
            Append(mFunction_fileName,name);
            Append(mFunction_fileName,".m");

            File *mFunction_file=NewFile(mFunction_fileName,"w",SWIG_output_files());
            if (!mFunction_file) {
                FileErrorDisplay(mFunction_fileName);
                SWIG_exit(EXIT_FAILURE);
            }

            Dump(mFunction,mFunction_file);
            Close(mFunction_file);
            Delete(mFunction);
            Delete(mFunction_fileName);
        }

    }





    /* tidy up */
    Delete(wname);
    Delete(parmstr);
    DelWrapper(wrapper);

    return SWIG_OK;
}






int MATLAB::functionHandler(Node *n) {
#ifdef MATLABPRINTFUNCTIONENTRY
    Printf(stderr,"Entering functionHandler\n");
#endif
    Language::functionHandler(n);
    return SWIG_OK;
}

int MATLAB::globalfunctionHandler(Node *n) {
#ifdef MATLABPRINTFUNCTIONENTRY
    Printf(stderr,"Entering globalfunctionHandler\n");
#endif
    Language::globalfunctionHandler(n);
    return SWIG_OK;
}


int MATLAB::memberfunctionHandler(Node *n) {
    memberfunction_flag=true;
#ifdef MATLABPRINTFUNCTIONENTRY
    Printf(stderr,"Entering memberfunctionHandler\n");
#endif

    Language::memberfunctionHandler(n);

    memberfunction_flag=false;
    return SWIG_OK;
}


int MATLAB::staticmemberfunctionHandler(Node *n) {
    staticmemberfunction_flag=true;
#ifdef MATLABPRINTFUNCTIONENTRY
    Printf(stderr,"Entering staticmemberfunctionHandler\n");
#endif
    Language::staticmemberfunctionHandler(n);
    staticmemberfunction_flag=false;
    return SWIG_OK;
}


//unsure of what this does so far
int MATLAB::callbackfunctionHandler(Node *n) {
#ifdef MATLABPRINTFUNCTIONENTRY
    Printf(stderr,"Entering callbackfunctionHandler\n");
#endif
    Language::callbackfunctionHandler(n);
    return SWIG_OK;
}


int MATLAB::variableHandler(Node *n) {
    variable_flag=true;
#ifdef MATLABPRINTFUNCTIONENTRY
    Printf(stderr,"Entering variableHandler\n");
#endif
    Language::variableHandler(n);
    variable_flag=false;
    return SWIG_OK;
}


int MATLAB::globalvariableHandler(Node *n) {
    globalvariable_flag = true;
#ifdef MATLABPRINTFUNCTIONENTRY
    Printf(stderr,"Entering globalvariableHandler\n");
#endif
    Language::globalvariableHandler(n);
    globalvariable_flag = false;
    return SWIG_OK;
}





int MATLAB::membervariableHandler(Node *n) {
    membervariable_flag=true;
#ifdef MATLABPRINTFUNCTIONENTRY
    Printf(stderr,"Entering membervariableHandler\n");
#endif
    Language::membervariableHandler(n);
    membervariable_flag=false;
    return SWIG_OK;
}





int MATLAB::staticmembervariableHandler(Node *n) {
    staticmembervariable_flag=true;
#ifdef MATLABPRINTFUNCTIONENTRY
    Printf(stderr,"Entering staticmembervariableHandler\n");
#endif
    Language::staticmembervariableHandler(n);
    staticmembervariable_flag=false;
    return SWIG_OK;
}





int MATLAB::memberconstantHandler(Node *n) {
    memberconstant_flag=true;
#ifdef MATLABPRINTFUNCTIONENTRY
    Printf(stderr,"Entering memberconstantHandler\n");
#endif
    Language::memberconstantHandler(n);
    memberconstant_flag=false;
    return SWIG_OK;
}





int MATLAB::constructorHandler(Node *n) {
    constructor_flag=true;
#ifdef MATLABPRINTFUNCTIONENTRY
    Printf(stderr,"Entering constructorHandler\n");
#endif
    Language::constructorHandler(n);
    constructor_flag=false;
    return SWIG_OK;
}





int MATLAB::copyconstructorHandler(Node *n) {
    copyconstructor_flag=true;
#ifdef MATLABPRINTFUNCTIONENTRY
    Printf(stderr,"Entering copyconstructorHandler\n");
#endif
    Language::copyconstructorHandler(n);
    copyconstructor_flag=false;
    return SWIG_OK;
}





int MATLAB::destructorHandler(Node *n) {
    destructor_flag=true;
#ifdef MATLABPRINTFUNCTIONENTRY
    Printf(stderr,"Entering destructorHandler\n");
#endif
    Language::destructorHandler(n);
    destructor_flag=false;
    return SWIG_OK;
}





int MATLAB::classHandler(Node *n) {
#ifdef MATLABPRINTFUNCTIONENTRY
    Printf(stderr,"Entering classHandler\n");
#endif

    Printf(wrap_h_code,"typedef void * %s;\n", Getattr(n,"sym:name"));

    classhandler_flag=true;

    mClass=NewString("\n");

    Language::classHandler(n);

    // Add extra indentation
    Replaceall(mClass, "\n", "\n        ");
    Replaceall(mClass, "        \n", "\n");

    Printf(mClass, "\n    end\n");
    Printf(mClass, "\nend\n");

    String * mClass_fileName=NewString(packageDirname);
    Append(mClass_fileName,"/");
    Append(mClass_fileName,Getattr(n,"sym:name"));
    Append(mClass_fileName,".m");
//create the .m file
    File *mClass_file=NewFile(mClass_fileName,"w",SWIG_output_files());
    if (!mClass_file) {
        FileErrorDisplay(mClass_fileName);
        SWIG_exit(EXIT_FAILURE);
    }

    String *mClassHead = NewString("");

    Printf(mClassHead, "classdef %s < handle\n\n", Getattr(n,"sym:name"));
    Printf(mClassHead, "    properties (GetAccess = public, SetAccess = private)\n");
    Printf(mClassHead, "        pointer\n");
    Printf(mClassHead, "    end\n\n");
    Printf(mClassHead, "    methods\n"); //no indent - will be indented afterwards

    Dump(mClassHead,mClass_file);
    Delete(mClassHead);

    //write mClass to the .m file
    Dump(mClass,mClass_file);
    Close(mClass_file);
    Delete(mClass);
    Delete(mClass_fileName);


    classhandler_flag=false;

    return SWIG_OK;
}



int MATLAB::constantWrapper(Node *n) {
    constantWrapper_flag=true;
#ifdef MATLABPRINTFUNCTIONENTRY
    Printf(stderr,"Entering constantWrapper\n");
#endif
    Language::constantWrapper(n);
    constantWrapper_flag=false;
    return SWIG_OK;
}





int MATLAB::variableWrapper(Node *n) {
    variable_flag=true;
#ifdef MATLABPRINTFUNCTIONENTRY
    Printf(stderr,"Entering variableWrapper\n");
#endif
    Language::variableWrapper(n);
    variable_flag=false;
    return SWIG_OK;
}





int MATLAB::nativeWrapper(Node *n) {
#ifdef MATLABPRINTFUNCTIONENTRY
    Printf(stderr,"Entering nativeWrapper\n");
#endif
    Language::nativeWrapper(n);
    return SWIG_OK;
}
