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
                    if ~deleted
                        register(r) = [];
                        deleted = true;
                    else
                        retVal = false;
                        return;
                    end
                end
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
