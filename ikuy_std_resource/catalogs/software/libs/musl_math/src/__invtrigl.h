#include "libm.h"

/* shared by acosl, asinl and atan2l */
#define pio2_hi __pio2_hi
#define pio2_lo __pio2_lo

const long double __pio2_hi, __pio2_lo;

long double __invtrigl_R(long double z);
