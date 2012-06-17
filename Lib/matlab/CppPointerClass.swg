classdef CppPointerClass

properties (GetAccess = public, SetAccess = private)
    pointer;
end

methods
function this = CppPointerClass(p)
    if nargin ~= 1 || ~isa(p,'lib.pointer')
        error('Illegal constructor call');
    end
    this.pointer = p;
end
end

end %classdef
