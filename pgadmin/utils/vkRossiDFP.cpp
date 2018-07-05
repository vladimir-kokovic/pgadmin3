//
//Made for PgAdmin by Vladimir Kokovic (vladimir.kokovic@gmail.com)
//Based on Bill Rossi's Decimal Floating Point library for Java (http://dfp.sourceforge.net/)
//

#include "utils/vkRossiDFP.h"

int dfp::rMode = 4; // Current rounding mode ROUND_HALF_EVEN
int dfp::ieeeFlags = 0; // IEEE 854-1987 signals
const dfp dfp::zero;
const dfp dfp::one("1");
const dfp dfp::two("2");

vkSprintf vkRossiDFP::sc;
vkRossiDFP vkRossiDFP::plusminusvar;

const vkRossiDFP vkRossiDFP::ZERO(0);

dfp::dfp()
{
	mant = new int[DIGITS];
	for (int i = DIGITS - 1; i >= 0; i--)
		mant[i] = 0;
	sign = 1;
	exp = 0;
	nans = FINITE;
#ifdef DEBUG_CONSTRUCTOR_DESTRUCTOR
    std::cout << vkRossiDFP::sc.sprintf("dfp constructor 0 this=%16lX mant=%16lX", this, mant) << std::endl;
#endif
}

dfp::~dfp()
{
#ifdef DEBUG_CONSTRUCTOR_DESTRUCTOR
  std::cout << vkRossiDFP::sc.sprintf("dfp destructor    this=%16lX mant=%16lX", this, mant) << std::endl;
#endif
  if (mant)
  {
    delete [] mant;
  }
  mant = 0;
}

dfp::dfp(const dfp & d)
{
	mant = new int[DIGITS];

	for (int i = DIGITS - 1; i >= 0; i--)
		mant[i] = d.mant[i];
	sign = d.sign;
	exp = d.exp;
	nans = d.nans;
#ifdef DEBUG_CONSTRUCTOR_DESTRUCTOR
    std::cout << vkRossiDFP::sc.sprintf("dfp constructor 1 this=%16lX mant=%16lX", this, mant) << std::endl;
#endif
}

dfp::dfp(const std::string & s)
{
	mant = new int[DIGITS];

	dfp r = string2dfp(s);
    for (int i = DIGITS - 1; i >= 0; i--)
		mant[i] = r.mant[i];
	this->exp = r.exp;
	this->sign = r.sign;
	this->nans = r.nans;
#ifdef DEBUG_CONSTRUCTOR_DESTRUCTOR
    std::cout << vkRossiDFP::sc.sprintf("dfp constructor 2 this=%16lX mant=%16lX", this, mant) << std::endl;
#endif
}

void assignDFP(const dfp & source, dfp & target)
{
	if (target.mant) { delete [] target.mant; }
	target.mant = new int[dfp::DIGITS];
	for (int i = dfp::DIGITS - 1; i >= 0; i--)
		target.mant[i] = source.mant[i];
	target.sign = source.sign;
	target.exp = source.exp;
	target.nans = source.nans;
}

dfp & dfp::operator =(const dfp & a)
{
	assignDFP(a, *this);
	return *this;
}

dfp dfp::create(char sign, char nans)
{
	dfp result;
	result.sign = sign;
	result.nans = nans;

	return result;
}

dfp dfp::string2dfp(std::string fpin)
{
	std::string fpdecimal;
	int trailing_zeros;
	int significant_digits;
	int decimal;
	int p, q;
	dfp result;
	bool decimalFound = false;
	int decimalPos = 0; // position of the decimal.
	const int rsize = 4; // size of radix in decimal digits
	const int offset = 4; // Starting offset into Striped
	int sciexp = 0;
	int i;

	std::string Striped(DIGITS * rsize + offset * 2, 'v');

	/* Check some special cases */
	if (fpin == ("Infinite"))
		return create((char) 1, DFP_INFINITE);

	if (fpin == ("-Infinite"))
		return create((char) - 1, DFP_INFINITE);

	if (fpin == ("NaN"))
		return create((char) 1, QNAN);

	result = zero;

	/* Check for scientific notation */
	p = fpin.find("e");
	if (p == std::string::npos) // try upper case?
		p = fpin.find("E");

	if (p != std::string::npos) // scientific notation
	{
		fpdecimal = fpin.substr(0, p);
		std::string fpexp = fpin.substr(p + 1);
		bool negative = false;

		sciexp = 0;
		for (i = 0; i < fpexp.length(); i++)
		{
			if (fpexp[i] == '-')
			{
				negative = true;
				continue;
			}
			if (fpexp[i] >= '0' && fpexp[i] <= '9')
				sciexp = sciexp * 10 + fpexp[i] - '0';
		}

		if (negative)
			sciexp = -sciexp;
	} else // normal case
	{
		fpdecimal = fpin;
	}

	/* If there is a minus sign in the number then it is negative */

	if (fpdecimal.find("-") != std::string::npos)
		result.sign = -1;

	/* First off, find all of the leading zeros, trailing zeros, and
	   siginificant digits */

	p = 0;

	/* Move p to first significant digit */

	for (;;)
	{
		if (fpdecimal[p] >= '1' && fpdecimal[p] <= '9')
			break;

		if (decimalFound && fpdecimal[p] == '0')
			decimalPos--;

		if (fpdecimal[p] == '.')
			decimalFound = true;

		p++;

		if (p == fpdecimal.length())
			break;
	}

	/* Copy the string onto Stripped */

	q = offset;
	Striped[0] = '0';
	Striped[1] = '0';
	Striped[2] = '0';
	Striped[3] = '0';
	significant_digits = 0;
	for (;;)
	{
		if (p == (fpdecimal.length()))
			break;

		// Dont want to run pass the end of the array
		if (q == DIGITS * rsize + offset + 1)
			break;

		if (fpdecimal[p] == '.')
		{
			decimalFound = true;
			decimalPos = significant_digits;
			p++;
			continue;
		}

		if (fpdecimal[p] < '0' || fpdecimal[p] > '9')
		{
			p++;
			continue;
		}

		Striped[q] = fpdecimal[p];
		q++;
		p++;
		significant_digits++;
	}


	// If the decimal point has been found then get rid of trailing zeros.
	if (decimalFound && q != offset)
	{
		for (;;)
		{
			q--;
			if (q == offset)
				break;
			if (Striped[q] == '0')
			{
				significant_digits--;
			} else
			{
				break;
			}
		}
	}

	// special case of numbers like "0.00000"
	if (decimalFound && significant_digits == 0)
		decimalPos = 0;

	// Implicit decimal point at end of number if not present
	if (!decimalFound)
		decimalPos = q - offset;

	/* Find the number of significant trailing zeros */

	q = offset; // set q to point to first sig digit
	p = significant_digits - 1 + offset;

	trailing_zeros = 0;
	while (p > q)
	{
		if (Striped[p] != '0')
			break;
		trailing_zeros++;
		p--;
	}

	/* Make sure the decimal is on a mod 10000 boundary */
	i = (((rsize * 100) - decimalPos - sciexp % rsize) % rsize);
	q -= i;
	decimalPos += i;

	/* Make the mantissa length right by adding zeros at the end if
	   necessary */

	while ((p - q) < (DIGITS * rsize))
	{
		for (i = 0; i < rsize; i++)
			Striped[++p] = '0';
	}

	/* Ok, now we know how many trailing zeros there are, and
	   where the least significant digit is. */

	for (i = (DIGITS - 1); i >= 0; i--)
	{
		result.mant[i] = (Striped[q] - '0')*1000 +
				(Striped[q + 1] - '0')*100 +
				(Striped[q + 2] - '0')*10 +
				(Striped[q + 3] - '0');
		q += 4;
	}


	result.exp = ((decimalPos + sciexp) / rsize);

	if (q < Striped.length()) // Is there possible another digit?
		result.round((Striped[q] - '0')*1000);

	return result;
}

int dfp::round(int n)
{
	int r, rh, rl;
	bool inc = false;

	switch (rMode)
	{
		case ROUND_DOWN:
			inc = false;
			break;

		case ROUND_UP:
			inc = (n != 0); // round up if n!=0
			break;

		case ROUND_HALF_UP:
			inc = (n >= 5000); // round half up
			break;

		case ROUND_HALF_DOWN:
			inc = (n > 5000); // round half down
			break;

		case ROUND_HALF_EVEN:
			inc = (n > 5000 || (n == 5000 && (mant[0]&1) == 1)); // round half-even
			break;

		case ROUND_HALF_ODD:
			inc = (n > 5000 || (n == 5000 && (mant[0]&1) == 0)); // round half-odd
			break;

		case ROUND_CEIL:
			inc = (sign == 1 && n != 0); // round ceil
			break;

		case ROUND_FLOOR:
			inc = (sign == -1 && n != 0); // round floor
			break;
	}

	if (inc) // increment if necessary
	{
		rh = 1;
		for (int i = 0; i < DIGITS; i++)
		{
			r = mant[i] + rh;
			rh = r / radix;
			rl = r % radix;
			mant[i] = rl;
		}

		if (rh != 0)
		{
			shiftRight();
			mant[DIGITS - 1] = rh;
		}
	}

	/* Check for exceptional cases and raise signals if necessary */
	if (exp < minExp) // Gradual Underflow
	{
		ieeeFlags |= FLAG_UNDERFLOW;
		return FLAG_UNDERFLOW;
	}

	if (exp > maxExp) // Overflow
	{
		ieeeFlags |= FLAG_OVERFLOW;
		return FLAG_OVERFLOW;
	}

	if (n != 0) // Inexact
	{
		ieeeFlags |= FLAG_INEXACT;
		return FLAG_INEXACT;
	}
	return 0;
}

void dfp::shiftLeft()
{
	for (int i = DIGITS - 1; i > 0; i--)
		mant[i] = mant[i - 1];
	mant[0] = 0;
	exp--;
}

/* Note that shiftRight() does not call round() as that round() itself uses shiftRight() */
void dfp::shiftRight()
{
	for (int i = 0; i < DIGITS - 1; i++)
		mant[i] = mant[i + 1];
	mant[DIGITS - 1] = 0;
	exp++;
}

int dfp::align(int e)
{
	int diff;
	int adiff;
	int lostdigit = 0;
	bool inexact = false;

	diff = exp - e;

	adiff = diff;
	if (adiff < 0)
		adiff = -adiff;

	if (diff == 0)
		return 0;

	if (adiff > (DIGITS + 1)) // Special case
	{
		for (int i = DIGITS - 1; i >= 0; i--)
			mant[i] = 0;
		exp = e;

		ieeeFlags |= FLAG_INEXACT;
		dotrap(FLAG_INEXACT, "align", this, this);

		return 0;
	}

	for (int i = 0; i < adiff; i++)
	{
		if (diff < 0)
		{
			/* Keep track of loss -- only signal inexact after losing 2 digits.
			 * the first lost digit is returned to add() and may be incorporated
			 * into the result.
			 */
			if (lostdigit != 0)
				inexact = true;

			lostdigit = mant[0];

			shiftRight();
		} else
			shiftLeft();
	}

	if (inexact)
	{
		ieeeFlags |= FLAG_INEXACT;
		dotrap(FLAG_INEXACT, "align", this, this);
	}

	return lostdigit;
}

dfp dfp::dotrap(int type, const char *what, dfp *oper, dfp *result)
{
	dfp def(zero);
	std::string x;

	switch (type)
	{
		case FLAG_INVALID:
            def.sign = result->sign;
            def.nans = QNAN;
            break;

        case FLAG_DIV_ZERO:
			if (nans == FINITE && mant[DIGITS-1] != 0)  // normal case, we are finite, non-zero
            {
              def.sign = sign * oper->sign;
              def.nans = DFP_INFINITE;
            }

            if (nans == FINITE && mant[DIGITS-1] == 0)  //  0/0
            {
              def.nans = QNAN;
            }

            if (nans == DFP_INFINITE || nans == QNAN)
            {
              def.nans = QNAN;
            }

            if (nans == DFP_INFINITE || nans == SNANDFP)
            {
              def.nans = QNAN;
            }
            break;

        case FLAG_UNDERFLOW:
			if ((result->exp + DIGITS) < minExp)
            {
              def.sign = result->sign;
            }
            else
            {
              def = dfp(*result);  // gradual underflow
            }
            result->exp = result->exp + errScale;
            break;

        case FLAG_OVERFLOW:
			result->exp = result->exp - errScale;
            def.sign = result->sign;
            def.nans = DFP_INFINITE;
            break;

        default: def = *result;
			break;
	}

	return trap(type, what, oper, &def, result);
}

dfp dfp::trap(int type, const char *what, dfp *oper, dfp *def, dfp *result)
{
	return *def;
}

int dfp::getIEEEFlags()
{
	return ieeeFlags;
}

void dfp::clearIEEEFlags()
{
	ieeeFlags = 0;
}

void dfp::setIEEEFlags(int flags)
{
	ieeeFlags = flags;
}

int dfp::classify()
{
	return nans;
}

std::string dfp::toString()
{
	if (nans != FINITE) // if non-finite exceptional cases
	{
		switch (sign * nans)
		{
			case DFP_INFINITE: return "Infinity";
			case -DFP_INFINITE: return "-Infinity";
			case QNAN: return "NaN";
			case -QNAN: return "NaN";
			case SNANDFP: return "NaN";
			case -SNANDFP: return "NaN";
		}
	}

	if (exp > DIGITS || exp < -1)
		return dfp2sci(*this);

	return dfp2string(*this);
}

/* Convert a dfp to a string using scientific notation */
std::string dfp::dfp2sci(dfp & a)
{
	std::string rawdigits(DIGITS * 4, 'v');
	std::string outputbuffer(DIGITS * 4 + 20, 'v');
	int p;
	int q;
	int e;
	int ae;
	int shf;


	/* Get all the digits */
	p = 0;
	for (int i = DIGITS - 1; i >= 0; i--)
	{
		rawdigits[p++] = (char) ((a.mant[i] / 1000) + '0');
		rawdigits[p++] = (char) (((a.mant[i] / 100) % 10) + '0');
		rawdigits[p++] = (char) (((a.mant[i] / 10) % 10) + '0');
		rawdigits[p++] = (char) (((a.mant[i]) % 10) + '0');
	}

	/* find the first non-zero one */
	for (p = 0; p < rawdigits.length(); p++)
		if (rawdigits[p] != '0')
			break;
	shf = p;

	/* Now do the conversion */
	q = 0;
	if (a.sign == -1)
		outputbuffer[q++] = '-';

	if (p != rawdigits.length()) // there are non zero digits...
	{
		outputbuffer[q++] = rawdigits[p++];
		outputbuffer[q++] = '.';

		while (p < rawdigits.length())
			outputbuffer[q++] = rawdigits[p++];
	} else
	{
		outputbuffer[q++] = '0';
		outputbuffer[q++] = '.';
		outputbuffer[q++] = '0';
		outputbuffer[q++] = 'e';
		outputbuffer[q++] = '0';
		return outputbuffer.substr(0, 5);
	}

	outputbuffer[q++] = 'e';

	/* find the msd of the exponent */

	e = a.exp * 4 - shf - 1;
	ae = e;
	if (e < 0)
		ae = -e;

	/* Find the largest p such that p < e */
	bool minusjedodat = false;
	for (p = 1000000000; p > ae; p /= 10)
	{

		if (e < 0 && !minusjedodat)
		{
			outputbuffer[q++] = '-';
			minusjedodat = true;
		}
	}

	while (p > 0)
	{
		outputbuffer[q++] = (char) (ae / p + '0');
		ae = ae % p;
		p = p / 10;
	}

	return outputbuffer.substr(0, q);
}

/* converts a dfp to a string handling the normal case */
std::string dfp::dfp2string(dfp & a)
{
	std::string buffer(DIGITS * 4 + 1024 + 3, 'v');
	int p = 1;
	int q;
	int e = a.exp;
	bool pointInserted = false;

	buffer[0] = ' ';

	if (e <= 0)
	{
		buffer[p++] = '0';
		buffer[p++] = '.';
		pointInserted = true;
	}

	while (e < 0)
	{
		buffer[p++] = '0';
		buffer[p++] = '0';
		buffer[p++] = '0';
		buffer[p++] = '0';
		e++;
	}

	for (int i = DIGITS - 1; i >= 0; i--)
	{
		buffer[p++] = (char) ((a.mant[i] / 1000) + '0');
		buffer[p++] = (char) (((a.mant[i] / 100) % 10) + '0');
		buffer[p++] = (char) (((a.mant[i] / 10) % 10) + '0');
		buffer[p++] = (char) (((a.mant[i]) % 10) + '0');
		if (--e == 0)
		{
			buffer[p++] = '.';
			pointInserted = true;
		}
	}

	while (e > 0)
	{
		buffer[p++] = '0';
		buffer[p++] = '0';
		buffer[p++] = '0';
		buffer[p++] = '0';
		e--;
	}

	if (!pointInserted) /* Ensure we have a radix point! */
		buffer[p++] = '.';

	/* Supress leading zeros */
	q = 1;
	while (buffer[q] == '0')
		q++;
	if (buffer[q] == '.')
		q--;

	/* Suppress trailing zeros */
	while (buffer[p - 1] == '0')
		p--;

	/* Insert sign */
	if (a.sign < 0)
		buffer[--q] = '-';

	return buffer.substr(q, p - q);
}

dfp dfp::copysign(dfp x, dfp y)
{
	dfp result(x);
	result.sign = y.sign;
	return result;
}

bool dfp::lessThan(dfp & x)
{
	/* if a nan is involved, signal invalid and return false */
	if (nans == SNANDFP || nans == QNAN || x.nans == SNANDFP || x.nans == QNAN)
	{
		ieeeFlags |= FLAG_INVALID;
		dfp zero1(zero);
		dotrap(FLAG_INVALID, "lessThan", &x, &zero1);
		return false;
	}

	return (compare(*this, x) < 0);
}

bool dfp::greaterThan(dfp & x)
{
	/* if a nan is involved, signal invalid and return false */
	if (nans == SNANDFP || nans == QNAN || x.nans == SNANDFP || x.nans == QNAN)
	{
		ieeeFlags |= FLAG_INVALID;
		dfp zero1(zero);
		dotrap(FLAG_INVALID, "lessThan", &x, &zero1);
		return false;
	}

	return (compare(*this, x) > 0);
}

bool dfp::equalDFP(dfp & x)
{
	if (nans == SNANDFP || nans == QNAN || x.nans == SNANDFP || x.nans == QNAN)
		return false;

	return (compare(*this, x) == 0);
}

bool dfp::unequalDFP(dfp & x)
{
	if (nans == SNANDFP || nans == QNAN || x.nans == SNANDFP || x.nans == QNAN)
		return false;

	return (greaterThan(x) || lessThan(x));
}

int dfp::compare(dfp & a, dfp & b)
{
	/* Ignore the sign of zero */
	if (a.mant[DIGITS - 1] == 0 && b.mant[DIGITS - 1] == 0 && a.nans == FINITE && b.nans == FINITE)
		return 0;

	if (a.sign != b.sign)
	{
		if (a.sign == -1)
			return -1;
		else
			return 1;
	}

	/* deal with the infinities */
	if (a.nans == DFP_INFINITE && b.nans == FINITE)
		return a.sign;

	if (a.nans == FINITE && b.nans == DFP_INFINITE)
		return -b.sign;

	if (a.nans == DFP_INFINITE && b.nans == DFP_INFINITE)
		return 0;

	/* Handle special case when a or b is zero, by ignoring the exponents */
	if (b.mant[DIGITS - 1] != 0 && a.mant[DIGITS - 1] != 0)
	{
		if (a.exp < b.exp)
			return -a.sign;

		if (a.exp > b.exp)
			return a.sign;
	}

	/* compare the mantissas */
	for (int i = DIGITS - 1; i >= 0; i--)
	{
		if (a.mant[i] > b.mant[i])
			return a.sign;

		if (a.mant[i] < b.mant[i])
			return -a.sign;
	}

	return 0;
}

int dfp::complement(int extra)
{
	int r, rl, rh;

	extra = radix - extra;
	for (int i = 0; i < DIGITS; i++)
		mant[i] = radix - mant[i] - 1;

	rh = extra / radix;
	extra = extra % radix;
	for (int i = 0; i < DIGITS; i++)
	{
		r = mant[i] + rh;
		rl = r % radix;
		rh = r / radix;
		mant[i] = rl;
	}

	return extra;
}

dfp dfp::add(dfp x)
{
	int r, rh, rl;
	dfp a, b, result;
	char asign, bsign, rsign;
	int aextradigit = 0, bextradigit = 0;

	/* handle special cases */
	if (nans != FINITE || x.nans != FINITE)
	{
		if (nans == QNAN || nans == SNANDFP)
			return *this;

		if (x.nans == QNAN || x.nans == SNANDFP)
			return x;

		if (nans == DFP_INFINITE && x.nans == FINITE)
			return *this;

		if (x.nans == DFP_INFINITE && nans == FINITE)
			return x;

		if (x.nans == DFP_INFINITE && nans == DFP_INFINITE && sign == x.sign)
			return x;

		if (x.nans == DFP_INFINITE && nans == DFP_INFINITE && sign != x.sign)
		{
			ieeeFlags |= FLAG_INVALID;
			result = zero;
			result.nans = QNAN;
			result = dotrap(FLAG_INVALID, "add", &x, &result);
			return result;
		}
	}

	/* copy this and the arg */
	a = *this;
	b = x;

	/* initialize the result object */
	result = zero;

	/* Make all numbers positive, but remember their sign */
	asign = a.sign;
	bsign = b.sign;

	a.sign = 1;
	b.sign = 1;

	/* The result will be signed like the arg with greatest magnitude */
	rsign = bsign;
	if (compare(a, b) > 0)
		rsign = asign;

	/* Handle special case when a or b is zero, by setting the exponent
	   of the zero number equalDFP to the other one.  This avoids an alignment
	   which would cause catastropic loss of precision */
	if (b.mant[DIGITS - 1] == 0)
		b.exp = a.exp;

	if (a.mant[DIGITS - 1] == 0)
		a.exp = b.exp;

	/* align number with the smaller exponent */
	if (a.exp < b.exp)
		aextradigit = a.align(b.exp);
	else
		bextradigit = b.align(a.exp);

	/* complement the smaller of the two if the signs are different */
	if (asign != bsign)
	{
		if (asign == rsign)
			bextradigit = b.complement(bextradigit);
		else
			aextradigit = a.complement(aextradigit);
	}

	/* add the mantissas */
	rh = 0; /* acts as a carry */
	for (int i = 0; i < DIGITS; i++)
	{
		r = a.mant[i] + b.mant[i] + rh;
		rl = r % radix;
		rh = r / radix;
		result.mant[i] = rl;
	}
	result.exp = a.exp;
	result.sign = rsign;

	//System.out.println("a = "+a.mant[2]+" "+a.mant[1]+" "+a.mant[0]);
	//System.out.println("b = "+b.mant[2]+" "+b.mant[1]+" "+b.mant[0]);
	//System.out.println("result = "+result.mant[2]+" "+result.mant[1]+" "+result.mant[0]);

	/* handle overflow -- note, when asign!=bsign an overflow is
	 * normal and should be ignored.  */

	if (rh != 0 && (asign == bsign))
	{
		int lostdigit = result.mant[0];
		result.shiftRight();
		result.mant[DIGITS - 1] = rh;
		int excp = result.round(lostdigit);
		if (excp != 0)
			result = dotrap(excp, "add", &x, &result);
	}

	/* normalize the result */
	for (int i = 0; i < DIGITS; i++)
	{
		if (result.mant[DIGITS - 1] != 0)
			break;
		result.shiftLeft();
		if (i == 0)
		{
			result.mant[0] = aextradigit + bextradigit;
			aextradigit = 0;
			bextradigit = 0;
		}
	}

	/* result is zero if after normalization the most sig. digit is zero */
	if (result.mant[DIGITS - 1] == 0)
	{
		result.exp = 0;

		if (asign != bsign) // Unless adding 2 negative zeros, sign is positive
			result.sign = 1; // Per IEEE 854-1987 Section 6.3
	}

	/* Call round to test for over/under flows */
	int excp = result.round((aextradigit + bextradigit));
	if (excp != 0)
		result = dotrap(excp, "add", &x, &result);

	return result;
}

dfp dfp::negate()
{
	dfp result = *this;
	result.sign = -result.sign;
	return result;
}

dfp dfp::subtract(dfp a)
{
	dfp x = a.negate();
	dfp y = this->add(x);
	return y;
}

dfp dfp::nextAfter(dfp x)
{
	bool up = false;
	dfp result, inc, zero1(zero);

	// if this is greater than x
	if (this->lessThan(x))
		up = true;

	if (compare(*this, x) == 0)
	{
		result = x;
		return result;
	}

	if (this->lessThan(zero1))
		up = !up;

	if (up)
	{
		inc = one;
		inc.exp = this->exp - DIGITS + 1;
		inc.sign = this->sign;

		if (this->equalDFP(zero1))
			inc.exp = minExp - DIGITS;

		result = this->add(inc);
	} else
	{
		inc = one;
		inc.exp = this->exp;
		inc.sign = this->sign;

		if (this->equalDFP(inc))
			inc.exp = this->exp - DIGITS;
		else
			inc.exp = this->exp - DIGITS + 1;

		if (this->equalDFP(zero1))
			inc.exp = minExp - DIGITS;

		result = this->subtract(inc);
	}

	if (result.classify() == DFP_INFINITE && this->classify() != DFP_INFINITE)
	{
		ieeeFlags |= FLAG_INEXACT;
		result = dotrap(FLAG_INEXACT, "nextAfter", &x, &result);
	}

	if (result.equalDFP(zero1) && this->equalDFP(zero1) == false)
	{
		ieeeFlags |= FLAG_INEXACT;
		result = dotrap(FLAG_INEXACT, "nextAfter", &x, &result);
	}

	return result;
}

dfp dfp::multiply(dfp x)
{
	int *product; // product array
	int r, rh, rl; // working registers
	int md = 0; // most sig digit in result
	int excp; // exception code if any
	dfp result(zero);

	/* handle special cases */
	if (nans != FINITE || x.nans != FINITE)
	{
		if (nans == QNAN || nans == SNANDFP)
			return *this;

		if (x.nans == QNAN || x.nans == SNANDFP)
			return x;

		if (nans == DFP_INFINITE && x.nans == FINITE && x.mant[DIGITS - 1] != 0)
		{
			result = *this;
			result.sign = (sign * x.sign);
			return result;
		}

		if (x.nans == DFP_INFINITE && nans == FINITE && mant[DIGITS - 1] != 0)
		{
			result = x;
			result.sign = (sign * x.sign);
			return result;
		}

		if (x.nans == DFP_INFINITE && nans == DFP_INFINITE)
		{
			result = *this;
			result.sign = (sign * x.sign);
			return result;
		}

		if ((x.nans == DFP_INFINITE && nans == FINITE && mant[DIGITS - 1] == 0) ||
				(nans == DFP_INFINITE && x.nans == FINITE && x.mant[DIGITS - 1] == 0))
		{
			ieeeFlags |= FLAG_INVALID;
			result = zero;
			result.nans = QNAN;
			result = dotrap(FLAG_INVALID, "multiply", &x, &result);
			return result;
		}
	}

	product = new int[DIGITS * 2]; // Big enough to hold even the largest result

	for (int i = 0; i < DIGITS * 2; i++)
		product[i] = 0;

	for (int i = 0; i < DIGITS; i++)
	{
		rh = 0; // acts as a carry
		for (int j = 0; j < DIGITS; j++)
		{
			r = mant[i] * x.mant[j]; // multiply the 2 digits
			r = r + product[i + j] + rh; // add to the product digit with carry in
			rl = r % radix;
			rh = r / radix;
			product[i + j] = rl;
		}
		product[i + DIGITS] = rh;
	}

	/* Find the most sig digit */
	md = DIGITS * 2 - 1; // default, in case result is zero
	for (int i = DIGITS * 2 - 1; i >= 0; i--)
	{
		if (product[i] != 0)
		{
			md = i;
			break;
		}
	}

	/* Copy the digits into the result */
	for (int i = 0; i < DIGITS; i++)
		result.mant[DIGITS - i - 1] = product[md - i];

	/* Fixup the exponent. */
	result.exp = (exp + x.exp + md - 2 * DIGITS + 1);
	result.sign = ((sign == x.sign) ? 1 : -1);

	if (result.mant[DIGITS - 1] == 0) // if result is zero, set exp to zero
		result.exp = 0;

	if (md > (DIGITS - 1))
		excp = result.round(product[md - DIGITS]);
	else
		excp = result.round(0); // has no effect except to check status

	if (excp != 0)
		result = dotrap(excp, "multiply", &x, &result);

	return result;
}

dfp dfp::multiply(int x)
{
	int r, rh, rl; // working registers
	int excp; // exception code if any
	int lostdigit; // rounded off digit
	dfp result = *this;

	/* handle special cases */
	if (nans != FINITE)
	{
		if (nans == QNAN || nans == SNANDFP)
			return *this;

		if (nans == DFP_INFINITE && x != 0)
		{
			result = *this;
			return result;
		}

		if (nans == DFP_INFINITE && x == 0)
		{
			ieeeFlags |= FLAG_INVALID;
			result = zero;
			result.nans = QNAN;
			dfp zero1 = zero;
			result = dotrap(FLAG_INVALID, "multiply", &zero1, &result);
			return result;
		}
	}

	/* range check x */
	if (x < 0 || x >= radix)
	{
		ieeeFlags |= FLAG_INVALID;
		result = zero;
		result.nans = QNAN;
		result = dotrap(FLAG_INVALID, "multiply", &result, &result);
		return result;
	}

	rh = 0;
	for (int i = 0; i < DIGITS; i++)
	{
		r = mant[i] * x + rh;
		rl = r % radix;
		rh = r / radix;
		result.mant[i] = rl;
	}

	lostdigit = 0;
	if (rh != 0)
	{
		lostdigit = result.mant[0];
		result.shiftRight();
		result.mant[DIGITS - 1] = rh;
	}

	if (result.mant[DIGITS - 1] == 0) // if result is zero, set exp to zero
		result.exp = 0;

	excp = result.round(lostdigit);

	if (excp != 0)
		result = dotrap(excp, "multiply", &result, &result);

	return result;
}

dfp dfp::divide(dfp divisor)
{
	int *dividend; // current status of the dividend
	int *quotient; // quotient
	int *remainder; // remainder
	int qd; // current quotient digit we're working with
	int nsqd; // number of significant quotient digits we have
	int trial = 0; // trial quotient digit
	int min, max; // quotient digit limits
	int minadj; // minimum adjustment
	bool trialgood; // Flag to indicate a good trail digit
	int r, rh, rl; // working registers
	int md = 0; // most sig digit in result
	int excp; // exceptions
	dfp result = zero;

	/* handle special cases */
	if (nans != FINITE || divisor.nans != FINITE)
	{
		if (nans == QNAN || nans == SNANDFP)
			return *this;

		if (divisor.nans == QNAN || divisor.nans == SNANDFP)
			return divisor;

		if (nans == DFP_INFINITE && divisor.nans == FINITE)
		{
			result = *this;
			result.sign = (sign * divisor.sign);
			return result;
		}

		if (divisor.nans == DFP_INFINITE && nans == FINITE)
		{
			result = zero;
			result.sign = (sign * divisor.sign);
			return result;
		}

		if (divisor.nans == DFP_INFINITE && nans == DFP_INFINITE)
		{
			ieeeFlags |= FLAG_INVALID;
			result = zero;
			result.nans = QNAN;
			result = dotrap(FLAG_INVALID, "divide", &divisor, &result);
			return result;
		}
	}

	/* Test for divide by zero */
	if (divisor.mant[DIGITS - 1] == 0)
	{
		ieeeFlags |= FLAG_DIV_ZERO;
		result = zero;
		result.sign = (sign * divisor.sign);
		result.nans = DFP_INFINITE;
		result = dotrap(FLAG_DIV_ZERO, "divide", &divisor, &result);
		return result;
	}

	dividend = new int[DIGITS + 1]; // one extra digit needed
	quotient = new int[DIGITS + 2]; // two extra digits needed 1 for overflow, 1 for rounding
	remainder = new int[DIGITS + 1]; // one extra digit needed

	/* Initialize our most significat digits to zero */

	dividend[DIGITS] = 0;
	quotient[DIGITS] = 0;
	quotient[DIGITS + 1] = 0;
	remainder[DIGITS] = 0;

	/* copy our mantissa into the dividend, initialize the
	   quotient while we are at it */

	for (int i = 0; i < DIGITS; i++)
	{
		dividend[i] = mant[i];
		quotient[i] = 0;
		remainder[i] = 0;
	}

	/* outer loop.  Once per quotient digit */
	nsqd = 0;
	for (qd = DIGITS + 1; qd >= 0; qd--)
	{
		/* Determine outer limits of our quotient digit */

		// r =  most sig 2 digits of dividend
		r = dividend[DIGITS] * radix + dividend[DIGITS - 1];
		min = r / (divisor.mant[DIGITS - 1] + 1);
		max = (r + 1) / divisor.mant[DIGITS - 1];

		trialgood = false;

		while (!trialgood)
		{
			// try the mean
			trial = (min + max) / 2;

			//System.out.println("dividend = "+dividend[2]+" "+dividend[1]+" "+dividend[0]);
			//System.out.println("divisor = "+divisor.mant[1]+" "+divisor.mant[0]);
			//System.out.println("min = "+min+"  max = "+max+"  trial = "+trial);

			/* Multiply by divisor and store as remainder */
			rh = 0;
			for (int i = 0; i < (DIGITS + 1); i++)
			{
				int dm = (i < DIGITS) ? divisor.mant[i] : 0;
				r = (dm * trial) + rh;

				rh = r / radix;
				rl = r % radix;
				remainder[i] = rl;
			}

			//System.out.println("  *remainder = "+remainder[1]+" "+remainder[0]);

			/* subtract the remainder from the dividend */
			rh = 1; // carry in to aid the subtraction
			for (int i = 0; i < (DIGITS + 1); i++)
			{
				r = ((radix - 1) - remainder[i]) + dividend[i] + rh;
				rh = r / radix;
				rl = r % radix;
				remainder[i] = rl;
			}
			//System.out.println("  +remainder = "+remainder[1]+" "+remainder[0]);

			/* Lets analyse what we have here */
			if (rh == 0) // trial is too big -- negative remainder
			{
				max = trial - 1;
				//System.out.println("neg remainder");
				//System.out.println("  remainder = "+remainder[1]+" "+remainder[0]);
				//System.out.println("  rh = "+rh);
				continue;
			}

			/* find out how far off the remainder is telling us we are */
			minadj = (remainder[DIGITS] * radix) + remainder[DIGITS - 1];
			minadj = minadj / (divisor.mant[DIGITS - 1] + 1);

			//System.out.println("minadj = "+minadj);

			if (minadj >= 2)
			{
				min = trial + minadj; // update the minium
				continue;
			}

			/* May have a good one here, check more thoughly.  Basically
			   its a good one if it is less than the divisor */
			trialgood = false; // assume false
			for (int i = (DIGITS - 1); i >= 0; i--)
			{
				if (divisor.mant[i] > remainder[i])
					trialgood = true;
				if (divisor.mant[i] < remainder[i])
					break;
			}

			//System.out.println("remainder = "+remainder[1]+" "+remainder[0]);
			if (remainder[DIGITS] != 0)
				trialgood = false;

			if (trialgood == false)
				min = trial + 1;
		}

		/* Great we have a digit! */
		quotient[qd] = trial;
		if (trial != 0 || nsqd != 0)
			nsqd++;

		if (rMode == ROUND_DOWN && nsqd == DIGITS) // We have enough for this mode
			break;

		if (nsqd > DIGITS) // We have enough digits
			break;

		/* move the remainder into the dividend while left shifting */
		dividend[0] = 0;
		for (int i = 0; i < DIGITS; i++)
			dividend[i + 1] = remainder[i];
	}

	/* Find the most sig digit */
	md = DIGITS; // default
	for (int i = DIGITS + 1; i >= 0; i--)
	{
		if (quotient[i] != 0)
		{
			md = i;
			break;
		}
	}

	//System.out.println("quotient = "+quotient[2]+" "+quotient[1]+" "+quotient[0]);

	/* Copy the digits into the result */
	for (int i = 0; i < DIGITS; i++)
		result.mant[DIGITS - i - 1] = quotient[md - i];

	/* Fixup the exponent. */
	result.exp = (exp - divisor.exp + md - DIGITS + 1 - 1);
	result.sign = ((sign == divisor.sign) ? 1 : -1);

	if (result.mant[DIGITS - 1] == 0) // if result is zero, set exp to zero
		result.exp = 0;

	if (md > (DIGITS - 1))
		excp = result.round(quotient[md - DIGITS]);
	else
		excp = result.round(0);

	if (excp != 0)
		result = dotrap(excp, "divide", &divisor, &result);

	return result;
}

dfp dfp::divide(int divisor)
{
	dfp result;
	int r, rh, rl;
	int excp;

	/* handle special cases */
	if (nans != FINITE)
	{
		if (nans == QNAN || nans == SNANDFP)
			return *this;

		if (nans == DFP_INFINITE)
		{
			result = *this;
			return result;
		}
	}

	/* Test for divide by zero */
	if (divisor == 0)
	{
		ieeeFlags |= FLAG_DIV_ZERO;
		result = zero;
		result.sign = sign;
		result.nans = DFP_INFINITE;
		dfp zero1 = zero;
		result = dotrap(FLAG_DIV_ZERO, "divide", &zero1, &result);
		return result;
	}

	/* range check divisor */
	if (divisor < 0 || divisor >= radix)
	{
		ieeeFlags |= FLAG_INVALID;
		result = zero;
		result.nans = QNAN;
		result = dotrap(FLAG_INVALID, "divide", &result, &result);
		return result;
	}

	result = *this;
	rl = 0;
	for (int i = DIGITS - 1; i >= 0; i--)
	{
		r = rl * radix + result.mant[i];
		rh = r / divisor;
		rl = r % divisor;
		result.mant[i] = rh;
	}

	if (result.mant[DIGITS - 1] == 0) // normalize
	{
		result.shiftLeft();
		r = rl*radix; // compute the next digit and put it in
		rh = r / divisor;
		rl = r % divisor;
		result.mant[0] = rh;
	}

	excp = result.round(rl * radix / divisor); // do the rounding

	if (excp != 0)
		result = dotrap(excp, "divide", &result, &result);

	return result;
}

dfp dfp::sqrt() /* returns the square root of this */
{
	dfp x, dx, px;

	/* check for unusual cases */
	if (nans == FINITE && mant[DIGITS - 1] == 0) // if zero
		return *this;

	if (nans != FINITE)
	{
		if (nans == DFP_INFINITE && sign == 1) // if positive infinity
			return *this;

		if (nans == QNAN)
			return *this;

		if (nans == SNANDFP)
		{
			dfp result;

			ieeeFlags |= FLAG_INVALID;
			result = *this;
			result = dotrap(FLAG_INVALID, "sqrt", 0, &result);
			return result;
		}
	}

	if (sign == -1) // if negative
	{
		dfp result;

		ieeeFlags |= FLAG_INVALID;
		result = *this;
		result.nans = QNAN;
		result = dotrap(FLAG_INVALID, "sqrt", 0, &result);
		return result;
	}

	x = *this;

	/* Lets make a reasonable guess as to the size of the square root */
	if (x.exp < -1 || x.exp > 1)
		x.exp = (this->exp / 2);

	/* Coarsely estimate the mantissa */
	switch (x.mant[DIGITS - 1] / 2000)
	{
		case 0: x.mant[DIGITS - 1] = x.mant[DIGITS - 1] / 2 + 1;
			break;
		case 2: x.mant[DIGITS - 1] = 1500;
			break;
		case 3: x.mant[DIGITS - 1] = 2200;
			break;
		case 4: x.mant[DIGITS - 1] = 3000;
			break;
	}

	dx = x;

	/* Now that we have the first pass estimiate, compute the rest
	   by the formula dx = (y - x*x) / (2x); */

	px = zero;
	while (x.unequalDFP(px))
	{
		dx = x;
		dx.sign = -1;
		dfp vk1 = this->divide(x);
		dx = dx.add(vk1);
		dx = dx.divide(2);
		px = x;
		x = x.add(dx);

		// if dx is zero, break.  Note testing the most sig digit
		// is a sufficient test since dx is normalized
		if (dx.mant[DIGITS - 1] == 0)
			break;
	}

	return x;
}

dfp dfp::rint()
{
	return trunc(ROUND_HALF_EVEN);
}

dfp dfp::floor()
{
	return trunc(ROUND_FLOOR);
}

dfp dfp::ceil()
{
	return trunc(ROUND_CEIL);
}

dfp dfp::remainder(dfp & d)
{
	dfp q = this->divide(d);
	dfp result;

	dfp vk1 = this->divide(d);
	dfp vk2 = vk1.rint().multiply(d);
	result = this->subtract(vk2);

	/* IEEE 854-1987 says that if the result is zero, then it
	   carries the sign of this */

	if (result.mant[DIGITS - 1] == 0)
		result.sign = sign;

	return result;
}

dfp dfp::trunc(int rmode)
{
	dfp result, a, half;
	bool changed = false;

	if (nans == SNANDFP || nans == QNAN)
		return *this;

	if (nans == DFP_INFINITE)
		return *this;

	if (mant[DIGITS - 1] == 0) // a is zero
		return *this;

	/* If the exponent is less than zero then we can certainly
	 * return zero */
	if (exp < 0)
	{
		ieeeFlags |= FLAG_INEXACT;
		result = zero;
		result = dotrap(FLAG_INEXACT, "trunc", this, &result);
		return result;
	}

	/* If the exponent is greater than or equalDFP to digits, then it
	 * must already be an integer since there is no precision left
	 * for any fractional part */

	if (exp >= DIGITS)
		return *this;

	/* General case:  create another dfp, result, that contains the
	 * a with the fractional part lopped off.  */

	result = *this;
	for (int i = 0; i < (DIGITS - result.exp); i++)
	{
		changed |= (result.mant[i] != 0);
		result.mant[i] = 0;
	}

	if (changed)
	{
		switch (rmode)
		{
			case ROUND_FLOOR:
				if (result.sign == -1) // then we must increment the mantissa by one
				{
					dfp vk1("-1");
					result = result.add(vk1);
				}
				break;

			case ROUND_CEIL:
				if (result.sign == 1) // then we must increment the mantissa by one
				{
					dfp vk1 = one;
					result = result.add(vk1);
				}
				break;

			case ROUND_HALF_EVEN:
			default:
				half = dfp("0.5");
				a = subtract(result); // difference between this and result
				a.sign = 1; // force positive (take abs)
				if (a.greaterThan(half))
				{
					a = one;
					a.sign = sign;
					result = result.add(a);
				}

				/** If exactly equal to 1/2 and odd then increment */
				if (a.equalDFP(half) && result.exp > 0 && (result.mant[DIGITS - result.exp]&1) != 0)
				{
					a = one;
					a.sign = sign;
					result = result.add(a);
				}
				break;
		}

		ieeeFlags |= FLAG_INEXACT; // signal inexact
		result = dotrap(FLAG_INEXACT, "trunc", this, &result);
		return result;
	}

	return result;
}

int dfp::intValue()
{
	dfp rounded;
	int result = 0;

	rounded = rint();

	dfp vk1("2147483647");
	if (rounded.greaterThan(vk1))
		return 2147483647;

	dfp vk2("-2147483648");
	if (rounded.lessThan(vk2))
		return -2147483648;

	for (int i = DIGITS - 1; i >= DIGITS - rounded.exp; i--)
		result = result * radix + rounded.mant[i];

	if (rounded.sign == -1)
		result = -result;

	return result;
}

int dfp::log10K()
{
	return exp - 1;
}

dfp dfp::power10K(int e)
{
	dfp d = one;
	d.exp = e + 1;
	return d;
}

int dfp::log10()
{
	if (mant[DIGITS - 1] > 1000) return exp * 4 - 1;
	if (mant[DIGITS - 1] > 100) return exp * 4 - 2;
	if (mant[DIGITS - 1] > 10) return exp * 4 - 3;
	return exp * 4 - 4;
}

dfp dfp::power10(int e)
{
	dfp d = one;

	if (e >= 0)
		d.exp = e / 4 + 1;
	else
		d.exp = (e + 1) / 4;

	switch ((e % 4 + 4) % 4)
	{
		case 0:
			break;
		case 1: d = d.multiply(10);
			break;
		case 2: d = d.multiply(100);
			break;
		case 3: d = d.multiply(1000);
			break;
	}

	return d;
}

void dfp::setRoundingMode(int mode)
{
	rMode = mode;
}

int dfp::getRoundingMode()
{
	return rMode;
}

/**
 * C++ version 0.4 std::string style "itoa":
 * Contributions from Stuart Lowe, Ray-Yuan Sheu,
 * http://www.strudel.org.uk/itoa/
 * Rodrigo de Salvo Braz, Luc Gallant, John Maloney and Brian Hunt
 */
std::string itoa(int value, int base)
{
	std::string buf;

	// check that the base if valid
	if (base < 2 || base > 16) return buf;

	enum
	{
		kMaxDigits = 35
	};
	buf.reserve(kMaxDigits); // Pre-allocate enough space.

	int quotient = value;

	// Translating number to string with base:
	do
	{
		buf += "0123456789abcdef"[ std::abs(quotient % base) ];
		quotient /= base;
	} while (quotient);

	// Append the negative sign
	if (value < 0) buf += '-';

	std::reverse(buf.begin(), buf.end());
	return buf;
}

std::string itoa10(int value)
{
	return itoa(value, 10);
}

vkSprintf::vkSprintf() { s = NULL; }

vkSprintf::~vkSprintf() { if (s)  free(s); s = NULL; }

char *vkSprintf::vsprintf(const char *format, va_list argPtr)
{
    int size = 512, x = -1;
    while (x == -1)
    {
        if (s)  free(s);
        if (NULL == (s = malloc(size + 1)))  return (char *)"";
#ifndef WIN32
        x = vsnprintf((char *)s, size, format, argPtr);
#else
        x = _vsnprintf((char *)s, size, format, argPtr);
#endif
        if ((x != -1) && ((x + 1) <= size))
        {
            if ((x + 1) < size)  s = realloc(s, x + 1);
            return (s) ? (char *)s : (char *)"";
        }
        size *= 2;  x = -1;
    }
    return (char *)"";
}

char *vkSprintf::sprintf(const char *format, ...)
{
    va_list argPtr;
    va_start(argPtr, format);
    char *x = vsprintf(format, argPtr);
    va_end(argPtr);
    return x;
}

char *vkSprintf::sprintfFD(double x)
{
    std::stringstream ss;
    ss << x;
    std::string s(ss.str());
    return (char *)s.c_str();
}

char *vkSprintf::sprintfLL(LONG_LONG x)
{
    std::string s;
#if defined(_WIN32)
    char buf[64];
    _i64toa(x, buf, 10);
    s = buf;
#else
    vkSprintf sp;
    s = sp.sprintf(FMT_LONG_LONG, x);
#endif
    return (char *)s.c_str();
}

char *vkSprintf::sprintfUL(ULONG_LONG x)
{
    std::string s;
#if defined(_WIN32)
    char buf[64];
    _ui64toa(x, buf, 10);
    s = buf;
#else
    vkSprintf sp;
    s = sp.sprintf(FMT_ULONG_LONG, x);
#endif
    return (char *)s.c_str();
}

vkRossiDFP::vkRossiDFP() : dfp() {}

vkRossiDFP::~vkRossiDFP() {}

vkRossiDFP::vkRossiDFP(const vkRossiDFP & x) :   dfp(x) {}
vkRossiDFP::vkRossiDFP(const char x) :           dfp(itoa10((int)x)) {}
vkRossiDFP::vkRossiDFP(const unsigned char x) :  dfp(itoa10((int)x)) {}
vkRossiDFP::vkRossiDFP(const short x) :          dfp(itoa10(x)) {}
vkRossiDFP::vkRossiDFP(const unsigned short x) : dfp(itoa10(x)) {}
vkRossiDFP::vkRossiDFP(const int x) :            dfp(itoa10(x)) {}
vkRossiDFP::vkRossiDFP(const unsigned int x) :   dfp(itoa10(x)) {}
vkRossiDFP::vkRossiDFP(const long x) :           dfp(sc.sprintf("%ld", x)) {}
vkRossiDFP::vkRossiDFP(const unsigned long x) :  dfp(sc.sprintf("%lu", x)) {}
vkRossiDFP::vkRossiDFP(const float x) :          dfp(sc.sprintfFD(x)) {}
vkRossiDFP::vkRossiDFP(const double x) :         dfp(sc.sprintfFD(x)) {}
vkRossiDFP::vkRossiDFP(const char *x) :          dfp(x) {}
vkRossiDFP::vkRossiDFP(const unsigned char *x) : dfp((char *)x) {}
vkRossiDFP::vkRossiDFP(const LONG_LONG x) :      dfp(sc.sprintfLL(x)) {}
vkRossiDFP::vkRossiDFP(const ULONG_LONG x) :     dfp(sc.sprintfUL(x)) {}

void assignvkRossiDFP(const vkRossiDFP & source, vkRossiDFP & target)
{
	if (target.mant) { delete [] target.mant; }
	target.mant = new int[dfp::DIGITS];
	for (int i = dfp::DIGITS - 1; i >= 0; i--)
		target.mant[i] = source.mant[i];
	target.sign = source.sign;
	target.exp = source.exp;
	target.nans = source.nans;
}

vkRossiDFP & vkRossiDFP::operator =(const vkRossiDFP & a)
{
	assignvkRossiDFP(a, *this);
	return *this;
}
vkRossiDFP & vkRossiDFP::operator =(const char x)
{
	vkRossiDFP a(x);
    assignvkRossiDFP(a, *this);
    return *this;
}
vkRossiDFP & vkRossiDFP::operator =(const unsigned char x)
{
    vkRossiDFP a(x);
    assignvkRossiDFP(a, *this);
    return *this;
}
vkRossiDFP & vkRossiDFP::operator =(const short x)
{
    vkRossiDFP a(x);
    assignvkRossiDFP(a, *this);
    return *this;
}
vkRossiDFP & vkRossiDFP::operator =(const unsigned short x)
{
    vkRossiDFP a(x);
    assignvkRossiDFP(a, *this);
    return *this;
}
vkRossiDFP & vkRossiDFP::operator =(const int x)
{
    vkRossiDFP a(x);
    assignvkRossiDFP(a, *this);
    return *this;
}
vkRossiDFP & vkRossiDFP::operator =(const unsigned int x)
{
    vkRossiDFP a(x);
    assignvkRossiDFP(a, *this);
    return *this;
}
vkRossiDFP & vkRossiDFP::operator =(const float x)
{
    vkRossiDFP a(x);
    assignvkRossiDFP(a, *this);
    return *this;
}
vkRossiDFP & vkRossiDFP::operator =(const double x)
{
    vkRossiDFP a(x);
    assignvkRossiDFP(a, *this);
    return *this;
}
vkRossiDFP & vkRossiDFP::operator =(const char *x)
{
    vkRossiDFP a(x);
    assignvkRossiDFP(a, *this);
    return *this;
}
vkRossiDFP & vkRossiDFP::operator =(const unsigned char *x)
{
    vkRossiDFP a(x);
    assignvkRossiDFP(a, *this);
    return *this;
}
vkRossiDFP & vkRossiDFP::operator =(const LONG_LONG x)
{
    vkRossiDFP a(x);
    assignvkRossiDFP(a, *this);
    return *this;
}
vkRossiDFP & vkRossiDFP::operator =(const ULONG_LONG x)
{
    vkRossiDFP a(x);
    assignvkRossiDFP(a, *this);
    return *this;
}

vkRossiDFP & vkRossiDFP::operator +=(const vkRossiDFP & a)   { *this = *this + a; return *this; }
vkRossiDFP & vkRossiDFP::operator +=(const char c)           { vkRossiDFP x = c;  *this = *this + x; return *this; }
vkRossiDFP & vkRossiDFP::operator +=(const unsigned char uc) { vkRossiDFP x = uc; *this = *this + x; return *this; }
vkRossiDFP & vkRossiDFP::operator +=(const short i)          { vkRossiDFP x = i;  *this = *this + x; return *this; }
vkRossiDFP & vkRossiDFP::operator +=(const unsigned short i) { vkRossiDFP x = i;  *this = *this + x; return *this; }
vkRossiDFP & vkRossiDFP::operator +=(const int i)            { vkRossiDFP x = i;  *this = *this + x; return *this; }
vkRossiDFP & vkRossiDFP::operator +=(const unsigned int i)   { vkRossiDFP x = i;  *this = *this + x; return *this; }
vkRossiDFP & vkRossiDFP::operator +=(const long i)           { vkRossiDFP x = i;  *this = *this + x; return *this; }
vkRossiDFP & vkRossiDFP::operator +=(const unsigned long i)  { vkRossiDFP x = i;  *this = *this + x; return *this; }
vkRossiDFP & vkRossiDFP::operator +=(const float f)          { vkRossiDFP x = f;  *this = *this + x; return *this; }
vkRossiDFP & vkRossiDFP::operator +=(const double d)         { vkRossiDFP x = d;  *this = *this + x; return *this; }
vkRossiDFP & vkRossiDFP::operator +=(const char *s)          { vkRossiDFP x = s;  *this = *this + x; return *this; }
vkRossiDFP & vkRossiDFP::operator +=(const unsigned char *s) { vkRossiDFP x = (char *)s;  *this = *this + x; return *this; }
vkRossiDFP & vkRossiDFP::operator +=(const LONG_LONG i)      { vkRossiDFP x = i;  *this = *this + x; return *this; }
vkRossiDFP & vkRossiDFP::operator +=(const ULONG_LONG i)     { vkRossiDFP x = i;  *this = *this + x; return *this; }

vkRossiDFP & vkRossiDFP::operator -=(const vkRossiDFP & a)   { *this = *this - a; return *this; }
vkRossiDFP & vkRossiDFP::operator -=(const char c)           { vkRossiDFP x = c;  *this = *this - x; return *this; }
vkRossiDFP & vkRossiDFP::operator -=(const unsigned char uc) { vkRossiDFP x = uc; *this = *this - x; return *this; }
vkRossiDFP & vkRossiDFP::operator -=(const short i)          { vkRossiDFP x = i;  *this = *this - x; return *this; }
vkRossiDFP & vkRossiDFP::operator -=(const unsigned short i) { vkRossiDFP x = i;  *this = *this - x; return *this; }
vkRossiDFP & vkRossiDFP::operator -=(const int i)            { vkRossiDFP x = i;  *this = *this - x; return *this; }
vkRossiDFP & vkRossiDFP::operator -=(const unsigned int i)   { vkRossiDFP x = i;  *this = *this - x; return *this; }
vkRossiDFP & vkRossiDFP::operator -=(const long i)           { vkRossiDFP x = i;  *this = *this - x; return *this; }
vkRossiDFP & vkRossiDFP::operator -=(const unsigned long i)  { vkRossiDFP x = i;  *this = *this - x; return *this; }
vkRossiDFP & vkRossiDFP::operator -=(const float f)          { vkRossiDFP x = f;  *this = *this - x; return *this; }
vkRossiDFP & vkRossiDFP::operator -=(const double d)         { vkRossiDFP x = d;  *this = *this - x; return *this; }
vkRossiDFP & vkRossiDFP::operator -=(const char *x)          { vkRossiDFP xx = x; *this = *this - xx; return *this; }
vkRossiDFP & vkRossiDFP::operator -=(const unsigned char *x) { vkRossiDFP xx = (char *)x;  *this = *this - xx; return *this; }
vkRossiDFP & vkRossiDFP::operator -=(const LONG_LONG i)      { vkRossiDFP x = i;  *this = *this - x; return *this; }
vkRossiDFP & vkRossiDFP::operator -=(const ULONG_LONG i)     { vkRossiDFP x = i;  *this = *this - x; return *this; }

vkRossiDFP & vkRossiDFP::operator *=(const vkRossiDFP & a)   { *this = *this * a; return *this; }
vkRossiDFP & vkRossiDFP::operator *=(const char c)           { vkRossiDFP x = c;  *this = *this * x; return *this; }
vkRossiDFP & vkRossiDFP::operator *=(const unsigned char uc) { vkRossiDFP x = uc; *this = *this * x; return *this; }
vkRossiDFP & vkRossiDFP::operator *=(const short i)          { vkRossiDFP x = i;  *this = *this * x; return *this; }
vkRossiDFP & vkRossiDFP::operator *=(const unsigned short i) { vkRossiDFP x = i;  *this = *this * x; return *this; }
vkRossiDFP & vkRossiDFP::operator *=(const int i)            { vkRossiDFP x = i;  *this = *this * x; return *this; }
vkRossiDFP & vkRossiDFP::operator *=(const unsigned int i)   { vkRossiDFP x = i;  *this = *this * x; return *this; }
vkRossiDFP & vkRossiDFP::operator *=(const long i)           { vkRossiDFP x = i;  *this = *this * x; return *this; }
vkRossiDFP & vkRossiDFP::operator *=(const unsigned long i)  { vkRossiDFP x = i;  *this = *this * x; return *this; }
vkRossiDFP & vkRossiDFP::operator *=(const float f)          { vkRossiDFP x = f;  *this = *this * x; return *this; }
vkRossiDFP & vkRossiDFP::operator *=(const double d)         { vkRossiDFP x = d;  *this = *this * x; return *this; }
vkRossiDFP & vkRossiDFP::operator *=(const char *x)          { vkRossiDFP xx = x; *this = *this * xx; return *this; }
vkRossiDFP & vkRossiDFP::operator *=(const unsigned char *x) { vkRossiDFP xx = (char *)x;  *this = *this * xx; return *this; }
vkRossiDFP & vkRossiDFP::operator *=(const LONG_LONG i)      { vkRossiDFP x = i;  *this = *this * x; return *this; }
vkRossiDFP & vkRossiDFP::operator *=(const ULONG_LONG i)     { vkRossiDFP x = i;  *this = *this * x; return *this; }

vkRossiDFP & vkRossiDFP::operator /=(const vkRossiDFP & a)   { *this = *this / a; return *this; }
vkRossiDFP & vkRossiDFP::operator /=(const char c)           { vkRossiDFP x = c;  *this = *this / x; return *this; }
vkRossiDFP & vkRossiDFP::operator /=(const unsigned char uc) { vkRossiDFP x = uc; *this = *this / x; return *this; }
vkRossiDFP & vkRossiDFP::operator /=(const short i)          { vkRossiDFP x = i;  *this = *this / x; return *this; }
vkRossiDFP & vkRossiDFP::operator /=(const unsigned short i) { vkRossiDFP x = i;  *this = *this / x; return *this; }
vkRossiDFP & vkRossiDFP::operator /=(const int i)            { vkRossiDFP x = i;  *this = *this / x; return *this; }
vkRossiDFP & vkRossiDFP::operator /=(const unsigned int i)   { vkRossiDFP x = i;  *this = *this / x; return *this; }
vkRossiDFP & vkRossiDFP::operator /=(const long i)           { vkRossiDFP x = i;  *this = *this / x; return *this; }
vkRossiDFP & vkRossiDFP::operator /=(const unsigned long i)  { vkRossiDFP x = i;  *this = *this / x; return *this; }
vkRossiDFP & vkRossiDFP::operator /=(const float f)          { vkRossiDFP x = f;  *this = *this / x; return *this; }
vkRossiDFP & vkRossiDFP::operator /=(const double d)         { vkRossiDFP x = d;  *this = *this / x; return *this; }
vkRossiDFP & vkRossiDFP::operator /=(const char *x)          { vkRossiDFP xx = x; *this = *this / xx; return *this; }
vkRossiDFP & vkRossiDFP::operator /=(const unsigned char *x) { vkRossiDFP xx = (char *)x;  *this = *this / xx; return *this; }
vkRossiDFP & vkRossiDFP::operator /=(const LONG_LONG i)      { vkRossiDFP x = i;  *this = *this / x; return *this; }
vkRossiDFP & vkRossiDFP::operator /=(const ULONG_LONG i)     { vkRossiDFP x = i;  *this = *this / x; return *this; }

const vkRossiDFP & vkRossiDFP::operator ++()
{
    return *this += 1;
}

const vkRossiDFP & vkRossiDFP::operator ++(int a)
{
    plusminusvar = *this;
    *this += 1;
    return plusminusvar;
}

const vkRossiDFP & vkRossiDFP::operator --()
{
    return *this -= 1;
}

const vkRossiDFP & vkRossiDFP::operator --(int a)
{
    plusminusvar = *this;
    *this -= 1;
    return plusminusvar;
}

//const vkRossiDFP vkRossiDFP::operator +() const { return vkRossiDFP(*this); }

const vkRossiDFP operator +(const vkRossiDFP & a)
{
    return vkRossiDFP(a);
}

// const vkRossiDFP vkRossiDFP::operator -() const
// {
//     dfp x = static_cast<dfp>(*this).negate();
//     return vkRossiDFP(x.toString().c_str());
// }

const vkRossiDFP operator -(const vkRossiDFP & a)
{
    dfp x = static_cast<dfp>(a).negate();
    return vkRossiDFP(x.toString().c_str());
}

vkRossiDFP operator +(const vkRossiDFP & a, const vkRossiDFP & b)
{
    dfp x = static_cast<dfp>(a).add(static_cast<dfp>(b));
    return vkRossiDFP(x.toString().c_str());
}
vkRossiDFP operator +(const vkRossiDFP & a, const char b)           { vkRossiDFP x = b;  return a + x; }
vkRossiDFP operator +(const vkRossiDFP & a, const unsigned char b)  { vkRossiDFP x = b;  return a + x; }
vkRossiDFP operator +(const vkRossiDFP & a, const short b)          { vkRossiDFP x = b;  return a + x; }
vkRossiDFP operator +(const vkRossiDFP & a, const unsigned short b) { vkRossiDFP x = b;  return a + x; }
vkRossiDFP operator +(const vkRossiDFP & a, const int b)            { vkRossiDFP x = b;  return a + x; }
vkRossiDFP operator +(const vkRossiDFP & a, const unsigned int b)   { vkRossiDFP x = b;  return a + x; }
vkRossiDFP operator +(const vkRossiDFP & a, const long b)           { vkRossiDFP x = b;  return a + x; }
vkRossiDFP operator +(const vkRossiDFP & a, const unsigned long b)  { vkRossiDFP x = b;  return a + x; }
vkRossiDFP operator +(const vkRossiDFP & a, const float b)          { vkRossiDFP x = b;  return a + x; }
vkRossiDFP operator +(const vkRossiDFP & a, const double b)         { vkRossiDFP x = b;  return a + x; }
vkRossiDFP operator +(const vkRossiDFP & a, const char *b)          { vkRossiDFP x = b;  return a + x; }
vkRossiDFP operator +(const vkRossiDFP & a, const unsigned char *b) { vkRossiDFP x = (char *)b;  return a + x; }
vkRossiDFP operator +(const vkRossiDFP & a, const LONG_LONG b)      { vkRossiDFP x = b;  return a + x; }
vkRossiDFP operator +(const vkRossiDFP & a, const ULONG_LONG b)     { vkRossiDFP x = b;  return a + x; }
vkRossiDFP operator +(const char b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x + a; }
vkRossiDFP operator +(const unsigned char b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x + a; }
vkRossiDFP operator +(const short b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x + a; }
vkRossiDFP operator +(const unsigned short b, const vkRossiDFP & a) { vkRossiDFP x = b;  return x + a; }
vkRossiDFP operator +(const int b, const vkRossiDFP & a)            { vkRossiDFP x = b;  return x + a; }
vkRossiDFP operator +(const unsigned int b, const vkRossiDFP & a)   { vkRossiDFP x = b;  return x + a; }
vkRossiDFP operator +(const long b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x + a; }
vkRossiDFP operator +(const unsigned long b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x + a; }
vkRossiDFP operator +(const float b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x + a; }
vkRossiDFP operator +(const double b, const vkRossiDFP & a)         { vkRossiDFP x = b;  return x + a; }
vkRossiDFP operator +(const char *b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x + a; }
vkRossiDFP operator +(const unsigned char *b, const vkRossiDFP & a) { vkRossiDFP x = (char *)b;  return x + a; }
vkRossiDFP operator +(const LONG_LONG b, const vkRossiDFP & a)      { vkRossiDFP x = b;  return x + a; }
vkRossiDFP operator +(const ULONG_LONG b, const vkRossiDFP & a)     { vkRossiDFP x = b;  return x + a; }

vkRossiDFP operator -(const vkRossiDFP & a, const vkRossiDFP & b)
{
    dfp x = static_cast<dfp>(a).subtract(static_cast<dfp>(b));
    return vkRossiDFP(x.toString().c_str());
}
vkRossiDFP operator -(const vkRossiDFP & a, const char b)           { vkRossiDFP x = b;  return a - x; }
vkRossiDFP operator -(const vkRossiDFP & a, const unsigned char b)  { vkRossiDFP x = b;  return a - x; }
vkRossiDFP operator -(const vkRossiDFP & a, const short b)          { vkRossiDFP x = b;  return a - x; }
vkRossiDFP operator -(const vkRossiDFP & a, const unsigned short b) { vkRossiDFP x = b;  return a - x; }
vkRossiDFP operator -(const vkRossiDFP & a, const int b)            { vkRossiDFP x = b;  return a - x; }
vkRossiDFP operator -(const vkRossiDFP & a, const unsigned int b)   { vkRossiDFP x = b;  return a - x; }
vkRossiDFP operator -(const vkRossiDFP & a, const long b)           { vkRossiDFP x = b;  return a - x; }
vkRossiDFP operator -(const vkRossiDFP & a, const unsigned long b)  { vkRossiDFP x = b;  return a - x; }
vkRossiDFP operator -(const vkRossiDFP & a, const float b)          { vkRossiDFP x = b;  return a - x; }
vkRossiDFP operator -(const vkRossiDFP & a, const double b)         { vkRossiDFP x = b;  return a - x; }
vkRossiDFP operator -(const vkRossiDFP & a, const char *b)          { vkRossiDFP x = b;  return a - x; }
vkRossiDFP operator -(const vkRossiDFP & a, const unsigned char *b) { vkRossiDFP x = (char *)b;  return a - x; }
vkRossiDFP operator -(const vkRossiDFP & a, const LONG_LONG b)      { vkRossiDFP x = b;  return a - x; }
vkRossiDFP operator -(const vkRossiDFP & a, const ULONG_LONG b)     { vkRossiDFP x = b;  return a - x; }
vkRossiDFP operator -(const char b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x - a; }
vkRossiDFP operator -(const unsigned char b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x - a; }
vkRossiDFP operator -(const short b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x - a; }
vkRossiDFP operator -(const unsigned short b, const vkRossiDFP & a) { vkRossiDFP x = b;  return x - a; }
vkRossiDFP operator -(const int b, const vkRossiDFP & a)            { vkRossiDFP x = b;  return x - a; }
vkRossiDFP operator -(const unsigned int b, const vkRossiDFP & a)   { vkRossiDFP x = b;  return x - a; }
vkRossiDFP operator -(const long b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x - a; }
vkRossiDFP operator -(const unsigned long b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x - a; }
vkRossiDFP operator -(const float b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x - a; }
vkRossiDFP operator -(const double b, const vkRossiDFP & a)         { vkRossiDFP x = b;  return x - a; }
vkRossiDFP operator -(const char *b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x - a; }
vkRossiDFP operator -(const unsigned char *b, const vkRossiDFP & a) { vkRossiDFP x = (char *)b;  return x - a; }
vkRossiDFP operator -(const LONG_LONG b, const vkRossiDFP & a)      { vkRossiDFP x = b;  return x - a; }
vkRossiDFP operator -(const ULONG_LONG b, const vkRossiDFP & a)     { vkRossiDFP x = b;  return x - a; }

vkRossiDFP operator *(const vkRossiDFP & a, const vkRossiDFP & b)
{
    dfp x = static_cast<dfp>(a).multiply(static_cast<dfp>(b));
    return vkRossiDFP(x.toString().c_str());
}
vkRossiDFP operator *(const vkRossiDFP & a, const char b)           { vkRossiDFP x = b;  return a * x; }
vkRossiDFP operator *(const vkRossiDFP & a, const unsigned char b)  { vkRossiDFP x = b;  return a * x; }
vkRossiDFP operator *(const vkRossiDFP & a, const short b)          { vkRossiDFP x = b;  return a * x; }
vkRossiDFP operator *(const vkRossiDFP & a, const unsigned short b) { vkRossiDFP x = b;  return a * x; }
vkRossiDFP operator *(const vkRossiDFP & a, const int b)            { vkRossiDFP x = b;  return a * x; }
vkRossiDFP operator *(const vkRossiDFP & a, const unsigned int b)   { vkRossiDFP x = b;  return a * x; }
vkRossiDFP operator *(const vkRossiDFP & a, const long b)           { vkRossiDFP x = b;  return a * x; }
vkRossiDFP operator *(const vkRossiDFP & a, const unsigned long b)  { vkRossiDFP x = b;  return a * x; }
vkRossiDFP operator *(const vkRossiDFP & a, const float b)          { vkRossiDFP x = b;  return a * x; }
vkRossiDFP operator *(const vkRossiDFP & a, const double b)         { vkRossiDFP x = b;  return a * x; }
vkRossiDFP operator *(const vkRossiDFP & a, const char *b)          { vkRossiDFP x = b;  return a * x; }
vkRossiDFP operator *(const vkRossiDFP & a, const unsigned char *b) { vkRossiDFP x = (char *)b;  return a * x; }
vkRossiDFP operator *(const vkRossiDFP & a, const LONG_LONG b)      { vkRossiDFP x = b;  return a * x; }
vkRossiDFP operator *(const vkRossiDFP & a, const ULONG_LONG b)     { vkRossiDFP x = b;  return a * x; }
vkRossiDFP operator *(const char b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x * a; }
vkRossiDFP operator *(const unsigned char b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x * a; }
vkRossiDFP operator *(const short b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x * a; }
vkRossiDFP operator *(const unsigned short b, const vkRossiDFP & a) { vkRossiDFP x = b;  return x * a; }
vkRossiDFP operator *(const int b, const vkRossiDFP & a)            { vkRossiDFP x = b;  return x * a; }
vkRossiDFP operator *(const unsigned int b, const vkRossiDFP & a)   { vkRossiDFP x = b;  return x * a; }
vkRossiDFP operator *(const long b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x * a; }
vkRossiDFP operator *(const unsigned long b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x * a; }
vkRossiDFP operator *(const float b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x * a; }
vkRossiDFP operator *(const double b, const vkRossiDFP & a)         { vkRossiDFP x = b;  return x * a; }
vkRossiDFP operator *(const char *b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x * a; }
vkRossiDFP operator *(const unsigned char *b, const vkRossiDFP & a) { vkRossiDFP x = (char *)b;  return x * a; }
vkRossiDFP operator *(const LONG_LONG b, const vkRossiDFP & a)      { vkRossiDFP x = b;  return x * a; }
vkRossiDFP operator *(const ULONG_LONG b, const vkRossiDFP & a)     { vkRossiDFP x = b;  return x * a; }

vkRossiDFP operator /(const vkRossiDFP & a, const vkRossiDFP & b)
{
    dfp x = static_cast<dfp>(a).divide(static_cast<dfp>(b));
    return vkRossiDFP(x.toString().c_str());
}
vkRossiDFP operator /(const vkRossiDFP & a, const char b)           { vkRossiDFP x = b;  return a / x; }
vkRossiDFP operator /(const vkRossiDFP & a, const unsigned char b)  { vkRossiDFP x = b;  return a / x; }
vkRossiDFP operator /(const vkRossiDFP & a, const short b)          { vkRossiDFP x = b;  return a / x; }
vkRossiDFP operator /(const vkRossiDFP & a, const unsigned short b) { vkRossiDFP x = b;  return a / x; }
vkRossiDFP operator /(const vkRossiDFP & a, const int b)            { vkRossiDFP x = b;  return a / x; }
vkRossiDFP operator /(const vkRossiDFP & a, const unsigned int b)   { vkRossiDFP x = b;  return a / x; }
vkRossiDFP operator /(const vkRossiDFP & a, const long b)           { vkRossiDFP x = b;  return a / x; }
vkRossiDFP operator /(const vkRossiDFP & a, const unsigned long b)  { vkRossiDFP x = b;  return a / x; }
vkRossiDFP operator /(const vkRossiDFP & a, const float b)          { vkRossiDFP x = b;  return a / x; }
vkRossiDFP operator /(const vkRossiDFP & a, const double b)         { vkRossiDFP x = b;  return a / x; }
vkRossiDFP operator /(const vkRossiDFP & a, const char *b)          { vkRossiDFP x = b;  return a / x; }
vkRossiDFP operator /(const vkRossiDFP & a, const unsigned char *b) { vkRossiDFP x = (char *)b;  return a / x; }
vkRossiDFP operator /(const vkRossiDFP & a, const LONG_LONG b)      { vkRossiDFP x = b;  return a / x; }
vkRossiDFP operator /(const vkRossiDFP & a, const ULONG_LONG b)     { vkRossiDFP x = b;  return a / x; }
vkRossiDFP operator /(const char b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x / a; }
vkRossiDFP operator /(const unsigned char b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x / a; }
vkRossiDFP operator /(const short b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x / a; }
vkRossiDFP operator /(const unsigned short b, const vkRossiDFP & a) { vkRossiDFP x = b;  return x / a; }
vkRossiDFP operator /(const int b, const vkRossiDFP & a)            { vkRossiDFP x = b;  return x / a; }
vkRossiDFP operator /(const unsigned int b, const vkRossiDFP & a)   { vkRossiDFP x = b;  return x / a; }
vkRossiDFP operator /(const long b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x / a; }
vkRossiDFP operator /(const unsigned long b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x / a; }
vkRossiDFP operator /(const float b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x / a; }
vkRossiDFP operator /(const double b, const vkRossiDFP & a)         { vkRossiDFP x = b;  return x / a; }
vkRossiDFP operator /(const char *b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x / a; }
vkRossiDFP operator /(const unsigned char *b, const vkRossiDFP & a) { vkRossiDFP x = (char *)b;  return x / a; }
vkRossiDFP operator /(const LONG_LONG b, const vkRossiDFP & a)      { vkRossiDFP x = b;  return x / a; }
vkRossiDFP operator /(const ULONG_LONG b, const vkRossiDFP & a)     { vkRossiDFP x = b;  return x / a; }

vkRossiDFP operator %(const vkRossiDFP & a, const vkRossiDFP & b)
{
    dfp xb = static_cast<dfp>(b);
    dfp x = static_cast<dfp>(a).remainder(xb);
    return vkRossiDFP(x.toString().c_str());
}
vkRossiDFP operator %(const vkRossiDFP & a, const char b)           { vkRossiDFP x = b;  return a % x; }
vkRossiDFP operator %(const vkRossiDFP & a, const unsigned char b)  { vkRossiDFP x = b;  return a % x; }
vkRossiDFP operator %(const vkRossiDFP & a, const short b)          { vkRossiDFP x = b;  return a % x; }
vkRossiDFP operator %(const vkRossiDFP & a, const unsigned short b) { vkRossiDFP x = b;  return a % x; }
vkRossiDFP operator %(const vkRossiDFP & a, const int b)            { vkRossiDFP x = b;  return a % x; }
vkRossiDFP operator %(const vkRossiDFP & a, const unsigned int b)   { vkRossiDFP x = b;  return a % x; }
vkRossiDFP operator %(const vkRossiDFP & a, const long b)           { vkRossiDFP x = b;  return a % x; }
vkRossiDFP operator %(const vkRossiDFP & a, const unsigned long b)  { vkRossiDFP x = b;  return a % x; }
vkRossiDFP operator %(const vkRossiDFP & a, const float b)          { vkRossiDFP x = b;  return a % x; }
vkRossiDFP operator %(const vkRossiDFP & a, const double b)         { vkRossiDFP x = b;  return a % x; }
vkRossiDFP operator %(const vkRossiDFP & a, const char *b)          { vkRossiDFP x = b;  return a % x; }
vkRossiDFP operator %(const vkRossiDFP & a, const unsigned char *b) { vkRossiDFP x = (char *)b;  return a % x; }
vkRossiDFP operator %(const vkRossiDFP & a, const LONG_LONG b)      { vkRossiDFP x = b;  return a % x; }
vkRossiDFP operator %(const vkRossiDFP & a, const ULONG_LONG b)     { vkRossiDFP x = b;  return a % x; }
vkRossiDFP operator %(const char b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x % a; }
vkRossiDFP operator %(const unsigned char b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x % a; }
vkRossiDFP operator %(const short b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x % a; }
vkRossiDFP operator %(const unsigned short b, const vkRossiDFP & a) { vkRossiDFP x = b;  return x % a; }
vkRossiDFP operator %(const int b, const vkRossiDFP & a)            { vkRossiDFP x = b;  return x % a; }
vkRossiDFP operator %(const unsigned int b, const vkRossiDFP & a)   { vkRossiDFP x = b;  return x % a; }
vkRossiDFP operator %(const long b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x % a; }
vkRossiDFP operator %(const unsigned long b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x % a; }
vkRossiDFP operator %(const float b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x % a; }
vkRossiDFP operator %(const double b, const vkRossiDFP & a)         { vkRossiDFP x = b;  return x % a; }
vkRossiDFP operator %(const char *b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x % a; }
vkRossiDFP operator %(const unsigned char *b, const vkRossiDFP & a) { vkRossiDFP x = (char *)b;  return x % a; }
vkRossiDFP operator %(const LONG_LONG b, const vkRossiDFP & a)      { vkRossiDFP x = b;  return x % a; }
vkRossiDFP operator %(const ULONG_LONG b, const vkRossiDFP & a)     { vkRossiDFP x = b;  return x % a; }

bool operator ==(const vkRossiDFP & a, const vkRossiDFP & b)
{
    dfp xb = static_cast<dfp>(b);
    return static_cast<dfp>(a).equalDFP(xb);
}
bool operator ==(const vkRossiDFP & a, const char b)           { vkRossiDFP x = b;  return a == x; }
bool operator ==(const vkRossiDFP & a, const unsigned char b)  { vkRossiDFP x = b;  return a == x; }
bool operator ==(const vkRossiDFP & a, const short b)          { vkRossiDFP x = b;  return a == x; }
bool operator ==(const vkRossiDFP & a, const unsigned short b) { vkRossiDFP x = b;  return a == x; }
bool operator ==(const vkRossiDFP & a, const int b)            { vkRossiDFP x = b;  return a == x; }
bool operator ==(const vkRossiDFP & a, const unsigned int b)   { vkRossiDFP x = b;  return a == x; }
bool operator ==(const vkRossiDFP & a, const long b)           { vkRossiDFP x = b;  return a == x; }
bool operator ==(const vkRossiDFP & a, const unsigned long b)  { vkRossiDFP x = b;  return a == x; }
bool operator ==(const vkRossiDFP & a, const float b)          { vkRossiDFP x = b;  return a == x; }
bool operator ==(const vkRossiDFP & a, const double b)         { vkRossiDFP x = b;  return a == x; }
bool operator ==(const vkRossiDFP & a, const char *b)          { vkRossiDFP x = b;  return a == x; }
bool operator ==(const vkRossiDFP & a, const unsigned char *b) { vkRossiDFP x = (char *)b;  return a == x; }
bool operator ==(const vkRossiDFP & a, const LONG_LONG b)      { vkRossiDFP x = b;  return a == x; }
bool operator ==(const vkRossiDFP & a, const ULONG_LONG b)     { vkRossiDFP x = b;  return a == x; }
bool operator ==(const char b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x == a; }
bool operator ==(const unsigned char b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x == a; }
bool operator ==(const short b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x == a; }
bool operator ==(const unsigned short b, const vkRossiDFP & a) { vkRossiDFP x = b;  return x == a; }
bool operator ==(const int b, const vkRossiDFP & a)            { vkRossiDFP x = b;  return x == a; }
bool operator ==(const unsigned int b, const vkRossiDFP & a)   { vkRossiDFP x = b;  return x == a; }
bool operator ==(const long b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x == a; }
bool operator ==(const unsigned long b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x == a; }
bool operator ==(const float b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x == a; }
bool operator ==(const double b, const vkRossiDFP & a)         { vkRossiDFP x = b;  return x == a; }
bool operator ==(const char *b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x == a; }
bool operator ==(const unsigned char *b, const vkRossiDFP & a) { vkRossiDFP x = (char *)b;  return x == a; }
bool operator ==(const LONG_LONG b, const vkRossiDFP & a)      { vkRossiDFP x = b;  return x == a; }
bool operator ==(const ULONG_LONG b, const vkRossiDFP & a)     { vkRossiDFP x = b;  return x == a; }

bool operator !=(const vkRossiDFP & a, const vkRossiDFP & b)
{
    dfp xb = static_cast<dfp>(b);
    return static_cast<dfp>(a).unequalDFP(xb);
}
bool operator !=(const vkRossiDFP & a, const char b)           { vkRossiDFP x = b;  return a != x; }
bool operator !=(const vkRossiDFP & a, const unsigned char b)  { vkRossiDFP x = b;  return a != x; }
bool operator !=(const vkRossiDFP & a, const short b)          { vkRossiDFP x = b;  return a != x; }
bool operator !=(const vkRossiDFP & a, const unsigned short b) { vkRossiDFP x = b;  return a != x; }
bool operator !=(const vkRossiDFP & a, const int b)            { vkRossiDFP x = b;  return a != x; }
bool operator !=(const vkRossiDFP & a, const unsigned int b)   { vkRossiDFP x = b;  return a != x; }
bool operator !=(const vkRossiDFP & a, const long b)           { vkRossiDFP x = b;  return a != x; }
bool operator !=(const vkRossiDFP & a, const unsigned long b)  { vkRossiDFP x = b;  return a != x; }
bool operator !=(const vkRossiDFP & a, const float b)          { vkRossiDFP x = b;  return a != x; }
bool operator !=(const vkRossiDFP & a, const double b)         { vkRossiDFP x = b;  return a != x; }
bool operator !=(const vkRossiDFP & a, const char *b)          { vkRossiDFP x = b;  return a != x; }
bool operator !=(const vkRossiDFP & a, const unsigned char *b) { vkRossiDFP x = (char *)b;  return a != x; }
bool operator !=(const vkRossiDFP & a, const LONG_LONG b)      { vkRossiDFP x = b;  return a != x; }
bool operator !=(const vkRossiDFP & a, const ULONG_LONG b)     { vkRossiDFP x = b;  return a != x; }
bool operator !=(const char b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x != a; }
bool operator !=(const unsigned char b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x != a; }
bool operator !=(const short b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x != a; }
bool operator !=(const unsigned short b, const vkRossiDFP & a) { vkRossiDFP x = b;  return x != a; }
bool operator !=(const int b, const vkRossiDFP & a)            { vkRossiDFP x = b;  return x != a; }
bool operator !=(const unsigned int b, const vkRossiDFP & a)   { vkRossiDFP x = b;  return x != a; }
bool operator !=(const long b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x != a; }
bool operator !=(const unsigned long b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x != a; }
bool operator !=(const float b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x != a; }
bool operator !=(const double b, const vkRossiDFP & a)         { vkRossiDFP x = b;  return x != a; }
bool operator !=(const char *b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x != a; }
bool operator !=(const unsigned char *b, const vkRossiDFP & a) { vkRossiDFP x = (char *)b;  return x != a; }
bool operator !=(const LONG_LONG b, const vkRossiDFP & a)      { vkRossiDFP x = b;  return x != a; }
bool operator !=(const ULONG_LONG b, const vkRossiDFP & a)     { vkRossiDFP x = b;  return x != a; }

bool operator <(const vkRossiDFP & a, const vkRossiDFP & b)
{
    dfp xb = static_cast<dfp>(b);
    return static_cast<dfp>(a).lessThan(xb);
}
bool operator <(const vkRossiDFP & a, const char b)           { vkRossiDFP x = b;  return a < x; }
bool operator <(const vkRossiDFP & a, const unsigned char b)  { vkRossiDFP x = b;  return a < x; }
bool operator <(const vkRossiDFP & a, const short b)          { vkRossiDFP x = b;  return a < x; }
bool operator <(const vkRossiDFP & a, const unsigned short b) { vkRossiDFP x = b;  return a < x; }
bool operator <(const vkRossiDFP & a, const int b)            { vkRossiDFP x = b;  return a < x; }
bool operator <(const vkRossiDFP & a, const unsigned int b)   { vkRossiDFP x = b;  return a < x; }
bool operator <(const vkRossiDFP & a, const long b)           { vkRossiDFP x = b;  return a < x; }
bool operator <(const vkRossiDFP & a, const unsigned long b)  { vkRossiDFP x = b;  return a < x; }
bool operator <(const vkRossiDFP & a, const float b)          { vkRossiDFP x = b;  return a < x; }
bool operator <(const vkRossiDFP & a, const double b)         { vkRossiDFP x = b;  return a < x; }
bool operator <(const vkRossiDFP & a, const char *b)          { vkRossiDFP x = b;  return a < x; }
bool operator <(const vkRossiDFP & a, const unsigned char *b) { vkRossiDFP x = (char *)b;  return a < x; }
bool operator <(const vkRossiDFP & a, const LONG_LONG b)      { vkRossiDFP x = b;  return a < x; }
bool operator <(const vkRossiDFP & a, const ULONG_LONG b)     { vkRossiDFP x = b;  return a < x; }
bool operator <(const char b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x < a; }
bool operator <(const unsigned char b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x < a; }
bool operator <(const short b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x < a; }
bool operator <(const unsigned short b, const vkRossiDFP & a) { vkRossiDFP x = b;  return x < a; }
bool operator <(const int b, const vkRossiDFP & a)            { vkRossiDFP x = b;  return x < a; }
bool operator <(const unsigned int b, const vkRossiDFP & a)   { vkRossiDFP x = b;  return x < a; }
bool operator <(const long b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x < a; }
bool operator <(const unsigned long b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x < a; }
bool operator <(const float b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x < a; }
bool operator <(const double b, const vkRossiDFP & a)         { vkRossiDFP x = b;  return x < a; }
bool operator <(const char *b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x < a; }
bool operator <(const unsigned char *b, const vkRossiDFP & a) { vkRossiDFP x = (char *)b;  return x < a; }
bool operator <(const LONG_LONG b, const vkRossiDFP & a)      { vkRossiDFP x = b;  return x < a; }
bool operator <(const ULONG_LONG b, const vkRossiDFP & a)     { vkRossiDFP x = b;  return x < a; }

bool operator >(const vkRossiDFP & a, const vkRossiDFP & b)
{
    dfp xb = static_cast<dfp>(b);
    return static_cast<dfp>(a).greaterThan(xb);
}
bool operator >(const vkRossiDFP & a, const char b)           { vkRossiDFP x = b;  return a > x; }
bool operator >(const vkRossiDFP & a, const unsigned char b)  { vkRossiDFP x = b;  return a > x; }
bool operator >(const vkRossiDFP & a, const short b)          { vkRossiDFP x = b;  return a > x; }
bool operator >(const vkRossiDFP & a, const unsigned short b) { vkRossiDFP x = b;  return a > x; }
bool operator >(const vkRossiDFP & a, const int b)            { vkRossiDFP x = b;  return a > x; }
bool operator >(const vkRossiDFP & a, const unsigned int b)   { vkRossiDFP x = b;  return a > x; }
bool operator >(const vkRossiDFP & a, const long b)           { vkRossiDFP x = b;  return a > x; }
bool operator >(const vkRossiDFP & a, const unsigned long b)  { vkRossiDFP x = b;  return a > x; }
bool operator >(const vkRossiDFP & a, const float b)          { vkRossiDFP x = b;  return a > x; }
bool operator >(const vkRossiDFP & a, const double b)         { vkRossiDFP x = b;  return a > x; }
bool operator >(const vkRossiDFP & a, const char *b)          { vkRossiDFP x = b;  return a > x; }
bool operator >(const vkRossiDFP & a, const unsigned char *b) { vkRossiDFP x = (char *)b;  return a > x; }
bool operator >(const vkRossiDFP & a, const LONG_LONG b)      { vkRossiDFP x = b;  return a > x; }
bool operator >(const vkRossiDFP & a, const ULONG_LONG b)     { vkRossiDFP x = b;  return a > x; }
bool operator >(const char b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x > a; }
bool operator >(const unsigned char b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x > a; }
bool operator >(const short b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x > a; }
bool operator >(const unsigned short b, const vkRossiDFP & a) { vkRossiDFP x = b;  return x > a; }
bool operator >(const int b, const vkRossiDFP & a)            { vkRossiDFP x = b;  return x > a; }
bool operator >(const unsigned int b, const vkRossiDFP & a)   { vkRossiDFP x = b;  return x > a; }
bool operator >(const long b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x > a; }
bool operator >(const unsigned long b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x > a; }
bool operator >(const float b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x > a; }
bool operator >(const double b, const vkRossiDFP & a)         { vkRossiDFP x = b;  return x > a; }
bool operator >(const char *b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x > a; }
bool operator >(const unsigned char *b, const vkRossiDFP & a) { vkRossiDFP x = (char *)b;  return x > a; }
bool operator >(const LONG_LONG b, const vkRossiDFP & a)      { vkRossiDFP x = b;  return x > a; }
bool operator >(const ULONG_LONG b, const vkRossiDFP & a)     { vkRossiDFP x = b;  return x > a; }

bool operator <=(const vkRossiDFP & a, const vkRossiDFP & b)
{
    dfp xa = static_cast<dfp>(a);
    dfp xb = static_cast<dfp>(b);
    return (dfp::compare(xa ,xb) <= 0);
}
bool operator <=(const vkRossiDFP & a, const char b)           { vkRossiDFP x = b;  return a <= x; }
bool operator <=(const vkRossiDFP & a, const unsigned char b)  { vkRossiDFP x = b;  return a <= x; }
bool operator <=(const vkRossiDFP & a, const short b)          { vkRossiDFP x = b;  return a <= x; }
bool operator <=(const vkRossiDFP & a, const unsigned short b) { vkRossiDFP x = b;  return a <= x; }
bool operator <=(const vkRossiDFP & a, const int b)            { vkRossiDFP x = b;  return a <= x; }
bool operator <=(const vkRossiDFP & a, const unsigned int b)   { vkRossiDFP x = b;  return a <= x; }
bool operator <=(const vkRossiDFP & a, const long b)           { vkRossiDFP x = b;  return a <= x; }
bool operator <=(const vkRossiDFP & a, const unsigned long b)  { vkRossiDFP x = b;  return a <= x; }
bool operator <=(const vkRossiDFP & a, const float b)          { vkRossiDFP x = b;  return a <= x; }
bool operator <=(const vkRossiDFP & a, const double b)         { vkRossiDFP x = b;  return a <= x; }
bool operator <=(const vkRossiDFP & a, const char *b)          { vkRossiDFP x = b;  return a <= x; }
bool operator <=(const vkRossiDFP & a, const unsigned char *b) { vkRossiDFP x = (char *)b;  return a <= x; }
bool operator <=(const vkRossiDFP & a, const LONG_LONG b)      { vkRossiDFP x = b;  return a <= x; }
bool operator <=(const vkRossiDFP & a, const ULONG_LONG b)     { vkRossiDFP x = b;  return a <= x; }
bool operator <=(const char b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x <= a; }
bool operator <=(const unsigned char b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x <= a; }
bool operator <=(const short b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x <= a; }
bool operator <=(const unsigned short b, const vkRossiDFP & a) { vkRossiDFP x = b;  return x <= a; }
bool operator <=(const int b, const vkRossiDFP & a)            { vkRossiDFP x = b;  return x <= a; }
bool operator <=(const unsigned int b, const vkRossiDFP & a)   { vkRossiDFP x = b;  return x <= a; }
bool operator <=(const long b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x <= a; }
bool operator <=(const unsigned long b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x <= a; }
bool operator <=(const float b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x <= a; }
bool operator <=(const double b, const vkRossiDFP & a)         { vkRossiDFP x = b;  return x <= a; }
bool operator <=(const char *b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x <= a; }
bool operator <=(const unsigned char *b, const vkRossiDFP & a) { vkRossiDFP x = (char *)b;  return x <= a; }
bool operator <=(const LONG_LONG b, const vkRossiDFP & a)      { vkRossiDFP x = b;  return x <= a; }
bool operator <=(const ULONG_LONG b, const vkRossiDFP & a)     { vkRossiDFP x = b;  return x <= a; }

bool operator >=(const vkRossiDFP & a, const vkRossiDFP & b)
{
    dfp xa = static_cast<dfp>(a);
    dfp xb = static_cast<dfp>(b);
    return (dfp::compare(xa ,xb) >= 0);
}
bool operator >=(const vkRossiDFP & a, const char b)           { vkRossiDFP x = b;  return a >= x; }
bool operator >=(const vkRossiDFP & a, const unsigned char b)  { vkRossiDFP x = b;  return a >= x; }
bool operator >=(const vkRossiDFP & a, const short b)          { vkRossiDFP x = b;  return a >= x; }
bool operator >=(const vkRossiDFP & a, const unsigned short b) { vkRossiDFP x = b;  return a >= x; }
bool operator >=(const vkRossiDFP & a, const int b)            { vkRossiDFP x = b;  return a >= x; }
bool operator >=(const vkRossiDFP & a, const unsigned int b)   { vkRossiDFP x = b;  return a >= x; }
bool operator >=(const vkRossiDFP & a, const long b)           { vkRossiDFP x = b;  return a >= x; }
bool operator >=(const vkRossiDFP & a, const unsigned long b)  { vkRossiDFP x = b;  return a >= x; }
bool operator >=(const vkRossiDFP & a, const float b)          { vkRossiDFP x = b;  return a >= x; }
bool operator >=(const vkRossiDFP & a, const double b)         { vkRossiDFP x = b;  return a >= x; }
bool operator >=(const vkRossiDFP & a, const char *b)          { vkRossiDFP x = b;  return a >= x; }
bool operator >=(const vkRossiDFP & a, const unsigned char *b) { vkRossiDFP x = (char *)b;  return a >= x; }
bool operator >=(const vkRossiDFP & a, const LONG_LONG b)      { vkRossiDFP x = b;  return a >= x; }
bool operator >=(const vkRossiDFP & a, const ULONG_LONG b)     { vkRossiDFP x = b;  return a >= x; }
bool operator >=(const char b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x >= a; }
bool operator >=(const unsigned char b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x >= a; }
bool operator >=(const short b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x >= a; }
bool operator >=(const unsigned short b, const vkRossiDFP & a) { vkRossiDFP x = b;  return x >= a; }
bool operator >=(const int b, const vkRossiDFP & a)            { vkRossiDFP x = b;  return x >= a; }
bool operator >=(const unsigned int b, const vkRossiDFP & a)   { vkRossiDFP x = b;  return x >= a; }
bool operator >=(const long b, const vkRossiDFP & a)           { vkRossiDFP x = b;  return x >= a; }
bool operator >=(const unsigned long b, const vkRossiDFP & a)  { vkRossiDFP x = b;  return x >= a; }
bool operator >=(const float b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x >= a; }
bool operator >=(const double b, const vkRossiDFP & a)         { vkRossiDFP x = b;  return x >= a; }
bool operator >=(const char *b, const vkRossiDFP & a)          { vkRossiDFP x = b;  return x >= a; }
bool operator >=(const unsigned char *b, const vkRossiDFP & a) { vkRossiDFP x = (char *)b;  return x >= a; }
bool operator >=(const LONG_LONG b, const vkRossiDFP & a)      { vkRossiDFP x = b;  return x >= a; }
bool operator >=(const ULONG_LONG b, const vkRossiDFP & a)     { vkRossiDFP x = b;  return x >= a; }

const vkRossiDFP abs(const vkRossiDFP & a)
{
    vkRossiDFP::plusminusvar = a;
    if (a < vkRossiDFP::ZERO)
    {
        return -vkRossiDFP::plusminusvar;
    }
    return vkRossiDFP::plusminusvar;
}

const vkRossiDFP sqrt(const vkRossiDFP & a)
{
    dfp xa = static_cast<dfp>(a);
    dfp x = xa.sqrt();
    return vkRossiDFP(x.toString().c_str());
}

const vkRossiDFP floor(const vkRossiDFP & a)
{
    dfp xa = static_cast<dfp>(a);
    dfp x = xa.floor();
    return vkRossiDFP(x.toString().c_str());
}

const vkRossiDFP ceil(const vkRossiDFP & a)
{
    dfp xa = static_cast<dfp>(a);
    dfp x = xa.ceil();
    return vkRossiDFP(x.toString().c_str());
}
