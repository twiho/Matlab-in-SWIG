/* ----------------------------------------------------------------------------------------------
 * javascriptcode.swg
 *
 * Additional Javascript typemaps for generating code for classes, constants and variables
 * ----------------------------------------------------------------------------- ----------------*/

/* Additional typemaps for the  class code needed by the access layer. This code is copy-pasted into the wrapper header and source file */
%typemap (getpropertydecl) SWIGTYPE
%{
JSValueRef $getname(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)%}

%typemap (setpropertydecl) SWIGTYPE
%{
bool $setname(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef value, 
JSValueRef* exception)%}

%typemap (functiondecl) SWIGTYPE
%{
JSValueRef $functionname(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argc, const JSValueRef argv[], JSValueRef* exception)%}




%typemap (accessconstructordecl) SWIGTYPE 
%{
void $jsclassname_initialize(JSContextRef context, JSObjectRef thisObject)%}

%typemap (accessconstructorbody) SWIGTYPE
{
}

%typemap (accessdestructordecl) SWIGTYPE 
%{
void $jsclassname_finalize(JSObjectRef thisObject)%}

%typemap (accessdestructorbody) SWIGTYPE
{
	SWIG_PRV_DATA* t = (SWIG_PRV_DATA*)JSObjectGetPrivate(thisObject);
	if(t && t->swigCMemOwn) delete ($objecttype*)(t->swigCObject);
	if(t) delete t;
}

%typemap (accessvariablesdefn) SWIGTYPE
%{
JSStaticValue $jsclassname_staticValues[] = {
$jsstaticvaluescode
    { 0, 0, 0, 0 }
};

JSStaticFunction $jsclassname_staticFunctions[] = {
$jsstaticfuncscode
    { 0, 0, 0 }
};
%}

%typemap(accessfunctionsdecl) SWIGTYPE 
%{
bool $jsclassname_initClass(JSGlobalContextRef& context, JSObjectRef& parentObject, const char* className);
JSClassRef $jsclassname_createJSClass(JSContextRef context);
JSObjectRef $jsclassname_createcppObject(JSContextRef context, JSObjectRef thisObject, size_t argc, const JSValueRef argv[],
JSValueRef* exception);
%}

%typemap(accessfunctionsdefn) SWIGTYPE
%{ 
JSClassRef $jsclassname_createJSClass(JSContextRef context)
{
	static JSClassRef jsClass = NULL;
	if (!jsClass)
	{
		JSClassDefinition definition	= kJSClassDefinitionEmpty;
		definition.staticValues			= $jsclassname_staticValues;
		definition.staticFunctions		= $jsclassname_staticFunctions;
		definition.initialize			= $jsclassname_initialize;
		definition.finalize				= $jsclassname_finalize;
		jsClass 						= JSClassCreate(&definition);
	}
	return jsClass;
}

JSObjectRef $jsclassname_createcppObject(JSContextRef context, JSObjectRef thisObject, size_t argc, const JSValueRef argv[],
JSValueRef* exception)
{
  JSObjectRef jsresult;	
  $objecttype result = 0;
  $jscreatecppobjectcode
  SWIG_PRV_DATA *swigprivatedata = new SWIG_PRV_DATA();
  swigprivatedata->swigCMemOwn = true;
  swigprivatedata->swigCObject = result;
  	
  jsresult = JSObjectMake(context, $jsclassname_createJSClass(context), swigprivatedata);
  return jsresult;
}

bool $jsclassname_initClass(JSGlobalContextRef& context, JSObjectRef& parentObject, const char* className)
{
    JSStringRef jsstring = JSStringCreateWithUTF8CString(className);
	JSObjectSetProperty(context, parentObject, jsstring, 
						JSObjectMakeConstructor(context,
						$jsclassname_createJSClass(context),
						$jsclassname_createcppObject), 
						kJSPropertyAttributeNone, 
						NULL);
    JSStringRelease(jsstring);
    return true;
} 
%}


/* Additional typemaps needed for generating the code for handling constants as properties of the cconst object */
%typemap (constantdecl) SWIGTYPE
%{JSValueRef $constantgetname(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)%}

%typemap (constantbody) SWIGTYPE
%{
    SWIGUNUSEDPARM(thisObject);
    SWIGUNUSEDPARM(exception);
    JSValueRef jsresult;
%}

%typemap (cconstdecl) SWIGTYPE 
%{
bool cconst_initClass(JSGlobalContextRef& context, JSObjectRef& parentObject, const char* constclassname);
%}

%typemap (cconstdefn) SWIGTYPE
%{

JSStaticValue cconst_staticValues[] = {
$jsstaticconstscode
    { 0, 0, 0, 0 }
};

JSClassRef cconst_createJSClass(JSContextRef context)
{
	static JSClassRef jsClass = NULL;
	if (!jsClass)
	{
		JSClassDefinition definition	= kJSClassDefinitionEmpty;
		definition.staticValues			= cconst_staticValues;
		jsClass 						= JSClassCreate(&definition);
	}
	return jsClass;
}

bool cconst_initClass(JSGlobalContextRef& context, JSObjectRef& parentObject, const char* constclassname)
{
    JSStringRef jsstring = JSStringCreateWithUTF8CString(constclassname);
    static JSObjectRef cconstclassObject = JSObjectMake(context, cconst_createJSClass(context), NULL);
    JSObjectSetProperty(
		context,
	    parentObject,
	    jsstring,
	    cconstclassObject,
	    kJSPropertyAttributeNone,
	    NULL);
    JSStringRelease(jsstring);
    return true;
} 
%}



/* Additional typemaps needed for generating the code for handling C global variables as properties if the cvar object */

%typemap (variablegetdecl) SWIGTYPE
%{
JSValueRef $variablegetname(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)%}

%typemap (variablesetdecl) SWIGTYPE
%{
bool $variablesetname(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef value, 
JSValueRef* exception)%}


%typemap (variablebody) SWIGTYPE
%{
    SWIGUNUSEDPARM(thisObject);
    SWIGUNUSEDPARM(exception);
    JSValueRef jsresult;
%}

%typemap (cvardecl) SWIGTYPE 
%{
bool cvar_initClass(JSGlobalContextRef& context, JSObjectRef& parentObject, const char* cvarclassname);
%}

%typemap (cvardefn) SWIGTYPE
%{

JSStaticValue cvar_staticValues[] = {
$jsstaticvarcode
    { 0, 0, 0, 0 }
};

JSClassRef cvar_createJSClass(JSContextRef context)
{
	static JSClassRef jsClass = NULL;
	if (!jsClass)
	{
		JSClassDefinition definition	= kJSClassDefinitionEmpty;
		definition.staticValues			= cvar_staticValues;
		jsClass							= JSClassCreate(&definition);
	}
	return jsClass;
}

bool cvar_initClass(JSGlobalContextRef& context, JSObjectRef& parentObject, const char* cvarclassname)
{
    JSStringRef jsstring = JSStringCreateWithUTF8CString(cvarclassname);
    static JSObjectRef cvarclassObject = JSObjectMake(context, cvar_createJSClass(context), NULL);
    JSObjectSetProperty(
		context,
	    parentObject,
	    jsstring,
	    cvarclassObject,
	    kJSPropertyAttributeNone,
	    NULL);
   JSStringRelease(jsstring);
   return true;
} 
%}





