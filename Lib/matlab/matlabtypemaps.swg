
/* Typemaps for exact matlab data type */

%typemap(exact) bool                                                  "logical"
%typemap(exact) byte                                                  "int8"
%typemap(exact) char, signed char                                     "int8"
%typemap(exact) unsigned char                                         "uint8" 
%typemap(exact) short, signed short                                   "int16"
%typemap(exact) unsigned short                                        "uint16"
%typemap(exact) int, signed, signed int                               "int32"
%typemap(exact) unsigned int                                          "uint32"
%typemap(exact) long, signed long                                     "int32"
%typemap(exact) unsigned long                                         "uint32" 
%typemap(exact) long long, signed long long                           "int64"
%typemap(exact) unsigned long long                                    "uint64"
%typemap(exact) float                                                 "single"
%typemap(exact) double                                                "double"

/* Typemaps for compatible matlab data type */

%typemap(compatible) bool                                             "logical"
%typemap(compatible) char, signed char, unsigned char,
                     short, signed short, unsigned short,
                     int, signed, signed int, unsigned int,
                     long, signed long, unsigned long,
                     long long, signed long long, unsigned long long  "integer" 
%typemap(compatible) float, double                                    "float"
