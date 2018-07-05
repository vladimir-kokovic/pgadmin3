//
//Made for PgAdmin by Vladimir Kokovic (vladimir.kokovic@gmail.com)
//Based on Bill Rossi's Decimal Floating Point library for Java (http://dfp.sourceforge.net/)
//
#ifndef VKROSSIDFP_H
#define VKROSSIDFP_H

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>

//using namespace std;

/**
 *  Decimal floating point library
 *
 *  Another floating point class.
 *  This one is built using radix 10000  which is 10^4, so its almost decimal.
 *
 *  The design goals here are -
 *     1.) decimal math, or close to it.
 *     2.) Compile-time settable precision
 *     3.) Portability.  Code should be keep as portable as possible.
 *     4.) Performance
 *     5.) Accuracy  - Results should always be +/- 1 ULP for basic algebraic operation
 *     6.) Comply with IEEE 854-1987 as much as possible. (See IEEE 854-1987 notes below)
 *
 *  The trade offs -
 *     1.) Memory foot print.
 *         I'm using more memory than necessary to represent numbers to get better performance.
 *     2.) Digits are bigger, so rounding is a greater loss.
 *         So, if you really need 12 decimal digits, better use 4 base 10000 digits
 *         there can be one partially filled.
 *
 *  Numbers are represented  in the following form:
 *
 *  n  =  sign * mant * (radix) ^ exp;
 *
 *  where sign is +/- 1,
 *        mantissa represents a fractional number between zero and one.
 *        mant[0] is the least significant digit.
 *        exp is in the range of -32767 to 32768
 *
 *  dfp objects are immuatable via their public interface.
 *
 *  IEEE 854-1987  Notes and differences
 *
 *  IEEE 854 requires the radix to be either 2 or 10.
 *  The radix here is 10000, so that requirement is not met, but  it is possible that a
 *  subclassed can be made to make it behave as a radix 10 number.
 *  It is my opinion that if it looks and behaves as a radix
 *  10 number then it is one and that requirement would be met.
 *
 *  The radix of 10000 was chosen because it should be faster to operate
 *  on 4 decimal digits at once intead of one at a time.
 *  Radix 10 behaviour can be realized by add an additional rounding step to ensure that
 *  the number of decimal digits represented is constant.
 *
 *  The IEEE standard specifically leaves out internal data encoding,
 *  so it is reasonable to conclude that such a subclass of this radix
 *  10000 system is merely an encoding of a radix 10 system.
 *
 *  IEEE 854 also specifies the existance of "sub-normal" numbers.
 *  This class does not contain any such entities.
 *  The most significant radix 10000 digit is always non-zero.
 *  Instead, we support "gradual underflow"
 *  by raising the underflow flag for numbers less with exponent less than
 *  expMin, but dont flush to zero until the exponent reaches expMin-DIGITS.
 *  Thus the smallest number we can represent would be:
 *  1E(-(minExp-DIGITS-1)*4),  eg, for DIGITS=5, minExp=-32767, that would
 *  be 1e-131092.
 *
 *  IEEE 854 defines that the implied radix point lies just to the right
 *  of the most significant digit and to the left of the remaining digits.
 *  This implementation puts the implied radix point to the left of all
 *  digits including the most significant one.  The most significant digit
 *  here is the one just to the right of the radix point.  This is a fine
 *  detail and is really only a matter of definition.  Any side effects of
 *  this can be rendered invisible by a subclass.
 */

class dfp
{
protected:

    int *mant; // the mantissa
    char sign; // the sign bit.  1 for positive, -1 for negative
    int exp;   // the exponent.
    char nans; // Indicates non-finite / non-number values

    dfp string2dfp(std::string fpin);

    /** round this given the next digit n using the current rounding mode returns a flag if an exception occured */
    int round(int n);

    /** Shift the mantissa left, and adjust the exponent to compensate */
    void shiftLeft();

    /** Shift the mantissa right, and adjust the exponent to compensate */
    void shiftRight();

    /**
     * Make our exp equalDFP to the supplied one.  This may cause rounding.
     *  Also causes de-normalized numbers.  These numbers are generally
     *  dangerous because most routines assume normalized numbers.
     *  Align doesn't round, so it will return the last digit destroyed
     *  by shifting right.
     * @throws std::exception
     */
    int align(int e);

    /* Convert a dfp to a string using scientific notation */
    std::string dfp2sci(dfp & d);

    /* converts a dfp to a string handling the normal case */
    std::string dfp2string(dfp & a);

    /** Trap handler.  Subclasses may override this to provide trap
     *  functionality per IEEE 854-1987.
     *
     *  @param type  The exception type - e.g. FLAG_OVERFLOW
     *  @param what  The name of the routine we were in e.g. divide()
     *  @param oper  An operand to this function if any
     *  @param def   The default return value if trap not enabled
     *  @param result    The result that is spcefied to be delivered per
     *                   IEEE 854, if any
     * @return
     */
    dfp trap(int type, const char *what, dfp *oper, dfp *def, dfp *result);

    /** Negate the mantissa of this by computing the complement.
     *  Leaves the sign bit unchanged, used internally by add.
     *  Denormalized numbers are handled properly here.
     */
    int complement(int extra);

    /** Does the integer conversions with the spec rounding.
     * @throws std::exception
     */
    dfp trunc(int rmode);

public:

    friend void assignDFP(const dfp & source, dfp & target);

    /** Default constructor.  Makes a dfp with a value of zero */
    dfp();

    /** Copy constructor.  Creates a copy of the supplied dfp */
    dfp(const dfp & d);

    /** Create a dfp given a String representation */
    dfp(const std::string & s);

    dfp & operator =(const dfp &);

    /** Creates a dfp with a non-finite value */
    static dfp create(char sign, char nans);

    std::string toString();

    virtual ~dfp();

    /** Raises a trap.  This does not set the corresponding flag however.
     *  @param type the trap type
     *  @param what - name of routine trap occured in
     *  @param oper - input operator to function
     *  @param result - the result computed prior to the trap
     *  @return The suggested return value from the trap handler
     *
     *  @throws std::exception
     */
    dfp dotrap(int type, const char *what, dfp *oper, dfp *result);

    /** Returns the IEEE 854 status flags */
    static int getIEEEFlags();

    /** Clears the IEEE 854 status flags */
    static void clearIEEEFlags();

    /** Sets the IEEE 854 status flags */
    static void setIEEEFlags(int flags);

    /** Returns the type - one of FINITE, INFINITE, SNAN, QNAN */
    int classify();

    /** Creates a dfp that is the same as x except that it has
     *  the sign of y.   abs(x) = dfp.copysign(x, dfp.one)
     */
    static dfp copysign(dfp x, dfp y);

    /** returns true if this is less than x.
     *  returns false if this or x is NaN
     * @throws std::exception
     */
    bool lessThan(dfp & x);

    /** returns true if this is greater than x.
     *  returns false if this or x is NaN
     * @throws std::exception
     */
    bool greaterThan(dfp & x);

    /**
     * returns true if this is equalDFP to x. false if this or x is NaN
     * @param x
     * @return
     */
    bool equalDFP(dfp & x);

    /**
     * returns true if this is not equalDFP to x.
     * different from !equalDFP(x) in the way NaNs are handled.
     * @throws std::xception
     */
    bool unequalDFP(dfp & x);

    /** compare a and b.  return -1 if a<b, 1 if a>b and 0 if a==b
     *  Note this method does not properly handle NaNs.
     */
    static int compare(dfp & a, dfp & b);

    /** Returns the next number greater than this one in the direction of x.
     *  If this==x then simply returns this.
     * @throws std::Exception
     */
    dfp nextAfter(dfp x);

    /** Add x to this and return the result
     * @throws std::exception
     */
    dfp add(dfp x);

    /** Returns a number that is this number with the sign bit reversed */
    dfp negate();

    /** Subtract a from this
     * @throws java.lang.Exception
     */
    dfp subtract(dfp a);

    /** Multiply this by x
     * @throws std::exception
     */
    dfp multiply(dfp x);

    /** Multiply this by a single digit 0<=x<radix.
     *  There are speed advantages in this special case
     * @throws std::exception
     */
    dfp multiply(int x);

    /** Divide this by divisor
     * @throws std::exception
     */
    dfp divide(dfp divisor);

    /** Divide by a single digit less than radix.
     *  Special case, so there are speed advantages.
     *  0 <== divisor < radix
     * @throws std::exception
     */
    dfp divide(int divisor);

    dfp sqrt();

    /** Round to nearest integer using the round-half-even method.
     *  That is round to nearest integer unless both are equidistant.
     *  In which case round to the even one.
     * @throws std::exception
     */
    dfp rint();

    /** Round to an integer using the round floor mode.
     *  That is, round toward -Infinity
     * @throws std::exception
     */
    dfp floor();

    /** Round to an integer using the ceil floor mode.  That is,
     *  round toward +Infinity
     * @throws std::exception
     */
    dfp ceil();

    /** Returns the IEEE remainder.  That is the result of this less
     *  n times d, where n is the integer closest to this/d.
     * @throws std::exception
     */
    dfp remainder(dfp & d);

    /** Convert this to an integer.  If greater than 2147483647, it returns
     *  2147483647.  If less than -2147483648 it returns -2147483648.
     * @throws std::exception
     */
    int intValue();

    /**
     * Returns the exponent of the greatest power of 10000 that is
     *  less than or equalDFP to the absolute value of this.  I.E.  if
     *  this is 10e6 then log10K would return 1.
     */
    int log10K();

    /** Return the specified  power of 10000
     */
    dfp power10K(int e);

    /** Return the specified  power of 10
     * @throws std::exception
     */
    dfp power10(int e);

    /**
     *  Return the exponent of the greatest power of 10 that is less than
     *  or equalDFP to than abs(this).
     */
    int log10();

    /** Set the rounding mode to be one of the following values:
     *  ROUND_UP, ROUND_DOWN, ROUND_HALF_UP, ROUND_HALF_DOWN,
     *  ROUND_HALF_EVEN, ROUND_HALF_ODD, ROUND_CEIL, ROUND_FLOOR.
     *
     *  Default is ROUND_HALF_EVEN
     *
     *  Note that the rounding mode is common to all instances
     *  in the system and will effect all future calculations.
    */
    static void setRoundingMode(int mode);

    /** Returns the current rounding mode */
    static int getRoundingMode();

    static int rMode; // Current rounding mode
    static int ieeeFlags; // IEEE 854-1987 signals

    /** The radix, or base of this system.  Set to 10000 */
    static const int radix = 10000;

    /** The minium exponent before underflow is signaled.
     *  Flush to zero occurs at minExp-DIGITS */
    static const int minExp = -32767;

    /** The maximum exponent before overflow is signaled and results flushed to infinity */
    static const int maxExp = 32768;

    /** The amount under/overflows are scaled by before going to trap handler */
    static const int errScale = 32760;

    /**** Rounding modes *****/

    /** Rounds toward zero.  I.E. truncation */
    static const int ROUND_DOWN = 0;

    /** Rounds away from zero if discarded digit is non-zero */
    static const int ROUND_UP = 1;

    /** Rounds towards nearest unless both are equidistant in which case
     *  it rounds away from zero */
    static const int ROUND_HALF_UP = 2;

    /** Rounds towards nearest unless both are equidistant in which case
     *  it rounds toward zero */
    static const int ROUND_HALF_DOWN = 3;

    /** Rounds towards nearest unless both are equidistant in which case
     *  it rounds toward the even neighbor.
     *  This is the default as specified by IEEE 854-1987 */
    static const int ROUND_HALF_EVEN = 4;

    /** Rounds towards nearest unless both are equidistant in which case
     *  it rounds toward the odd neighbor.  */
    static const int ROUND_HALF_ODD = 5;

    /** Rounds towards positive infinity  */
    static const int ROUND_CEIL = 6;

    /** Rounds towards negative infinity  */
    static const int ROUND_FLOOR = 7;

    /* non-numbers per IEEE 854-1987 */
    static const char FINITE = 0;   // Normal finite numbers
    static const char DFP_INFINITE = 1; // Infinity
    static const char SNANDFP = 2;     // Signaling NaN
    static const char QNAN = 3;     // Quiet NaN

    /* Flags */
    static const int FLAG_INVALID = 1;   // Invalid operation
    static const int FLAG_DIV_ZERO = 2;  // Division by zero
    static const int FLAG_OVERFLOW = 4;  // Overflow
    static const int FLAG_UNDERFLOW = 8; // Underflow
    static const int FLAG_INEXACT = 16;  // Inexact

    /* Handy constants */
    static const dfp zero;
    static const dfp one;
    static const dfp two;

    /** The number of digits.  note these are radix 10000 digits, so each
     *  one is equivilent to 4 decimal digits */
    static const int DIGITS = 5/*11*/; // each digit yeilds 4 decimal digits
};


#ifdef __GNUC__
  #define LONG_LONG  long long
  #define ULONG_LONG unsigned long long
  #ifdef __MINGW32__
    #define FMT_LONG_LONG "%I64d"
    #define FMT_ULONG_LONG "%I64u"
  #else
    #define FMT_LONG_LONG "%lld"
    #define FMT_ULONG_LONG "%llu"
  #endif
  #define CONST_LONG_LONG(a)  a##ll
  #define CONST_ULONG_LONG(a) a##ull
#else
  #define LONG_LONG  __int64
  #define ULONG_LONG unsigned __int64
  #define FMT_LONG_LONG "%I64d"
  #define FMT_ULONG_LONG "%I64u"
  #define CONST_LONG_LONG(a)  a
  #define CONST_ULONG_LONG(a) a
#endif

extern std::string itoa(int value, int base);
extern std::string itoa10(int value);

class vkSprintf {
  protected:
    void *s;
  public:
    vkSprintf();
    ~vkSprintf();
    char *vsprintf(const char *format, va_list argPtr);
    char *sprintf(const char *format, ...);
    char *sprintfFD(double x);
    char *sprintfLL(LONG_LONG x);
    char *sprintfUL(ULONG_LONG x);
};

class vkRossiDFP : public dfp
{
protected:
    static vkSprintf sc;
    static vkRossiDFP plusminusvar;

public:
    friend dfp;
    friend void assignvkRossiDFP(const vkRossiDFP & source, vkRossiDFP & target);
    static const vkRossiDFP ZERO;
    vkRossiDFP();
    virtual ~vkRossiDFP();
    vkRossiDFP(const vkRossiDFP &);
    vkRossiDFP(const char);
    vkRossiDFP(const unsigned char);
    vkRossiDFP(const short);
    vkRossiDFP(const unsigned short);
    vkRossiDFP(const int);
    vkRossiDFP(const unsigned int);
    vkRossiDFP(const long);
    vkRossiDFP(const unsigned long);
    vkRossiDFP(const float);
    vkRossiDFP(const double);
    vkRossiDFP(const char *);
    vkRossiDFP(const unsigned char *);
    vkRossiDFP(const LONG_LONG);
    vkRossiDFP(const ULONG_LONG);

    vkRossiDFP & operator =(const vkRossiDFP &);
    vkRossiDFP & operator =(const char);
    vkRossiDFP & operator =(const unsigned char);
    vkRossiDFP & operator =(const short);
    vkRossiDFP & operator =(const unsigned short);
    vkRossiDFP & operator =(const int);
    vkRossiDFP & operator =(const unsigned int);
    vkRossiDFP & operator =(const long);
    vkRossiDFP & operator =(const unsigned long);
    vkRossiDFP & operator =(const float);
    vkRossiDFP & operator =(const double);
    vkRossiDFP & operator =(const char *);
    vkRossiDFP & operator =(const unsigned char *);
    vkRossiDFP & operator =(const LONG_LONG);
    vkRossiDFP & operator =(const ULONG_LONG);

    vkRossiDFP & operator +=(const vkRossiDFP &);
    vkRossiDFP & operator +=(const char);
    vkRossiDFP & operator +=(const unsigned char);
    vkRossiDFP & operator +=(const short);
    vkRossiDFP & operator +=(const unsigned short);
    vkRossiDFP & operator +=(const int);
    vkRossiDFP & operator +=(const unsigned int);
    vkRossiDFP & operator +=(const long);
    vkRossiDFP & operator +=(const unsigned long);
    vkRossiDFP & operator +=(const float);
    vkRossiDFP & operator +=(const double);
    vkRossiDFP & operator +=(const char *);
    vkRossiDFP & operator +=(const unsigned char *);
    vkRossiDFP & operator +=(const LONG_LONG);
    vkRossiDFP & operator +=(const ULONG_LONG);

    vkRossiDFP & operator -=(const vkRossiDFP &);
    vkRossiDFP & operator -=(const char);
    vkRossiDFP & operator -=(const unsigned char);
    vkRossiDFP & operator -=(const short);
    vkRossiDFP & operator -=(const unsigned short);
    vkRossiDFP & operator -=(const int);
    vkRossiDFP & operator -=(const unsigned int);
    vkRossiDFP & operator -=(const long);
    vkRossiDFP & operator -=(const unsigned long);
    vkRossiDFP & operator -=(const float);
    vkRossiDFP & operator -=(const double);
    vkRossiDFP & operator -=(const char *);
    vkRossiDFP & operator -=(const unsigned char *);
    vkRossiDFP & operator -=(const LONG_LONG);
    vkRossiDFP & operator -=(const ULONG_LONG);

    vkRossiDFP & operator *=(const vkRossiDFP &);
    vkRossiDFP & operator *=(const char);
    vkRossiDFP & operator *=(const unsigned char);
    vkRossiDFP & operator *=(const short);
    vkRossiDFP & operator *=(const unsigned short);
    vkRossiDFP & operator *=(const int);
    vkRossiDFP & operator *=(const unsigned int);
    vkRossiDFP & operator *=(const long);
    vkRossiDFP & operator *=(const unsigned long);
    vkRossiDFP & operator *=(const float);
    vkRossiDFP & operator *=(const double);
    vkRossiDFP & operator *=(const char *);
    vkRossiDFP & operator *=(const unsigned char *);
    vkRossiDFP & operator *=(const LONG_LONG);
    vkRossiDFP & operator *=(const ULONG_LONG);

    vkRossiDFP & operator /=(const vkRossiDFP &);
    vkRossiDFP & operator /=(const char);
    vkRossiDFP & operator /=(const unsigned char);
    vkRossiDFP & operator /=(const short);
    vkRossiDFP & operator /=(const unsigned short);
    vkRossiDFP & operator /=(const int);
    vkRossiDFP & operator /=(const unsigned int);
    vkRossiDFP & operator /=(const long);
    vkRossiDFP & operator /=(const unsigned long);
    vkRossiDFP & operator /=(const float);
    vkRossiDFP & operator /=(const double);
    vkRossiDFP & operator /=(const char *);
    vkRossiDFP & operator /=(const unsigned char *);
    vkRossiDFP & operator /=(const LONG_LONG);
    vkRossiDFP & operator /=(const ULONG_LONG);

//     friend const vkRossiDFP operator +() const;
//     friend const vkRossiDFP operator -() const;

    friend vkRossiDFP operator  +(const vkRossiDFP &, const vkRossiDFP &);
    friend vkRossiDFP operator  +(const vkRossiDFP &, const char);
    friend vkRossiDFP operator  +(const vkRossiDFP &, const unsigned char);
    friend vkRossiDFP operator  +(const vkRossiDFP &, const short);
    friend vkRossiDFP operator  +(const vkRossiDFP &, const unsigned short);
    friend vkRossiDFP operator  +(const vkRossiDFP &, const int);
    friend vkRossiDFP operator  +(const vkRossiDFP &, const unsigned int);
    friend vkRossiDFP operator  +(const vkRossiDFP &, const long);
    friend vkRossiDFP operator  +(const vkRossiDFP &, const unsigned long);
    friend vkRossiDFP operator  +(const vkRossiDFP &, const float);
    friend vkRossiDFP operator  +(const vkRossiDFP &, const double);
    friend vkRossiDFP operator  +(const vkRossiDFP &, const char *);
    friend vkRossiDFP operator  +(const vkRossiDFP &, const unsigned char *);
    friend vkRossiDFP operator  +(const vkRossiDFP &, const LONG_LONG);
    friend vkRossiDFP operator  +(const vkRossiDFP &, const ULONG_LONG);
    friend vkRossiDFP operator  +(const char,            const vkRossiDFP &);
    friend vkRossiDFP operator  +(const unsigned char,   const vkRossiDFP &);
    friend vkRossiDFP operator  +(const short,           const vkRossiDFP &);
    friend vkRossiDFP operator  +(const unsigned short,  const vkRossiDFP &);
    friend vkRossiDFP operator  +(const int,             const vkRossiDFP &);
    friend vkRossiDFP operator  +(const unsigned int,    const vkRossiDFP &);
    friend vkRossiDFP operator  +(const long,            const vkRossiDFP &);
    friend vkRossiDFP operator  +(const unsigned long,   const vkRossiDFP &);
    friend vkRossiDFP operator  +(const float,           const vkRossiDFP &);
    friend vkRossiDFP operator  +(const double,          const vkRossiDFP &);
    friend vkRossiDFP operator  +(const char *,          const vkRossiDFP &);
    friend vkRossiDFP operator  +(const unsigned char *, const vkRossiDFP &);
    friend vkRossiDFP operator  +(const LONG_LONG,       const vkRossiDFP &);
    friend vkRossiDFP operator  +(const ULONG_LONG,      const vkRossiDFP &);

    friend vkRossiDFP operator  -(const vkRossiDFP &, const vkRossiDFP &);
    friend vkRossiDFP operator  -(const vkRossiDFP &, const char);
    friend vkRossiDFP operator  -(const vkRossiDFP &, const unsigned char);
    friend vkRossiDFP operator  -(const vkRossiDFP &, const short);
    friend vkRossiDFP operator  -(const vkRossiDFP &, const unsigned short);
    friend vkRossiDFP operator  -(const vkRossiDFP &, const int);
    friend vkRossiDFP operator  -(const vkRossiDFP &, const unsigned int);
    friend vkRossiDFP operator  -(const vkRossiDFP &, const long);
    friend vkRossiDFP operator  -(const vkRossiDFP &, const unsigned long);
    friend vkRossiDFP operator  -(const vkRossiDFP &, const float);
    friend vkRossiDFP operator  -(const vkRossiDFP &, const double);
    friend vkRossiDFP operator  -(const vkRossiDFP &, const char *);
    friend vkRossiDFP operator  -(const vkRossiDFP &, const unsigned char *);
    friend vkRossiDFP operator  -(const vkRossiDFP &, const LONG_LONG);
    friend vkRossiDFP operator  -(const vkRossiDFP &, const ULONG_LONG);
    friend vkRossiDFP operator  -(const char,            const vkRossiDFP &);
    friend vkRossiDFP operator  -(const unsigned char,   const vkRossiDFP &);
    friend vkRossiDFP operator  -(const short,           const vkRossiDFP &);
    friend vkRossiDFP operator  -(const unsigned short,  const vkRossiDFP &);
    friend vkRossiDFP operator  -(const int,             const vkRossiDFP &);
    friend vkRossiDFP operator  -(const unsigned int,    const vkRossiDFP &);
    friend vkRossiDFP operator  -(const long,            const vkRossiDFP &);
    friend vkRossiDFP operator  -(const unsigned long,   const vkRossiDFP &);
    friend vkRossiDFP operator  -(const float,           const vkRossiDFP &);
    friend vkRossiDFP operator  -(const double,          const vkRossiDFP &);
    friend vkRossiDFP operator  -(const char *,          const vkRossiDFP &);
    friend vkRossiDFP operator  -(const unsigned char *, const vkRossiDFP &);
    friend vkRossiDFP operator  -(const LONG_LONG,       const vkRossiDFP &);
    friend vkRossiDFP operator  -(const ULONG_LONG,      const vkRossiDFP &);

    friend vkRossiDFP operator  *(const vkRossiDFP &, const vkRossiDFP &);
    friend vkRossiDFP operator  *(const vkRossiDFP &, const char);
    friend vkRossiDFP operator  *(const vkRossiDFP &, const unsigned char);
    friend vkRossiDFP operator  *(const vkRossiDFP &, const short);
    friend vkRossiDFP operator  *(const vkRossiDFP &, const unsigned short);
    friend vkRossiDFP operator  *(const vkRossiDFP &, const int);
    friend vkRossiDFP operator  *(const vkRossiDFP &, const unsigned int);
    friend vkRossiDFP operator  *(const vkRossiDFP &, const long);
    friend vkRossiDFP operator  *(const vkRossiDFP &, const unsigned long);
    friend vkRossiDFP operator  *(const vkRossiDFP &, const float);
    friend vkRossiDFP operator  *(const vkRossiDFP &, const double);
    friend vkRossiDFP operator  *(const vkRossiDFP &, const char *);
    friend vkRossiDFP operator  *(const vkRossiDFP &, const unsigned char *);
    friend vkRossiDFP operator  *(const vkRossiDFP &, const LONG_LONG);
    friend vkRossiDFP operator  *(const vkRossiDFP &, const ULONG_LONG);
    friend vkRossiDFP operator  *(const char,            const vkRossiDFP &);
    friend vkRossiDFP operator  *(const unsigned char,   const vkRossiDFP &);
    friend vkRossiDFP operator  *(const short,           const vkRossiDFP &);
    friend vkRossiDFP operator  *(const unsigned short,  const vkRossiDFP &);
    friend vkRossiDFP operator  *(const int,             const vkRossiDFP &);
    friend vkRossiDFP operator  *(const unsigned int,    const vkRossiDFP &);
    friend vkRossiDFP operator  *(const long,            const vkRossiDFP &);
    friend vkRossiDFP operator  *(const unsigned long,   const vkRossiDFP &);
    friend vkRossiDFP operator  *(const float,           const vkRossiDFP &);
    friend vkRossiDFP operator  *(const double,          const vkRossiDFP &);
    friend vkRossiDFP operator  *(const char *,          const vkRossiDFP &);
    friend vkRossiDFP operator  *(const unsigned char *, const vkRossiDFP &);
    friend vkRossiDFP operator  *(const LONG_LONG,       const vkRossiDFP &);
    friend vkRossiDFP operator  *(const ULONG_LONG,      const vkRossiDFP &);

    friend vkRossiDFP operator  /(const vkRossiDFP &, const vkRossiDFP &);
    friend vkRossiDFP operator  /(const vkRossiDFP &, const char);
    friend vkRossiDFP operator  /(const vkRossiDFP &, const unsigned char);
    friend vkRossiDFP operator  /(const vkRossiDFP &, const short);
    friend vkRossiDFP operator  /(const vkRossiDFP &, const unsigned short);
    friend vkRossiDFP operator  /(const vkRossiDFP &, const int);
    friend vkRossiDFP operator  /(const vkRossiDFP &, const unsigned int);
    friend vkRossiDFP operator  /(const vkRossiDFP &, const long);
    friend vkRossiDFP operator  /(const vkRossiDFP &, const unsigned long);
    friend vkRossiDFP operator  /(const vkRossiDFP &, const float);
    friend vkRossiDFP operator  /(const vkRossiDFP &, const double);
    friend vkRossiDFP operator  /(const vkRossiDFP &, const char *);
    friend vkRossiDFP operator  /(const vkRossiDFP &, const unsigned char *);
    friend vkRossiDFP operator  /(const vkRossiDFP &, const LONG_LONG);
    friend vkRossiDFP operator  /(const vkRossiDFP &, const ULONG_LONG);
    friend vkRossiDFP operator  /(const char,            const vkRossiDFP &);
    friend vkRossiDFP operator  /(const unsigned char,   const vkRossiDFP &);
    friend vkRossiDFP operator  /(const short,           const vkRossiDFP &);
    friend vkRossiDFP operator  /(const unsigned short,  const vkRossiDFP &);
    friend vkRossiDFP operator  /(const int,             const vkRossiDFP &);
    friend vkRossiDFP operator  /(const unsigned int,    const vkRossiDFP &);
    friend vkRossiDFP operator  /(const long,            const vkRossiDFP &);
    friend vkRossiDFP operator  /(const unsigned long,   const vkRossiDFP &);
    friend vkRossiDFP operator  /(const float,           const vkRossiDFP &);
    friend vkRossiDFP operator  /(const double,          const vkRossiDFP &);
    friend vkRossiDFP operator  /(const char *,          const vkRossiDFP &);
    friend vkRossiDFP operator  /(const unsigned char *, const vkRossiDFP &);
    friend vkRossiDFP operator  /(const LONG_LONG,       const vkRossiDFP &);
    friend vkRossiDFP operator  /(const ULONG_LONG,      const vkRossiDFP &);

    friend vkRossiDFP operator  %(const vkRossiDFP &, const vkRossiDFP &);
    friend vkRossiDFP operator  %(const vkRossiDFP &, const char);
    friend vkRossiDFP operator  %(const vkRossiDFP &, const unsigned char);
    friend vkRossiDFP operator  %(const vkRossiDFP &, const short);
    friend vkRossiDFP operator  %(const vkRossiDFP &, const unsigned short);
    friend vkRossiDFP operator  %(const vkRossiDFP &, const int);
    friend vkRossiDFP operator  %(const vkRossiDFP &, const unsigned int);
    friend vkRossiDFP operator  %(const vkRossiDFP &, const long);
    friend vkRossiDFP operator  %(const vkRossiDFP &, const unsigned long);
    friend vkRossiDFP operator  %(const vkRossiDFP &, const float);
    friend vkRossiDFP operator  %(const vkRossiDFP &, const double);
    friend vkRossiDFP operator  %(const vkRossiDFP &, const char *);
    friend vkRossiDFP operator  %(const vkRossiDFP &, const unsigned char *);
    friend vkRossiDFP operator  %(const vkRossiDFP &, const LONG_LONG);
    friend vkRossiDFP operator  %(const vkRossiDFP &, const ULONG_LONG);
    friend vkRossiDFP operator  %(const char,            const vkRossiDFP &);
    friend vkRossiDFP operator  %(const unsigned char,   const vkRossiDFP &);
    friend vkRossiDFP operator  %(const short,           const vkRossiDFP &);
    friend vkRossiDFP operator  %(const unsigned short,  const vkRossiDFP &);
    friend vkRossiDFP operator  %(const int,             const vkRossiDFP &);
    friend vkRossiDFP operator  %(const unsigned int,    const vkRossiDFP &);
    friend vkRossiDFP operator  %(const long,            const vkRossiDFP &);
    friend vkRossiDFP operator  %(const unsigned long,   const vkRossiDFP &);
    friend vkRossiDFP operator  %(const float,           const vkRossiDFP &);
    friend vkRossiDFP operator  %(const double,          const vkRossiDFP &);
    friend vkRossiDFP operator  %(const char *,          const vkRossiDFP &);
    friend vkRossiDFP operator  %(const unsigned char *, const vkRossiDFP &);
    friend vkRossiDFP operator  %(const LONG_LONG,       const vkRossiDFP &);
    friend vkRossiDFP operator  %(const ULONG_LONG,      const vkRossiDFP &);

    friend const vkRossiDFP operator  +(const vkRossiDFP &);
    friend const vkRossiDFP operator  -(const vkRossiDFP &);
    const vkRossiDFP & operator ++();
    const vkRossiDFP & operator ++(int);
    const vkRossiDFP & operator --();
    const vkRossiDFP & operator --(int);

    friend bool operator ==(const vkRossiDFP &, const vkRossiDFP &);
    friend bool operator ==(const vkRossiDFP &, const char);
    friend bool operator ==(const vkRossiDFP &, const unsigned char);
    friend bool operator ==(const vkRossiDFP &, const short);
    friend bool operator ==(const vkRossiDFP &, const unsigned short);
    friend bool operator ==(const vkRossiDFP &, const int);
    friend bool operator ==(const vkRossiDFP &, const unsigned int);
    friend bool operator ==(const vkRossiDFP &, const long);
    friend bool operator ==(const vkRossiDFP &, const unsigned long);
    friend bool operator ==(const vkRossiDFP &, const float);
    friend bool operator ==(const vkRossiDFP &, const double);
    friend bool operator ==(const vkRossiDFP &, const char *);
    friend bool operator ==(const vkRossiDFP &, const unsigned char *);
    friend bool operator ==(const vkRossiDFP &, const LONG_LONG);
    friend bool operator ==(const vkRossiDFP &, const ULONG_LONG);
    friend bool operator ==(const char,            const vkRossiDFP &);
    friend bool operator ==(const unsigned char,   const vkRossiDFP &);
    friend bool operator ==(const short,           const vkRossiDFP &);
    friend bool operator ==(const unsigned short,  const vkRossiDFP &);
    friend bool operator ==(const int,             const vkRossiDFP &);
    friend bool operator ==(const unsigned int,    const vkRossiDFP &);
    friend bool operator ==(const long,            const vkRossiDFP &);
    friend bool operator ==(const unsigned long,   const vkRossiDFP &);
    friend bool operator ==(const float,           const vkRossiDFP &);
    friend bool operator ==(const double,          const vkRossiDFP &);
    friend bool operator ==(const char *,          const vkRossiDFP &);
    friend bool operator ==(const unsigned char *, const vkRossiDFP &);
    friend bool operator ==(const LONG_LONG,       const vkRossiDFP &);
    friend bool operator ==(const ULONG_LONG,      const vkRossiDFP &);

    friend bool operator !=(const vkRossiDFP &, const vkRossiDFP &);
    friend bool operator !=(const vkRossiDFP &, const char);
    friend bool operator !=(const vkRossiDFP &, const unsigned char);
    friend bool operator !=(const vkRossiDFP &, const short);
    friend bool operator !=(const vkRossiDFP &, const unsigned short);
    friend bool operator !=(const vkRossiDFP &, const int);
    friend bool operator !=(const vkRossiDFP &, const unsigned int);
    friend bool operator !=(const vkRossiDFP &, const long);
    friend bool operator !=(const vkRossiDFP &, const unsigned long);
    friend bool operator !=(const vkRossiDFP &, const float);
    friend bool operator !=(const vkRossiDFP &, const double);
    friend bool operator !=(const vkRossiDFP &, const char *);
    friend bool operator !=(const vkRossiDFP &, const unsigned char *);
    friend bool operator !=(const vkRossiDFP &, const LONG_LONG);
    friend bool operator !=(const vkRossiDFP &, const ULONG_LONG);
    friend bool operator !=(const char,            const vkRossiDFP &);
    friend bool operator !=(const unsigned char,   const vkRossiDFP &);
    friend bool operator !=(const short,           const vkRossiDFP &);
    friend bool operator !=(const unsigned short,  const vkRossiDFP &);
    friend bool operator !=(const int,             const vkRossiDFP &);
    friend bool operator !=(const unsigned int,    const vkRossiDFP &);
    friend bool operator !=(const long,            const vkRossiDFP &);
    friend bool operator !=(const unsigned long,   const vkRossiDFP &);
    friend bool operator !=(const float,           const vkRossiDFP &);
    friend bool operator !=(const double,          const vkRossiDFP &);
    friend bool operator !=(const char *,          const vkRossiDFP &);
    friend bool operator !=(const unsigned char *, const vkRossiDFP &);
    friend bool operator !=(const LONG_LONG,       const vkRossiDFP &);
    friend bool operator !=(const ULONG_LONG,      const vkRossiDFP &);

    friend bool operator <(const vkRossiDFP &, const vkRossiDFP &);
    friend bool operator <(const vkRossiDFP &, const char);
    friend bool operator <(const vkRossiDFP &, const unsigned char);
    friend bool operator <(const vkRossiDFP &, const short);
    friend bool operator <(const vkRossiDFP &, const unsigned short);
    friend bool operator <(const vkRossiDFP &, const int);
    friend bool operator <(const vkRossiDFP &, const unsigned int);
    friend bool operator <(const vkRossiDFP &, const long);
    friend bool operator <(const vkRossiDFP &, const unsigned long);
    friend bool operator <(const vkRossiDFP &, const float);
    friend bool operator <(const vkRossiDFP &, const double);
    friend bool operator <(const vkRossiDFP &, const char *);
    friend bool operator <(const vkRossiDFP &, const unsigned char *);
    friend bool operator <(const vkRossiDFP &, const LONG_LONG);
    friend bool operator <(const vkRossiDFP &, const ULONG_LONG);
    friend bool operator <(const char,            const vkRossiDFP &);
    friend bool operator <(const unsigned char,   const vkRossiDFP &);
    friend bool operator <(const short,           const vkRossiDFP &);
    friend bool operator <(const unsigned short,  const vkRossiDFP &);
    friend bool operator <(const int,             const vkRossiDFP &);
    friend bool operator <(const unsigned int,    const vkRossiDFP &);
    friend bool operator <(const long,            const vkRossiDFP &);
    friend bool operator <(const unsigned long,   const vkRossiDFP &);
    friend bool operator <(const float,           const vkRossiDFP &);
    friend bool operator <(const double,          const vkRossiDFP &);
    friend bool operator <(const char *,          const vkRossiDFP &);
    friend bool operator <(const unsigned char *, const vkRossiDFP &);
    friend bool operator <(const LONG_LONG,       const vkRossiDFP &);
    friend bool operator <(const ULONG_LONG,      const vkRossiDFP &);

    friend bool operator <=(const vkRossiDFP &, const vkRossiDFP &);
    friend bool operator <=(const vkRossiDFP &, const char);
    friend bool operator <=(const vkRossiDFP &, const unsigned char);
    friend bool operator <=(const vkRossiDFP &, const short);
    friend bool operator <=(const vkRossiDFP &, const unsigned short);
    friend bool operator <=(const vkRossiDFP &, const int);
    friend bool operator <=(const vkRossiDFP &, const unsigned int);
    friend bool operator <=(const vkRossiDFP &, const long);
    friend bool operator <=(const vkRossiDFP &, const unsigned long);
    friend bool operator <=(const vkRossiDFP &, const float);
    friend bool operator <=(const vkRossiDFP &, const double);
    friend bool operator <=(const vkRossiDFP &, const char *);
    friend bool operator <=(const vkRossiDFP &, const unsigned char *);
    friend bool operator <=(const vkRossiDFP &, const LONG_LONG);
    friend bool operator <=(const vkRossiDFP &, const ULONG_LONG);
    friend bool operator <=(const char,            const vkRossiDFP &);
    friend bool operator <=(const unsigned char,   const vkRossiDFP &);
    friend bool operator <=(const short,           const vkRossiDFP &);
    friend bool operator <=(const unsigned short,  const vkRossiDFP &);
    friend bool operator <=(const int,             const vkRossiDFP &);
    friend bool operator <=(const unsigned int,    const vkRossiDFP &);
    friend bool operator <=(const long,            const vkRossiDFP &);
    friend bool operator <=(const unsigned long,   const vkRossiDFP &);
    friend bool operator <=(const float,           const vkRossiDFP &);
    friend bool operator <=(const double,          const vkRossiDFP &);
    friend bool operator <=(const char *,          const vkRossiDFP &);
    friend bool operator <=(const unsigned char *, const vkRossiDFP &);
    friend bool operator <=(const LONG_LONG,       const vkRossiDFP &);
    friend bool operator <=(const ULONG_LONG,      const vkRossiDFP &);

    friend bool operator >(const vkRossiDFP &, const vkRossiDFP &);
    friend bool operator >(const vkRossiDFP &, const char);
    friend bool operator >(const vkRossiDFP &, const unsigned char);
    friend bool operator >(const vkRossiDFP &, const short);
    friend bool operator >(const vkRossiDFP &, const unsigned short);
    friend bool operator >(const vkRossiDFP &, const int);
    friend bool operator >(const vkRossiDFP &, const unsigned int);
    friend bool operator >(const vkRossiDFP &, const long);
    friend bool operator >(const vkRossiDFP &, const unsigned long);
    friend bool operator >(const vkRossiDFP &, const float);
    friend bool operator >(const vkRossiDFP &, const double);
    friend bool operator >(const vkRossiDFP &, const char *);
    friend bool operator >(const vkRossiDFP &, const unsigned char *);
    friend bool operator >(const vkRossiDFP &, const LONG_LONG);
    friend bool operator >(const vkRossiDFP &, const ULONG_LONG);
    friend bool operator >(const char,            const vkRossiDFP &);
    friend bool operator >(const unsigned char,   const vkRossiDFP &);
    friend bool operator >(const short,           const vkRossiDFP &);
    friend bool operator >(const unsigned short,  const vkRossiDFP &);
    friend bool operator >(const int,             const vkRossiDFP &);
    friend bool operator >(const unsigned int,    const vkRossiDFP &);
    friend bool operator >(const long,            const vkRossiDFP &);
    friend bool operator >(const unsigned long,   const vkRossiDFP &);
    friend bool operator >(const float,           const vkRossiDFP &);
    friend bool operator >(const double,          const vkRossiDFP &);
    friend bool operator >(const char *,          const vkRossiDFP &);
    friend bool operator >(const unsigned char *, const vkRossiDFP &);
    friend bool operator >(const LONG_LONG,       const vkRossiDFP &);
    friend bool operator >(const ULONG_LONG,      const vkRossiDFP &);

    friend bool operator >=(const vkRossiDFP &, const vkRossiDFP &);
    friend bool operator >=(const vkRossiDFP &, const char);
    friend bool operator >=(const vkRossiDFP &, const unsigned char);
    friend bool operator >=(const vkRossiDFP &, const short);
    friend bool operator >=(const vkRossiDFP &, const unsigned short);
    friend bool operator >=(const vkRossiDFP &, const int);
    friend bool operator >=(const vkRossiDFP &, const unsigned int);
    friend bool operator >=(const vkRossiDFP &, const long);
    friend bool operator >=(const vkRossiDFP &, const unsigned long);
    friend bool operator >=(const vkRossiDFP &, const float);
    friend bool operator >=(const vkRossiDFP &, const double);
    friend bool operator >=(const vkRossiDFP &, const char *);
    friend bool operator >=(const vkRossiDFP &, const unsigned char *);
    friend bool operator >=(const vkRossiDFP &, const LONG_LONG);
    friend bool operator >=(const vkRossiDFP &, const ULONG_LONG);
    friend bool operator >=(const char,            const vkRossiDFP &);
    friend bool operator >=(const unsigned char,   const vkRossiDFP &);
    friend bool operator >=(const short,           const vkRossiDFP &);
    friend bool operator >=(const unsigned short,  const vkRossiDFP &);
    friend bool operator >=(const int,             const vkRossiDFP &);
    friend bool operator >=(const unsigned int,    const vkRossiDFP &);
    friend bool operator >=(const long,            const vkRossiDFP &);
    friend bool operator >=(const unsigned long,   const vkRossiDFP &);
    friend bool operator >=(const float,           const vkRossiDFP &);
    friend bool operator >=(const double,          const vkRossiDFP &);
    friend bool operator >=(const char *,          const vkRossiDFP &);
    friend bool operator >=(const unsigned char *, const vkRossiDFP &);
    friend bool operator >=(const LONG_LONG,       const vkRossiDFP &);
    friend bool operator >=(const ULONG_LONG,      const vkRossiDFP &);

    friend const vkRossiDFP abs(const vkRossiDFP &);
//     friend vkRossiDFP   acos(   const vkRossiDFP & );
//     friend vkRossiDFP   asin(   const vkRossiDFP & );
//     friend vkRossiDFP   atan(   const vkRossiDFP & );
//     friend vkRossiDFP   cos(    const vkRossiDFP & );
//     friend vkRossiDFP   cosh(   const vkRossiDFP & );
//     friend vkRossiDFP   exp(    const vkRossiDFP & );
//     friend vkRossiDFP   log(    const vkRossiDFP & );
//     friend vkRossiDFP   maxMP(  const vkRossiDFP &, const vkRossiDFP & );
//     friend vkRossiDFP   mid(    const vkRossiDFP & );
//     friend vkRossiDFP   minMP(  const vkRossiDFP &, const vkRossiDFP & );
//     friend vkRossiDFP   range(  const vkRossiDFP & );
//     friend vkRossiDFP   sin(    const vkRossiDFP & );
//     friend vkRossiDFP   sinh(   const vkRossiDFP & );
    friend const vkRossiDFP sqrt(const vkRossiDFP &);
//     friend vkRossiDFP   tan(    const vkRossiDFP & );
//     friend vkRossiDFP   tanh(   const vkRossiDFP & );
//     friend vkRossiDFP   trunc(  const vkRossiDFP & );
    friend const vkRossiDFP floor(const vkRossiDFP &);
    friend const vkRossiDFP ceil(const vkRossiDFP &);
//     friend vkRossiDFP   round(  const vkRossiDFP &, int );

    friend char * operator !(vkRossiDFP &);
    friend std::ostream & operator <<(std::ostream &, vkRossiDFP &);
};

extern void assignDFP(const dfp & source, dfp & target);
extern void assignvkRossiDFP(const vkRossiDFP & source, vkRossiDFP & target);

#endif //VKROSSIDFP_H