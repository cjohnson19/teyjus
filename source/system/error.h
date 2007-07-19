/****************************************************************************
 *                                                                          *
 *   File error.h -- error-handling functions for all Teyjus builds.        *
 *   (TEMP)                                                                 *
 *                                                                          *
 ****************************************************************************/

#ifndef ERROR_H
#define ERROR_H

#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>
#include "tjsignal.h"
#include "../simulator/mctypes.h" //to be modified

/****************************************************************************
 * Exception stack declarations.                                            *
 ****************************************************************************/

typedef enum EM_ExnType{
    EM_NO_EXN = 0,     // used for warnings ?? 
    EM_ABORT,          // exit the executable immediately
    EM_EXIT,           // traverse the exception stack and exit
    EM_TOP_LEVEL,      // return to the toplevel 
    EM_QUERY,          // abort solving the query
    EM_QUERY_RESULT,   // query is solved; print answer
    EM_FAIL,           // fail to simulator level 
    EM_TY_UNI_FAIL,    // fail from type unification; needed?
    EM_HOPU_FAIL       // fail from pattern unification; needed?
} EM_ExnType;

//function call environment stack
extern SIGNAL_jmp_buf *EM_ExnHandlerStack;
extern int             EM_ExnHandlerStackTop;
extern int             EM_ExnHandlerStackSize;

//exception type
extern EM_ExnType      EM_CurrentExnType;

/****************************************************************************
 * Exception-handling macros                                                *
 ****************************************************************************/

//try 
#define EM_TRY \
if (EM_ExnHandlerStackTop >= EM_ExnHandlerStackSize) \
{ \
   EM_ExnHandlerStackSize = \
     (EM_ExnHandlerStackSize + 1) * 2; \
   EM_ExnHandlerStack = \
     (SIGNAL_jmp_buf *)EM_realloc((void *)EM_ExnHandlerStack, \
	   EM_ExnHandlerStackSize * sizeof(SIGNAL_jmp_buf)); \
} \
if (SIGNAL_setjmp(EM_ExnHandlerStack[EM_ExnHandlerStackTop++]) == 0) \
{

//catch
#define EM_CATCH \
   EM_ExnHandlerStackTop--; \
} \
else

//throw
/* Jump to the nearest (in a dynamic sense) EM_Try block, setting
   EM_CurrentExnType to TYPE. Given a constant, the conditional in
   this macro will be optimized away. */
#define EM_THROW(type) EM_THROWVAL((type), 1)

#define EM_THROWVAL(type, val) \
do { \
   if ((type) == EM_ABORT) \
      exit(1); \
   else \
   { \
      EM_CurrentExnType = (type); \
      SIGNAL_longjmp(EM_ExnHandlerStack[--EM_ExnHandlerStackTop], val); \
   } \
} while(0)

//rethrow
/* pass the current exception to the next handler.  Use only within an
   EM_Catch block. */
#define EM_RETHROW() \
   SIGNAL_longjmp(EM_ExnHandlerStack[--EM_ExnHandlerStackTop], 1)

/* Here's an example use of the above macros:

...
EM_TRY
{
   foo();
   if (foobar)
      EM_THROW(EM_FOOBAR);
}
EM_CATCH
{
   un_foo();			/* clean up *
   if (EM_CurrentExnType == EM_FOOBAR)
      printf("foobar!");        /* stop the error here *
   else
      EM_RETHROW();             /* let a later handler handle it *
}
*/

/****************************************************************************
 * Routines which will generate errors automatically.                       *
 ****************************************************************************/
void *EM_malloc(unsigned int);
void *EM_realloc(void *, unsigned int);
char *EM_strdup(char *);

/****************************************************************************
 * Beginning error indices for different modules (by module abbreviation)   *
 ****************************************************************************/
/* general errors */
#define EM_NO_ERROR              0
#define EM_FIRST_ERR_INDEX       1
#define LINKER_FIRST_ERR_INDEX   50
#define LOADER_FIRST_ERR_INDEX   100
#define STREAM_FIRST_ERR_INDEX   150
#define SIM_FIRST_ERR_INDEX      200
#define BI_FIRST_ERR_INDEX       300

/****************************************************************************
 * General-use error messages                                               *
 ****************************************************************************/
enum
{
   EM_OUT_OF_MEMORY = EM_FIRST_ERR_INDEX,
   EM_OUT_OF_HEAP,
   EM_NEWLINE,
   EM_ERROR_COLON,
   EM_WARNING_COLON
};

/****************************************************************************
 * The routine that gets called in the event of an error                    *
 ****************************************************************************/
void EM_error(int inIndex, ...);


/****************************************************************************
 * Have there been any errors since last EM_Reset()?                        *
 ****************************************************************************/
extern Boolean EM_anyErrors;
void EM_reset();

#endif  //ERROR_H 
