//
//  gu_math.h
//  CoreStructures
//

#pragma once

#include <cmath>
#include <limits> // include types to access NaN and +-inf constants for each primitive type
#include <algorithm>
#include <functional>


typedef unsigned char gu_byte;


static const char LogTable256[256] =
{
#define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
	-1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3,
	LT(4), LT(5), LT(5), LT(6), LT(6), LT(6), LT(6),
	LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)
};


//
// trignometric constants extending definitions in <cmath>
//
#define gu_pi				3.141593f
#define	gu_radian			0.017453f	// pi / 180
#define gu_radian_rcp		57.295779f	// radian reciprocal = 180 / pi




// gu_math inline template functions

// clamp given value a into [lower, upper] range
template <class T>
inline T clamp(T a, T lower, T upper) {

	return std::max<T>(lower, std::min<T>(a, upper));
}


// return the square of two numbers
template <class T>
inline T sqr(T x) {
	
	return x * x;
}


// linear interpolation over interval [a, b] where x lies in the interval [0, 1].  letp performs linear extrapolation if x lies in the intervals [-inf, a) or (b, +inf]
template <class T>
inline T lerp(T a, T b, float x) {

	return a * (1.0f - x) + b * x;
}


// return true if number if a single bit (log base 2 aligned)
inline bool isLogBase2(unsigned int x) {
	
	//return ((-x)&x)==x;
	return ((~x+1)&x)==x; // ~x+1 = -x (avoid compiler warning attemping negation of unsigned int)
}


// find the log base 2 of v (base 2 number index of v)
inline unsigned int logbase2(unsigned int v) {
	
	unsigned int			r;
	register unsigned int	t, tt;
	
	if (tt = v >> 16)
		r = (t = tt >> 8) ? 24 + LogTable256[t] : 16 + LogTable256[tt];
	else
		r = (t = v >> 8) ? 8 + LogTable256[t] : LogTable256[v];
	
	return r;
}


// round-up to next largest log base 2 number index for v
inline unsigned int roundBase2(unsigned int v) {
	
	return (v==0) ? 0 : (0x01 << ((isLogBase2(v)) ? logbase2(v) : logbase2(v)+1));
}



// floating point & overloaded functions

#define	gu_epsilon			std::numeric_limits<float>::epsilon() // single precision floating point error tolerance
#define	gu_epsilon_d		std::numeric_limits<double>::epsilon() // double precision floating point error tolerance


// fequal returns true if fabs(a - b) < gu_epsilon for single precision values (or < gu_epsilon_d for double precision values), false otherwise
inline bool fequal(float a, float b) {
	
	return fabs(a - b) < gu_epsilon;
}

inline bool fequal(double a, double b) {

	return fabs(a - b) < gu_epsilon_d;
}


// x < y
inline bool fless(float x, float y) {
	
	return (y - x) >= gu_epsilon;
}

inline bool fless(double x, double y) {
	
	return (y - x) >= gu_epsilon_d;
}


// x > y
inline bool fgreater(float x, float y) {
	
	return (x - y) >= gu_epsilon;
}

inline bool fgreater(double x, double y) {
	
	return (x - y) >= gu_epsilon_d;
}


// return -1.0 if x<0.0, 0.0 if x==0.0 (as determined by fequal) or 1.0 if x>0.0
inline float fsgn(float x) {
	
	return (fequal(x, 0.0f)) ? 0.0f : ((x < 0.0f) ? -1.0f : 1.0f);
}

inline double fsgn(double x) {
	
	return (fequal(x, 0.0)) ? 0.0 : ((x < 0.0) ? -1.0 : 1.0);
}


// round x to nearest whole number
inline float fround(float x) {

	return floor(x + 0.5f);
}

inline double fround(double x) {

	return floor(x + 0.5);
}


// 1/sqrt(x)
inline float invsqrt(float x) {
	
	float h = 0.5f * x;
	int i = *(int *)&x;
	i = 0x5f3759df - (i>>1);
	x = *(float *)&i;
	x = x * (1.5f - h * x * x);
	return x;
}


// x * 1/sqrt(x) = 1 / (1/sqrt(x)) = sqrt(x)
inline float fastsqrt(float x) {
	
	return x * invsqrt(x);
}


// Peachey's modified modulus function returns a mod b but ensures correct ordering for negative values of a (see Ebert et al. p.32).  It is assumed b > 0
inline float modP(float a, float b) {

	int n = int(a / b);

	a -= n * b;
	
	if (a < 0.0f)
		a += b;
	
	return a;
}

inline double modP(double a, double b) {

	int n = int(a / b);

	a -= n * b;
	
	if (a < 0.0)
		a += b;
	
	return a;
}


// the step function returns step(x, a) in the range [0, 1] where step(x, a) = 0 if x<a and step(x, a) = 1 otherwise
inline float step(float x, float a) {

	return (x < a) ? 0.0f : 1.0f;
}

inline double step(double x, double a) {

	return (x < a) ? 0.0 : 1.0;
}


// the inverse step function returns 1 if x < a and 0 otherwise
inline float inv_step(float x, float a) {

	return 1.0f - step(x, a);
}

inline double inv_step(double x, double a) {

	return 1.0 - step(x, a);
}



// ramp(x, a, b) returns a monotonically (linearly) increasing value in the range [0, 1] over the interval [a, b] (or [b, a] if b < a).  If x < a then ramp() returns 0.0 or if x >= b then ramp() returns 1.0.  If a == b (as determined by fequal) then the function reduces to the step function
inline float ramp(float x, float a, float b) {

	if (fequal(a, b))
		return step(x, a);
	
	float a_ = std::min<float>(a, b);
	float b_ = std::max<float>(a, b);

	if (x < a_)
		return 0.0f;
	else if (x >= b_)
		return 1.0f;
	else
		return (x - a_) / (b_ - a_);
}

inline double ramp(double x, double a, double b) {

	if (fequal(a, b))
		return step(x, a);

	double a_ = std::min<double>(a, b);
	double b_ = std::max<double>(a, b);

	if (x < a_)
		return 0.0;
	else if (x >= b_)
		return 1.0;
	else
		return (x - a_) / (b_ - a_);
}


// inv_ramp returns 1.0 - ramp(x, a, b) which results in a monotonically decreasing (linear) value in the range [0, 1] over the interval [a, b]
inline float inv_ramp(float x, float a, float b) {

	return 1.0f - ramp(x, a, b);
}

inline double inv_ramp(double x, double a, double b) {

	return 1.0 - ramp(x, a, b);
}


// smoothstep(x, a, b) returns a monotonically increasing sigmoidal curve in the range [0, 1] over the interval [a, b] (or [b, a] if b < a).  If x < a then smoothstep() returns 0.0 or if x >= b then smoothstep() returns 1.0.  If a == b (as determined by fequal) then the function reduces to the step function
inline float smoothstep(float x, float a, float b) {

	if (fequal(a, b))
		return step(x, a);

	float a_ = std::min<float>(a, b);
	float b_ = std::max<float>(a, b);

	if (x < a_)
		return 0.0f;
	else if (x >= b_)
		return 1.0f;
	else {

		float d = (x - a_) / (b_ - a_);
		return sqr(d) * (3.0f - 2.0f * d);
	}
}

inline double smoothstep(double x, double a, double b) {

	if (fequal(a, b))
		return step(x, a);

	double a_ = std::min<double>(a, b);
	double b_ = std::max<double>(a, b);

	if (x < a_)
		return 0.0;
	else if (x >= b_)
		return 1.0;
	else {

		double d = (x - a_) / (b_ - a_);
		return sqr(d) * (3.0 - 2.0 * d);
	}
}


// inv_smoothstep returns 1.0 - smoothstep(x, a, b) which results in a monotonically decreasing sigmoidal curve in the range [0, 1] over the interval [a, b]
inline float inv_smoothstep(float x, float a, float b) {

	return 1.0f - smoothstep(x, a, b);
}

inline double inv_smoothstep(double x, double a, double b) {

	return 1.0 - smoothstep(x, a, b);
}


// pulse() returns 1.0 when x lies in the interval [a, b] where a <= x <= b (or [b, a] where b <= x <= a if b < a), otherwise pulse() returns 0.0.  The bounary of the pulse region is determined by fequal.  note: Peachey's pulse macro (see Ebert et al p.28) does not work for a singularity at x = a = b so this case is handled separately
inline float pulse(float x, float a, float b) {

	if (fequal(a, b) && fequal(x, a)) // determine if x = a = b
		return 1.0f;
	else {

		float a_ = std::min<float>(a, b);
		float b_ = std::max<float>(a, b);

		return step(x, a_) - step(x, b_);
	}
}

inline double pulse(double x, double a, double b) {

	if (fequal(a, b) && fequal(x, a)) // determine if x = a = b
		return 1.0;
	else {

		double a_ = std::min<double>(a, b);
		double b_ = std::max<double>(a, b);

		return step(x, a_) - step(x, b_);
	}
}


// gamma correction transfer function maps x->gamma(x, g) where g = (0, +inf].  When g=1.0, x->x
inline float gamma(float x, float g) {

	return pow(x, 1.0f/g);
}

inline double gamma(double x, double g) {

	return pow(x, 1.0/g);
}


// Perlin and Hoffert's modified gamma transfer function (Ebert et al [89]) where x->bias(x, b) and b = (0, 1).  When b=0.5, x->x 
inline float bias(float x, float b) {

	return pow(x, log(b)/log(0.5f));
}


inline double bias(double x, double b) {

	return pow(x, log(b)/log(0.5));
}


// Perlin and Hoffert's gain transfer function (Ebert et al [89]) where x->gain(x, g) and g = (0, 1).  When g=0.5, x->x
inline float gain(float x, float g) {

	return (x<0.5f) ? bias(1.0f-g, 2.0f*x) / 2.0f : 1.0f - bias(1.0f-g, 2.0f - 2.0f*x) / 2.0f;
}


inline double gain(double x, double g) {

	return (x<0.5) ? bias(1.0-g, 2.0*x) / 2.0 : 1.0 - bias(1.0-g, 2.0 - 2.0*x) / 2.0;
}



// return the gaussian g(x) normalised to 1.0 at x=u with standard deviation sigma.  It is assumed sigma != 0.0 (as determined by fequal)
inline float gaussian(float x, float sigma, float u=0.0f) {

	return exp(-(sqr(x - u)) / (2.0f * sqr(sigma)));
}

inline double gaussian(double x, double sigma, double u=0.0) {

	return exp(-(sqr(x - u)) / (2.0 * sqr(sigma)));
}


// sigmoidal functions

// logistic curve f(x/sigma) where x=[-inf, +inf] and f(x/sigma) = (0, 1).  x is scaled by 1/sigma where sigma = (0, +inf].  It is assumed sigma != 0.0 (as determined by fequal)
inline float sigmoidL(float x, float sigma=1.0f) {

	return 1.0f / (1.0f + exp(-x / sigma));
}

inline double sigmoidL(double x, double sigma=1.0) {

	return 1.0 / (1.0 + exp(-x / sigma));
}


// error function approximation f(x/sigma) where x=[-inf, +inf] and f(x/sigma) = (-1, 1).  x is scaled by 1/sigma where sigma = (0, +inf].  It is assumed sigma != 0.0 (as determined by fequal)
inline float sigmoidE(float x, float sigma=1.0f) {

	return tanh(x/sigma);
}

inline double sigmoidE(double x, double sigma=1.0) {

	return tanh(x/sigma);
}


// cspline evaluates a 1D parametric cubic hermite (interpolating) spline.  This is a cardinal spline with tension coefficient c, where c = [0, 1].  When c = 0.0 (the default) cspline reduces to the Catmull-Rom interpolating spline.  When c=1.0 the resulting tangents are zero and the interpolation reduces to linear interpolation.  [Monotonicity can be approaximated when c>=0.5 (CHECK THIS)].  The spline parameter t is clamped to the interval [0, 1] which spans the control points (knot values) in p.  The knot values are uniformly distributed along the extent of the curve.  cspline models a 1D parametric curve so at least 4 knot values must be defined.  If not, the default value of T is returned.  cspline is templated and it is assumed operators <+ - * /> are defined on type T
template <class T>
inline T cspline(float t, int nKnots, const T *p, float c = 0.0f) {

	if (nKnots < 4)
		return T();

	t = clamp(t, 0.0f, 1.0f);

	int nSpans = nKnots - 3;

	t *= float(nSpans);

	int span = int(t);

	// make sure case where x=1 does not map to the wrong span (special case to deal with end of curve parameter value 1.0)
	if (span>=nSpans)
		span = nKnots - 4;

	int k = span + 1;

	t -= float(span); // local curve segment (span) parameter
	float td = 1.0f / float(nSpans); // uniform distance between each t[i] is 1.0f / (float)nSpans

	float tk = (float(k) - 1.0f) * td;

	T m0 = (p[k+1] - p[k-1]) / ((tk+td) - (tk-td)) * (1.0f - c); // gradient m0
	T m1 = (p[k+2] - p[k]) / ((tk + td*2.0f) - tk) * (1.0f - c); // gradient m1

	// cubic coefficients (calculated using Horner's rule)
	float h00 = (((2.0f*t - 3.0f) * t) * t) + 1.0f;
	float h10 = (((t - 2.0f) * t) + 1.0f) * t;
	float h01 = (-2.0f*t + 3.0f) * t * t;
	float h11 = (t - 1.0f) * t * t;

	return p[k]*h00 + m0*td*h10 + p[k+1]*h01 + m1*td*h11;
}

