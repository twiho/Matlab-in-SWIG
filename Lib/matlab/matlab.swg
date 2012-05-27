//%include <typemaps/swigmacros.swg>
//%include <typemaps/fragments.swg>
//%include <matlabuserdir.swg>
//%include <matlabtypemaps.swg>


%include <matlabopers.swg>


%include <matlabruntime.swg>

%define %docstring %feature("docstring") %enddef //what?

%typemap(in) char,
             signed char,
             unsigned char,
		     short,
             unsigned short,
             int,
             unsigned int,
             long,
             unsigned long,
             long long,
             unsigned long long,
             float,
             double,
             short *,
             unsigned short *,
             int *,
             unsigned int *,
             long *,
             unsigned long *,
             long long *,
             unsigned long long *,
             float *,
             double *,
             int *OUTPUT,
             SWIGTYPE *
%{
 //typemap in
 $1 = ($1_ltype) $1_name;
%}

%typemap(out) char,
             signed char,
             unsigned char,
		     short,
             unsigned short,
             int,
             unsigned int,
             long,
             unsigned long,
             long long,
             unsigned long long,
             float,
             double,
             char *,
             SWIGTYPE
%{
  //typemap out
  return $1;
%}

%typemap(out) SWIGTYPE &%{
 return *$1;
%}


%typemap(out) void %{
%}

%typemap(arginit) SWIGTYPE %{
 //typemap arginit swigtype
%}

/*
%typemap(except) SWIGTYPE %{
 //typemap except swigtype
%}
*/

%typemap(freearg) SWIGTYPE %{
 //typemap freearg SWIGTYPE
%}

%typemap(check) SWIGTYPE %{
 //typemap check SWIGTYPE
%}


%typemap(typecheck) SWIGTYPE %{
 //typemap typecheck SWIGTYPE
%}

%typemap(argout) SWIGTYPE,
                 SWIGTYPE *,
                 SWIGTYPE &%{
 //typemap argout SWIGTYPE
%}

%typemap(typecheck) SWIGTYPE %{
 //typemap typecheck SWIGTYPE
%}

%typemap(globalin) SWIGTYPE %{
 $1=$input;
%}

%typemap(loadCode) SWIGTYPE
%{
function load()
      if libisloaded('Pair_module')
            unloadlibrary('Pair_module');
      end
      loadlibrary('Pair_module','Pair_module.h');
%}
