#include "swigmod.h"

class MATLAB : public Language {
public:

  virtual void main(int argc, char *argv[]) {
    printf("I'm the MATLAB module.\n");
  }

  virtual int top(Node *n) {
    printf("Generating code.\n");
    return SWIG_OK;
  }

};

extern "C" Language *
swig_matlab(void) {
  return new MATLAB();
}
