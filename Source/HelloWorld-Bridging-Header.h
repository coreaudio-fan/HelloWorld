#include "HelloWorld.h"
#include "HelloWorld.hpp"
#include "HelloWorld_objc.h"

// run_demo_c declared here rather than via Demo_C.h to avoid exposing the C
// Student typedef, which would conflict with the Swift Student struct.
void	run_demo_c(void);
#include "Demo_Cpp.hpp"
#include "Demo_ObjC.h"
