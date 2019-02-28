//Empty
#define WIN32_LEAN_AND_MEAN
#ifdef WIN32_LEAN_AND_MEAN
#endif
#include "../helper.h"

//-------------------------------------------------------------------------------------------------
extern "C" [[noreturn]]
void start()
{
    RtlExitUserProcess(0);
}
