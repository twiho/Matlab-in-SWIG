
/* Typemaps for exact matlab data type */

%typemap(mexact) bool                                                 "logical"
%typemap(mexact) byte                                                 "int8"
%typemap(mexact) char, signed char                                    "int8"
%typemap(mexact) unsigned char                                        "uint8"
%typemap(mexact) short, signed short                                  "int16"
%typemap(mexact) unsigned short                                       "uint16"
%typemap(mexact) int, signed, signed int                              "int32"
%typemap(mexact) unsigned int                                         "uint32"
%typemap(mexact) long, signed long                                    "int32"
%typemap(mexact) unsigned long                                        "uint32"
%typemap(mexact) long long, signed long long                          "int64"
%typemap(mexact) unsigned long long                                   "uint64"
%typemap(mexact) float                                                "single"
%typemap(mexact) double                                               "double"

/* Typemaps for compatible matlab data type */

%typemap(mcompatible) bool                                            "logical"
%typemap(mcompatible) byte, char, signed char, unsigned char,
                     short, signed short, unsigned short,
                     int, signed, signed int, unsigned int,
                     long, signed long, unsigned long,
                     long long, signed long long, unsigned long long  "integer"
%typemap(mcompatible) float, double                                   "float"

/* Typemaps for free matlab data type */

%typemap(mfree) bool,
                byte, char, signed char, unsigned char,
                short, signed short, unsigned short,
                int, signed, signed int, unsigned int,
                long, signed long, unsigned long,
                long long, signed long long, unsigned long long, 
                float, double                                         "numeric"
