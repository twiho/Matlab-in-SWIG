classdef CppBaseClass < handle

% property with a C++ object pointer
properties (GetAccess = public, SetAccess = protected)
    pointer;
end

methods (Static, Access = private)
% registers allows multiple Matlab classes with the same C++ object pointer
% necessary for correct C++ destructor calls
function retVal = worker(method, pointer)
    mlock;
    persistent register;
    switch method
        case 'add'
            register = [register pointer];
        case 'remove'
            deleted = false;
            r = 1;
            while(r<=numel(register))
                if register(r) == pointer
                    register(r) = [];
                    deleted = true;
                    break;
                end
                r = r+1;
            end
            if ~deleted
                warning('LibraryUI:CppPointerRegister','Pointer not registered. The C++ object was either already deleted, or not registered - possible memory leak!');
                retVal = false;
                return;
            end
            while(r<=numel(register))
                if register(r) == pointer
                    retVal = false;
                    return;
                end
                r = r+1;
            end
            retVal = true;
    end
end
end

methods (Static, Access = protected)
% registers a C++ pointer
function addCppPointer(pointer)
    CppBaseClass.worker('add', pointer);
end

% returns true, when C++ destructor should be called
function retVal = removeCppPointer(pointer)
    retVal = CppBaseClass.worker('remove', pointer);
end
end

end %classdef
