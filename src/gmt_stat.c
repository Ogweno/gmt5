/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 *  Misc statistical and special functions.
 *
 * Author:	Walter H. F. Smith, P. Wessel, R. Scharroo
 * Date:	1-JAN-2010
 * Version:	5.x
 *
 * PUBLIC functions:
 *
 *	gmt_f_test :	Routine to compute the probability that two variances are the same
 *	gmt_f_test_new:	As above, but allows choosing 1- or 2-sided, and which side
 *	GMT_f_q:	Returns the probability integral Q(F,nu1,nu2) of the F-distribution.
 *	GMT_student_t_a:	Returns the prob integral A(t,nu) of the student-t distrib.
 *	GMT_sig_f :	Returns true if reduction in model misfit was significant
 *	GMT_bei:	Kelvin-Bessel function bei(x)
 *	GMT_ber:	Kelvin-Bessel function ber(x)
 *	GMT_kei:	Kelvin-Bessel function kei(x)
 *	GMT_ker:	Kelvin-Bessel function ker(x)
 *	GMT_plm:	Legendre polynomial of degree L order M
 *	GMT_plm_bar:	Normalized Legendre polynomial of degree L order M
 *	GMT_i0:		Modified Bessel function 1st kind order 0
 *	GMT_i1:		Modified Bessel function 1st kind order 1
 *	GMT_i2:		Modified Bessel function 1st kind order N
 *	GMT_k0:		Modified Kelvin function 2nd kind order 0
 *	GMT_k1:		Modified Kelvin function 2nd kind order 1
 *	GMT_kn:		Modified Kelvin function 2nd kind order N
 *	GMT_dilog:	The dilog function
 *	GMT_erfinv:	The inverse error function
 *	GMT_rand:	Uniformly distributed random numbers 0 < x < 1
 *	GMT_nrand:	Normally distributed random numbers from N(0,1)
 *	GMT_lrand:	Laplace random number generator
 *	GMT_corrcoeff:	Correlation coefficient.
 *	GMT_psi:	Digamma (psi) function.
 *	GMT_PvQv:	Legendre functions Pv and Qv for imaginary v and real x (-1/+1).
 *	GMT_factorial:	Factorials.
 */

#include "gmt_dev.h"
#include "gmt_internals.h"

int gmt_f_test_new (struct GMT_CTRL *GMT, double chisq1, uint64_t nu1, double chisq2, uint64_t nu2, double *prob, int iside)
{
	/* Given chisq1 and chisq2, random variables distributed as chi-square
		with nu1 and nu2 degrees of freedom, respectively, except that
		chisq1 is scaled by var1, and chisq2 is scaled by var2, let
		the null hypothesis, H0, be that var1 = var2.  This routine
		assigns prob, the probability that we can reject H0 in favor
		of a new hypothesis, H1, according to iside:
			iside=+1 means H1 is that var1 > var2
			iside=-1 means H1 is that var1 < var2
			iside=0  means H1 is that var1 != var2.
		This routine differs from the old gmt_f_test() by adding the
		argument iside and allowing one to choose the test.  The old
		routine in effect always set iside=0.
		This routine also differs from gmt_f_test() in that the former
		used the incomplete beta function and this one uses GMT_f_q().

		Returns 0 on success, -1 on failure.

		WHF Smith, 12 August 1999.
	*/

	double q;	/* The probability from GMT_f_q(), which is the prob
				that H0 should be retained even though
				chisq1/nu1 > chisq2/nu2.  */

	if (chisq1 <= 0.0 || chisq2 <= 0.0 || nu1 < 1 || nu2 < 1) {
		*prob = GMT->session.d_NaN;
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "gmt_f_test_new: Error: Bad argument(s).\n");
		return (-1);
	}

	GMT_f_q (GMT, chisq1, nu1, chisq2, nu2, &q);

	if (iside > 0)
		*prob = 1.0 - q;
	else if (iside < 0)
		*prob = q;
	else if ((chisq1/nu1) <= (chisq2/nu2))
		*prob = 2.0*q;
	else
		*prob = 2.0*(1.0 - q);

	return (0);
}

double gmt_cf_beta (struct GMT_CTRL *GMT, double a, double b, double x)
{
	/* Continued fraction method called by gmt_inc_beta.  */

	static int	itmax = 100;
	static double eps = 3.0e-7;

	double am = 1.0, bm = 1.0, az = 1.0;
	double qab, qap, qam, bz, em, tem, d;
	double ap, bp, app, bpp, aold;

	int m = 0;

	qab = a + b;
	qap = a + 1.0;
	qam = a - 1.0;
	bz = 1.0 - qab * x / qap;

	do {
		m++;
		em = (double)m;
		tem = em + em;
		d = em*(b-m)*x/((qam+tem)*(a+tem));
		ap = az+d*am;
		bp = bz+d*bm;
		d = -(a+m)*(qab+em)*x/((a+tem)*(qap+tem));
		app = ap+d*az;
		bpp = bp+d*bz;
		aold = az;
		am = ap/bpp;
		bm = bp/bpp;
		az = app/bpp;
		bz = 1.0;
	} while (((fabs (az-aold) ) >= (eps * fabs (az))) && (m < itmax));

	if (m == itmax) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "gmt_cf_beta:  A or B too big, or ITMAX too small.\n");

	return (az);
}

double gmt_ln_gamma (struct GMT_CTRL *GMT, double xx)
{
	/* Routine to compute natural log of Gamma(x)
		by Lanczos approximation.  Most accurate
		for x > 1; fails for x <= 0.  No error
		checking is done here; it is assumed
		that this is called by gmt_ln_gamma_r()  */

	static double cof[6] = {
		 76.18009173,
		-86.50532033,
		 24.01409822,
		 -1.231739516,
		0.120858003e-2,
		-0.536382e-5
	};

	static double stp = 2.50662827465, half = 0.5, one = 1.0, fpf = 5.5;
	double x, tmp, ser;

	int i;

	x = xx - one;
	tmp = x + fpf;
	tmp = (x + half) * d_log (GMT,tmp) - tmp;
	ser = one;
	for (i = 0; i < 6; i++) {
		x += one;
		ser += (cof[i]/x);
	}
	return (tmp + d_log (GMT,stp*ser) );
}

int gmt_ln_gamma_r (struct GMT_CTRL *GMT, double x, double *lngam)
{
	/* Get natural logrithm of Gamma(x), x > 0.
		To maintain full accuracy, this
		routine uses Gamma(1 + x) / x when
		x < 1.  This routine in turn calls
		gmt_ln_gamma(x), which computes the
		actual function value.  gmt_ln_gamma
		assumes it is being called in a
		smart way, and does not check the
		range of x.  */

	if (x > 1.0) {
		*lngam = gmt_ln_gamma (GMT, x);
		return (0);
	}
	if (x > 0.0 && x < 1.0) {
		*lngam = gmt_ln_gamma (GMT, 1.0 + x) - d_log (GMT,x);
		return (0);
	}
	if (x == 1.0) {
		*lngam = 0.0;
		return (0);
	}
	GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Ln Gamma:  Bad x (x <= 0).\n");
	return (-1);
}

int gmt_inc_beta (struct GMT_CTRL *GMT, double a, double b, double x, double *ibeta)
{
	double bt, gama, gamb, gamab;

	if (a <= 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "gmt_inc_beta:  Bad a (a <= 0).\n");
		return(-1);
	}
	if (b <= 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "gmt_inc_beta:  Bad b (b <= 0).\n");
		return(-1);
	}
	if (x > 0.0 && x < 1.0) {
		gmt_ln_gamma_r(GMT, a, &gama);
		gmt_ln_gamma_r(GMT, b, &gamb);
		gmt_ln_gamma_r(GMT, (a+b), &gamab);
		bt = exp(gamab - gama - gamb
			+ a * d_log (GMT, x) + b * d_log (GMT, 1.0 - x) );

		/* Here there is disagreement on the range of x which
			converges efficiently.  Abramowitz and Stegun
			say to use x < (a - 1) / (a + b - 2).  Editions
			of Numerical Recipes thru mid 1987 say
			x < ( (a + 1) / (a + b + 1), but the code has
			x < ( (a + 1) / (a + b + 2).  Editions printed
			late 1987 and after say x < ( (a + 1) / (a + b + 2)
			in text as well as code.  What to do ? */

		if (x < ( (a + 1) / (a + b + 2) ) )
			*ibeta = bt * gmt_cf_beta (GMT, a, b, x) / a;
		else
			*ibeta = 1.0 - bt * gmt_cf_beta (GMT, b, a, (1.0 - x) ) / b;
		return(0);
	}
	else if (x == 0.0) {
		*ibeta = 0.0;
		return (0);
	}
	else if (x == 1.0) {
		*ibeta = 1.0;
		return (0);
	}
	else if (x < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "gmt_inc_beta:  Bad x (x < 0).\n");
		*ibeta = 0.0;
	}
	else if (x > 1.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "gmt_inc_beta:  Bad x (x > 1).\n");
		*ibeta = 1.0;
	}
	return (-1);
}


int gmt_f_test (struct GMT_CTRL *GMT, double chisq1, uint64_t nu1, double chisq2, uint64_t nu2, double *prob)
{
	/* Routine to compute the probability that
		two variances are the same.
		chisq1 is distributed as chisq with
		nu1 degrees of freedom; ditto for
		chisq2 and nu2.  If these are independent
		and we form the ratio
		F = max(chisq1,chisq2)/min(chisq1,chisq2)
		then we can ask what is the probability
		that an F greater than this would occur
		by chance.  It is this probability that
		is returned in prob.  When prob is small,
		it is likely that the two chisq represent
		two different populations; the confidence
		that the two do not represent the same pop
		is 1.0 - prob.  This is a two-sided test.
	This follows some ideas in Numerical Recipes, CRC Handbook,
	and Abramowitz and Stegun.  */

	double f, df1, df2, p1, p2;

	if (chisq1 <= 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "gmt_f_test:  Chi-Square One <= 0.0\n");
		return(-1);
	}
	if (chisq2 <= 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "gmt_f_test:  Chi-Square Two <= 0.0\n");
		return(-1);
	}
	if (chisq1 > chisq2) {
		f = chisq1/chisq2;
		df1 = (double)nu1;
		df2 = (double)nu2;
	}
	else {
		f = chisq2/chisq1;
		df1 = (double)nu2;
		df2 = (double)nu1;
	}
	if (gmt_inc_beta(GMT, 0.5*df2, 0.5*df1, df2/(df2+df1*f), &p1) ) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "gmt_f_test:  Trouble on 1st gmt_inc_beta call.\n");
		return(-1);
	}
	if (gmt_inc_beta(GMT, 0.5*df1, 0.5*df2, df1/(df1+df2/f), &p2) ) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "gmt_f_test:  Trouble on 2nd gmt_inc_beta call.\n");
		return(-1);
	}
	*prob = p1 + (1.0 - p2);
	return (0);
}

int GMT_sig_f (struct GMT_CTRL *GMT, double chi1, uint64_t n1, double chi2, uint64_t n2, double level, double *prob)
{
	/* Returns true if chi1/n1 significantly less than chi2/n2
		at the level level.  Returns false if:
			error occurs in gmt_f_test_new();
			chi1/n1 not significantly < chi2/n2 at level.

			Changed 12 August 1999 to use gmt_f_test_new()  */

	int trouble;

	trouble = gmt_f_test_new (GMT, chi1, n1, chi2, n2, prob, -1);
	if (trouble) return (0);
	return ((*prob) >= level);
}

/* --------- LOWER LEVEL FUNCTIONS ------- */

#define ITMAX 100

void gmt_gamma_ser (struct GMT_CTRL *GMT, double *gamser, double a, double x, double *gln) {
	/* Returns the incomplete gamma function P(a,x) by series rep.
	 * Press et al, gser() */

	int n;
	double sum, del, ap;

	gmt_ln_gamma_r (GMT, a, gln);

	if (x < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT DOMAIN ERROR:  x < 0 in gmt_gamma_ser(x)\n");
		*gamser = GMT->session.d_NaN;
		return;
	}
	if (x == 0.0) {
		*gamser = 0.0;
		return;
	}
	ap = a;
	del = sum = 1.0 / a;
	for (n = 1; n <= ITMAX; n++) {
	 	ap += 1.0;
	 	del *= x / ap;
	 	sum += del;
	 	if (fabs (del) < fabs (sum) * DBL_EPSILON) {
	 		*gamser = sum * exp (-x  + a * log (x) - (*gln));
	 		return;
	 	}
	}
	GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT DOMAIN ERROR:  a too large, ITMAX too small in gmt_gamma_ser(x)\n");
}

void gmt_gamma_cf (struct GMT_CTRL *GMT, double *gammcf, double a, double x, double *gln) {
	/* Returns the incomplete gamma function P(a,x) by continued fraction.
	 * Press et al, gcf() */
	int n;
	double gold = 0.0, g, fac = 1.0, b1 = 1.0;
	double b0 = 0.0, anf, ana, an, a1, a0 = 1.0;

	gmt_ln_gamma_r (GMT, a, gln);

	a1 = x;
	for (n = 1; n <= ITMAX; n++) {
		an = (double) n;
		ana = an - a;
		a0 = (a1 + a0 * ana) * fac;
		b0 = (b1 + b0 * ana) * fac;
		anf = an * fac;
		a1 = x * a0 + anf * a1;
		b1 = x * b0 + anf * b1;
		if (a1 != 0.0) {
			fac = 1.0 / a1;
			g = b1 * fac;
			if (fabs ((g - gold) / g) < DBL_EPSILON) {
				*gammcf = exp (-x + a * log (x) - (*gln)) * g;
				return;
			}
			gold = g;
		}
	}
	GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT DOMAIN ERROR:  a too large, ITMAX too small in gmt_gamma_cf(x)\n");
}

double gmt_gammq (struct GMT_CTRL *GMT, double a, double x) {
	/* Returns Q(a,x) = 1 - P(a,x) Inc. Gamma function */

	double G, gln;

	if (x < 0.0 || a <= 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT DOMAIN ERROR:  Invalid arguments to GMT_gammaq\n");
		return (GMT->session.d_NaN);
	}

	if (x < (a + 1.0)) {
		gmt_gamma_ser (GMT, &G, a, x, &gln);
		return (1.0 - G);
	}
	gmt_gamma_cf (GMT, &G, a, x, &gln);
	return (G);
}

/*
 * Kelvin-Bessel functions, ber, bei, ker, kei.  ber(x) and bei(x) are even;
 * ker(x) and kei(x) are defined only for x > 0 and x >=0, respectively.
 * For x <= 8 we use polynomial approximations in Abramowitz & Stegun.
 * For x > 8 we use asymptotic series of Russell, quoted in Watson (Theory
 * of Bessel Functions).
 */

double GMT_ber (struct GMT_CTRL *GMT, double x) {
	double t, rxsq, alpha, beta;
	GMT_UNUSED(GMT);

	if (x == 0.0) return (1.0);

	/* ber is an even function of x:  */
	x = fabs (x);

	if (x <= 8.0) {
		/* Telescoped power series from Abramowitz & Stegun  */
		t = x * 0.125;
		t *= t;
		t *= t;	/* t = pow(x/8, 4)  */
		return (1.0 + t*(-64.0 + t*(113.77777774 + t*(-32.36345652 + t*(2.64191397 + t*(-0.08349609 + t*(0.00122552 - 0.00000901 * t)))))));
	}
	else {
		/* Russell's asymptotic approximation, from Watson, p. 204  */

		rxsq = 1.0 / (x * x);
		t = x / M_SQRT2;

		alpha = t;
		beta  = t - 0.125 * M_PI;
		t *= 0.125 * rxsq;
		alpha += t;
		beta  -= t;
		beta  -= 0.0625*rxsq;
		t *= (25.0/48.0)*rxsq;
		alpha -= t;
		beta  -= t;
		alpha -= (13.0/128.0)*(rxsq*rxsq);

		return (exp (alpha) * cos (beta) / sqrt (2.0 * M_PI * x) );
	}
}

double GMT_bei (struct GMT_CTRL *GMT, double x) {
	double t, rxsq, alpha, beta;
	GMT_UNUSED(GMT);

	if (x == 0.0) return (0.0);

	/* bei is an even function of x:  */
	x = fabs(x);

	if (x <= 8.0) {
		/* Telescoped power series from Abramowitz & Stegun  */
		t = x * 0.125;
		rxsq = t*t;
		t = rxsq * rxsq;	/* t = pow(x/8, 4)  */
		return (rxsq * (16.0 + t*(-113.77777774 + t*(72.81777742 + t*(-10.56765779 + t*(0.52185615 + t*(-0.01103667 + t*(0.00011346))))))));
	}
	else {
		/* Russell's asymptotic approximation, from Watson, p. 204  */

		rxsq = 1.0 / (x * x);
		t = x / M_SQRT2;

		alpha = t;
		beta  = t - 0.125 * M_PI;
		t *= 0.125 * rxsq;
		alpha += t;
		beta  -= t;
		beta  -= 0.0625*rxsq;
		t *= (25.0/48.0)*rxsq;
		alpha -= t;
		beta  -= t;
		alpha -= (13.0/128.0)*(rxsq*rxsq);

		return (exp (alpha) * sin (beta) / sqrt (2.0 * M_PI * x));
	}
}

double GMT_ker (struct GMT_CTRL *GMT, double x)
{
	double t, rxsq, alpha, beta;

	if (x <= 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT DOMAIN ERROR:  x <= 0 in GMT_ker(x)\n");
		return (GMT->session.d_NaN);
	}

	if (x <= 8.0) {
		/* Telescoped power series from Abramowitz & Stegun  */
		t = 0.125 * x;
		t *= t;
		t *= t;  /* t = pow(x/8, 4)  */
		return (-log (0.5 * x) * GMT_ber (GMT, x) + 0.25 * M_PI * GMT_bei (GMT, x) -M_EULER + \
			t * (-59.05819744 + t * (171.36272133 + t * (-60.60977451 + t * (5.65539121 + t * (-0.199636347 + t * (0.00309699 + t * (-0.00002458 * t))))))));
	}
	else {
		/* Russell's asymptotic approximation, from Watson, p. 204  */

		rxsq = 1.0 / (x * x);
		t = -x / M_SQRT2;

		alpha = t;
		beta  = t - 0.125 * M_PI;
		t *= 0.125 * rxsq;
		alpha += t;
		beta  -= t;
		beta  -= 0.0625*rxsq;
		t *= (25.0/48.0)*rxsq;
		alpha -= t;
		beta  -= t;
		alpha -= (13.0/128.0)*(rxsq*rxsq);

		return (exp (alpha) * cos (beta) / sqrt (2.0 * x / M_PI));
	}
}

double GMT_kei (struct GMT_CTRL *GMT, double x)
{
	double t, rxsq, alpha, beta;

	if (x <= 0.0) {
		/* Zero is valid.  If near enough to zero, return kei(0)  */
		if (x > -GMT_CONV8_LIMIT) return (-0.25 * M_PI);

		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT DOMAIN ERROR:  x < 0 in GMT_kei(x)\n");
		return (GMT->session.d_NaN);
	}

	if (x <= 8.0) {
		/* Telescoped power series from Abramowitz & Stegun  */
		t = x * 0.125;
		rxsq = t*t;
		t = rxsq * rxsq;	/* t = pow(x/8, 4)  */
		return (-log (0.5 * x) * GMT_bei (GMT, x) - 0.25 * M_PI * GMT_ber (GMT, x) +
			rxsq * (6.76454936 + t * (-142.91827687 + t * (124.23569650 + t * (-21.30060904 + t * (1.17509064 + t * (-0.02695875 + t * (0.00029532 * t))))))));
	}
	else {
		/* Russell's asymptotic approximation, from Watson, p. 204  */

		rxsq = 1.0 / (x * x);
		t = -x / M_SQRT2;

		alpha = t;
		beta  = t - 0.125 * M_PI;
		t *= 0.125 * rxsq;
		alpha += t;
		beta  -= t;
		beta  -= 0.0625*rxsq;
		t *= (25.0/48.0)*rxsq;
		alpha -= t;
		beta  -= t;
		alpha -= (13.0/128.0)*(rxsq*rxsq);

		return (exp (alpha) * sin (beta) / sqrt (2.0 * x / M_PI));
	}
}

double GMT_i0 (struct GMT_CTRL *GMT, double x) {
/* Modified from code in Press et al. */
	double y, res;
	GMT_UNUSED(GMT);

	if (x < 0.0) x = -x;

	if (x < 3.75) {
		y = x * x / 14.0625;
		res = 1.0 + y * (3.5156229 + y * (3.0899424 + y * (1.2067492 + y * (0.2659732 + y * (0.360768e-1 + y * 0.45813e-2)))));
	}
	else {
		y = 3.75 / x;
		res = (exp (x) / sqrt (x)) * (0.39894228 + y * (0.1328592e-1 + y * (0.225319e-2 + y * (-0.157565e-2 + y * (0.916281e-2 + y * (-0.2057706e-1 + y * (0.2635537e-1 + y * (-0.1647633e-1 + y * 0.392377e-2))))))));
	}
	return (res);
}

double GMT_i1 (struct GMT_CTRL *GMT, double x) {
	/* Modified Bessel function I1(x) */
	double y, res;
	GMT_UNUSED(GMT);

	if (x < 0.0) x = -x;
	if (x < 3.75) {
		y = pow (x / 3.75, 2.0);
		res = x * (0.5 + y * (0.87890594 + y * (0.51498869 + y * (0.15084934 + y * (0.02658733 + y * (0.00301532 + y * 0.00032411))))));
	}
	else {
		y = 3.75 / x;
		res = (exp (x) / sqrt (x)) * (0.39894228 + y * (-0.03988024 + y * (-0.00362018 + y * (0.00163801+ y * (-0.01031555 + y * (0.02282967 + y * (-0.02895312 + y * (0.01787654 -y * 0.00420059))))))));
		if (x < 0.0) res = - res;
	}
	return (res);
}

double GMT_in (struct GMT_CTRL *GMT, unsigned int n, double x)
{
	/* Modified Bessel function In(x) */

	unsigned int j, m, IACC = 40;
	double res, tox, bip, bi, bim;
	double BIGNO = 1.0e10, BIGNI = 1.0e-10;

	if (n == 0) return (GMT_i0 (GMT, x));
	if (n == 1) return (GMT_i1 (GMT, x));
	if (x == 0.0) return (0.0);

	tox = 2.0 / fabs (x);
	bip = res = 0.0;
	bi = 1.0;
	m = 2 * (n + urint (sqrt ((double)(IACC * n))));
	for (j = m; j >= 1; j--) {
		bim = bip + ((double)j) * tox * bi;
		bip = bi;
		bi = bim;
		if (fabs (bi) > BIGNO) {
			res *= BIGNI;
			bi *= BIGNI;
			bip *= BIGNI;
		}
		if (j == n) res = bip;
	}
	res *= (GMT_i0 (GMT, x) / bi);
	if (x < 0.0 && (n%2)) res = -res;

	return (res);
}

double GMT_k0 (struct GMT_CTRL *GMT, double x) {
/* Modified from code in Press et al. */
	double y, z, res;
	GMT_UNUSED(GMT);

	if (x < 0.0) x = -x;

	if (x <= 2.0) {
		y = 0.25 * x * x;
		z = x * x / 14.0625;
		res = (-log(0.5*x) * (1.0 + z * (3.5156229 + z * (3.0899424 + z * (1.2067492 + z * (0.2659732 + z * (0.360768e-1 + z * 0.45813e-2))))))) + (-M_EULER + y * (0.42278420 + y * (0.23069756 + y * (0.3488590e-1 + y * (0.262698e-2 + y * (0.10750e-3 + y * 0.74e-5))))));
	}
	else {
		y = 2.0 / x;
		res = (exp (-x) / sqrt (x)) * (1.25331414 + y * (-0.7832358e-1 + y * (0.2189568e-1 + y * (-0.1062446e-1 + y * (0.587872e-2 + y * (-0.251540e-2 + y * 0.53208e-3))))));
	}
	return (res);
}

double GMT_k1 (struct GMT_CTRL *GMT, double x)
{
	/* Modified Bessel function K1(x) */

	double y, res;

	if (x < 0.0) x = -x;
	if (x <= 2.0) {
		y = x * x / 4.0;
		res = (log (0.5 * x) * GMT_i1 (GMT, x)) + (1.0 / x) * (1.0 + y * (0.15443144 + y * (-0.67278579 + y * (-0.18156897 + y * (-0.01919402 + y * (-0.00110404 - y * 0.00004686))))));
	}
	else {
		y = 2.0 / x;
		res = (exp (-x) / sqrt (x)) * (1.25331414 + y * (0.23498619 + y * (-0.03655620 + y * (0.01504268 + y * (-0.00780353 + y * (0.00325614 - y * 0.00068245))))));
	}
	return (res);
}

double GMT_kn (struct GMT_CTRL *GMT, unsigned int n, double x)
{
	/* Modified Bessel function Kn(x) */

	unsigned int j;
	double bkm, bk, bkp, tox;

	if (n == 0) return (GMT_k0 (GMT, x));
	if (n == 1) return (GMT_k1 (GMT, x));

	tox = 2.0 / x;
	bkm = GMT_k0 (GMT, x);
	bk = GMT_k1 (GMT, x);
	for (j = 1; j <= (n-1); j++) {
		bkp = bkm + j * tox * bk;
		bkm = bk;
		bk = bkp;
	}

	return (bk);
}

double GMT_plm (struct GMT_CTRL *GMT, int l, int m, double x)
{
	/* Unnormalized associated Legendre polynomial of degree l and order m, including
	 * Condon-Shortley phase (-1)^m */
	double fact, pll = 0, pmm, pmmp1, somx2;
	int i, ll;

	/* x is cosine of colatitude and must be -1 <= x <= +1 */
	if (fabs(x) > 1.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: |x| > 1.0 in GMT_plm\n");
		return (GMT->session.d_NaN);
	}

	if (m < 0 || m > l) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: GMT_plm requires 0 <= m <= l.\n");
		return (GMT->session.d_NaN);
	}

	pmm = 1.0;
	if (m > 0) {
		somx2 = d_sqrt ((1.0 - x) * (1.0 + x));
		fact = 1.0;
		/* This loop used to go to i < m; corrected to i <= m by WHFS  */
		for (i = 1; i <= m; i++) {
			pmm *= -fact * somx2;
			fact += 2.0;
		}
	}
	if (l == m) return (pmm);

	pmmp1 = x * (2*m + 1) * pmm;
	if (l == (m + 1)) return (pmmp1);

	for (ll = (m+2); ll <= l; ll++) {
		pll = (x * (2*ll - 1) * pmmp1 - (ll + m - 1) * pmm) / (ll - m);
		pmm = pmmp1;
		pmmp1 = pll;
	}
	return (pll);
}

double GMT_plm_bar (struct GMT_CTRL *GMT, int l, int m, double x, bool ortho)
{
	/* This function computes the normalized associated Legendre function of x for degree
	 * l and order m. x must be in the range [-1;1] and 0 <= |m| <= l.
	 * The routine is largely based on the second modified forward column method described in
	 * Holmes and Featherstone (2002). It is stable up to very high degree and order
	 * (at least 3000). This is achieved by individually computing the sectorials to P_m,m
	 * and then iterate up to P_l,m divided by P_m,m and the scale factor 1e280.
	 * Eventually, the result is multiplied again with these two terms.
	 *
	 * When ortho=false, normalization is according to the geophysical convention.
	 * - The Condon-Shortley phase (-1)^m is NOT included.
	 * - The normalization factor is sqrt(k*(2l+1)*(l-m)!/(l+m)!) with k=2 except for m=0: k=1.
	 * - The integral of Plm**2 over [-1;1] is 2*k.
	 * - The integrals of (Plm*cos(m*lon))**2 and (Plm*sin(m*lon))**2 over a sphere are 4 pi.
	 *
	 * When ortho=true, the results are orthonormalized.
	 * - The Condon-Shortley phase (-1)^m is NOT included.
	 * - The normalization factor is sqrt((2l+1)*(l-m)!/(l+m)!/(4*pi)).
	 * - The integral of Plm**2 over [-1;1] is 1/(2*pi).
	 * - The integral of (Plm*exp(i*m*lon))**2 over a sphere is 1.
	 *
	 * When called with -m, the Condon-Shortley phase will be included.
	 *
	 * Note that the orthonormalized form produces an integral of 1 when using imaginary terms
	 * for longitude. In geophysics, the imaginary components are split into cosine and sine terms
	 * EACH of which have an average power of 1 over the sphere (i.e. an integral of 4 pi).
	 *
	 * This routine could be further expanded to produce an array of P_m,m through P_l,m. This
	 * would be practical if Legendre functions of various degrees are to be computed at the same
	 * latitude. Also, differentials could be added.
	 *
	 * Reference:
	 * S. A. Holmes and W. E. Featherstone. A unified approach to the Clenshaw summation and the
	 * recursive computation of very high degree and order normalised associated Legendre functions.
	 * Journal of Geodesy, 76, 279-299, 2002. doi:10.1007/s00190-002-0216-2.
	 */
	int i;
	bool csphase = false;
	double scalef = 1.0e280, u, r, pmm, pmm0, pmm1, pmm2;

	/* x is cosine of colatitude (sine of latitude) and must be -1 <= x <= +1 */

	if (fabs (x) > 1.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: |x| > 1.0 in GMT_plm_bar\n");
		return (GMT->session.d_NaN);
	}

	/* If m is negative, include Condon-Shortley phase */

	if (m < 0) {
		csphase = true;
		m = -m;
	}

	if (m > l) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: GMT_plm_bar requires 0 <= m <= l.\n");
		return (GMT->session.d_NaN);
	}

	/* u is sine of colatitude (cosine of latitude) so that 0 <= s <= 1 */

	u = d_sqrt ((1.0 - x)*(1.0 + x));

	/* Initialize P_00 = 1 and compute up to P_mm using recurrence relation.
	   The result is a normalisation factor: sqrt((2l+1)(l-m)!/(l+m)!) */

	pmm = 1.0;
	for (i = 1; i <= m; i++) pmm = d_sqrt (1.0 + 0.5/i) * u * pmm;

	/* If orthonormalization is requested: multiply by sqrt(1/4pi)
	   In case of geophysical conversion : multiply by sqrt(2-delta_0m) */

	if (ortho)
		pmm *= 0.5 / d_sqrt(M_PI);
	else if (m != 0)
		pmm *= d_sqrt(2.0);

	/* If C-S phase is requested, apply it now */

	if ((m & 1) && csphase) pmm = -pmm;

	/* If l==m, we are done */

	if (l == m) return (pmm);

	/* In the next section all P_l,m are divided by (P_m,m * scalef).
	   First compute P_m+1,m / P_m,m */

	pmm0 = 1.0/scalef;
	pmm1 = pmm0 * x * d_sqrt ((double)(2*m + 3));

	/* Use second modified column forward recurrence relation to compute P_m+2,m / P_m,m */

	for (i = m+2; i <= l; i++) {
		r = (2*i+1.0) / (i+m) / (i-m);
		pmm2 = x * pmm1 * d_sqrt(r*(2*i-1)) - pmm0 * d_sqrt(r*(i-m-1)*(i+m-1)/(2*i-3));
		pmm0 = pmm1;
		pmm1 = pmm2;
	}

	/* Return P_l,m */

	pmm1 *= pmm;
	pmm1 *= scalef;
	return (pmm1);
}

void GMT_plm_bar_all (struct GMT_CTRL *GMT, int lmax, double x, bool ortho, double *plm)
{
	/* This function computes the normalized associated Legendre function of x for all degrees
	 * l <= lmax and all orders m <= l. x must be in the range [-1;1] and 0 <= |m| <= l.
	 * The routine is largely based on the second modified forward column method described in
	 * Holmes and Featherstone (2002). It is stable up to very high degree and order
	 * (at least 3000). This is achieved by individually computing the sectorials to P_m,m
	 * and then iterate up to P_l,m divided by P_m,m and the scale factor 1e280.
	 * Eventually, the result is multiplied again with these two terms.
	 *
	 * When ortho=false, normalization is according to the geophysical convention.
	 * - The Condon-Shortley phase (-1)^m is NOT included.
	 * - The normalization factor is sqrt(k*(2l+1)*(l-m)!/(l+m)!) with k=2 except for m=0: k=1.
	 * - The integral of Plm**2 over [-1;1] is 2*k.
	 * - The integrals of (Plm*cos(m*lon))**2 and (Plm*sin(m*lon))**2 over a sphere are 4 pi.
	 *
	 * When ortho=true, the results are orthonormalized.
	 * - The Condon-Shortley phase (-1)^m is NOT included.
	 * - The normalization factor is sqrt((2l+1)*(l-m)!/(l+m)!/(4*pi)).
	 * - The integral of Plm**2 over [-1;1] is 1/(2*pi).
	 * - The integral of (Plm*exp(i*m*lon))**2 over a sphere is 1.
	 *
	 * When called with -lmax, the Condon-Shortley phase will be included.
	 *
	 * Note that the orthonormalized form produces an integral of 1 when using imaginary terms
	 * for longitude. In geophysics, the imaginary components are split into cosine and sine terms
	 * EACH of which have an average power of 1 over the sphere (i.e. an integral of 4 pi).
	 *
	 * This routine produces an array of all values of P_l,m(x) in the array plm, in the order
	 * P_0,0 P_1,0 P_1,1 P_2,0 P_2,1 P_2,2, etc. That means that plm[j] refers to:
	 * tesserals  P_l,m  when  j = l * (l+1) / 2 + m
	 * zonals     P_l,0  when  j = l * (l+1) / 2
	 * sectorials P_m,m  when  j = m * (m+3) / 2
	 *
	 * Reference:
	 * S. A. Holmes and W. E. Featherstone. A unified approach to the Clenshaw summation and the
	 * recursive computation of very high degree and order normalised associated Legendre functions.
	 * Journal of Geodesy, 76, 279-299, 2002. doi:10.1007/s00190-002-0216-2.
	 */
	int l, m, lm, mm;
	bool csphase = false;
	double scalef = 1.0e280, u, r, pmm, pmms, pmm0, pmm1, pmm2;

	/* x is cosine of colatitude (sine of latitude) and must be -1 <= x <= +1 */

	if (fabs (x) > 1.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: |x| > 1.0 in GMT_plm_bar_all\n");
		return;
	}

	/* If lmax is negative, include Condon-Shortley phase */

	if (lmax < 0) {
		csphase = true;
		lmax = -lmax;
	}

	/* u is sine of colatitude (cosine of latitude) so that 0 <= s <= 1 */

	u = d_sqrt ((1.0 - x)*(1.0 + x));

	/* Initialize P_00 = 1 */
	mm = 0;
	plm[mm] = pmm = 1.0;

	/* Loop over 0 <= m <= lmax */

	for (m = 0; m <= lmax; m++, mm += m+1) {

		/* Compute up P_mm using recurrence relation.
		The result is a normalisation factor: sqrt((2l+1)(l-m)!/(l+m)!) */

		if (m != 0) pmm *= d_sqrt (1.0 + 0.5/m) * u;

		/* If orthonormalization is requested: multiply by sqrt(1/4pi)
		In case of geophysical conversion : multiply by sqrt(2-delta_0m) */

		if (ortho)
			plm[mm] = pmm * 0.5 / d_sqrt(M_PI);
		else if (m != 0)
			plm[mm] = pmm * d_sqrt(2.0);

		/* If C-S phase is requested, apply it now */

		if ((m & 1) && csphase) plm[mm] = -plm[mm];

		/* In the next section all P_l,m are divided by (P_m,m * scalef).
		First compute P_m+1,m / P_m,m */

		pmms = plm[mm] * scalef;
		pmm0 = 1.0 / scalef;
		pmm1 = pmm0 * x * d_sqrt ((double)(2*m + 3));
		lm = mm + m + 1;
		plm[lm] = pmm1 * pmms;

		/* Use second modified column forward recurrence relation to compute P_m+2,m / P_m,m */

		for (l = m+2; l <= lmax; l++) {
			r = (2*l+1.0) / (l+m) / (l-m);
			pmm2 = x * pmm1 * d_sqrt(r*(2*l-1)) - pmm0 * d_sqrt(r*(l-m-1)*(l+m-1)/(2*l-3));
			pmm0 = pmm1;
			pmm1 = pmm2;
			lm += l;
			plm[lm] = pmm1 * pmms;
		}
	}
}

/* GMT_sinc (x) calculates the sinc function */

double GMT_sinc (struct GMT_CTRL *GMT, double x) {
	GMT_UNUSED(GMT);
	if (x == 0.0) return (1.0);
	x *= M_PI;
	return (sin (x) / x);
}

/* GMT_factorial (n) calculates the factorial n! */

double GMT_factorial (struct GMT_CTRL *GMT, int n) {
	int i;
	double val = 1.0;

	if (n < 0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: n < 0 in GMT_factorial(n)\n");
		return (GMT->session.d_NaN);
		/* This could be set to return 0 without warning, to facilitate
			sums over binomial coefficients, if desired.  -whfs  */
	}

	for (i = 1; i <= n; i++) val *= ((double)i);
	return (val);
}

double GMT_dilog (struct GMT_CTRL *GMT, double x)
{
	/* Compute dilog(x) (defined for x >= 0) by the method of Parker's
	   Appendix A of his Geophysical Inverse Theory.  The function
	   is needed for x in the range 0 <= x <= 1 when solving the
	   spherical spline interpolation in section 2.07 of Parker.

	   I tested this for x in range 0 to 1 by reproducing Figure
	   2.07c of Parker's book.  I also tested that the result was
	   smooth for x out to 25.  I also checked d[dilog(x)]/dx
	   obtained numerically from this routine against an analytic
	   expression, and I found the smallest angular separation
	   between two points on a sphere such that their Gram
	   matrix expression (see Parker) was different from 1; this
	   is smallest epsilon such that dilog(epsilon) - dilog(0)
	   != 0.0.  This turned out to be very much smaller than I
	   would have guessed from the apparent e-15 accuracy of
	   Parker's polynomial.  So I think this is very accurate.

	   If this is called with x < 0, it returns dilog(0) and
	   prints a warning.  It could also be set to return NaN.
	   I did this because in applications it might happen that
	   an x which should be zero is computed to be slightly off,
	   in which case the desired result is for x = 0.

	   23 Oct 1996 WHFS.   */

	double pisqon6, y, ysq, z;

	if (x < -GMT_CONV8_LIMIT) return (GMT->session.d_NaN);	/* Tolerate minor slop before we are outside domain */

	pisqon6 = M_PI * M_PI / 6.0;
	if (x <= 0.0) return (pisqon6);	/* Meaning -GMT_CONV8_LIMIT < x <= 0 */

	if (x < 0.5) {
		y = -log (1.0 - x);
		ysq = y * y;
		z = y * (1.0 + y * (-0.25 + y * (0.027777777777213 + ysq * (-2.7777776990e-04 + ysq * (4.724071696e-06 + ysq * (-9.1764954e-08 + 1.798670e-09 * ysq))))));
		return (pisqon6 - z + y * log (x));
	}
	if (x < 2.0) {
		y = -log (x);
		ysq = y * y;
		z = y * (1.0 + y * (-0.25 + y * (0.027777777777213 + ysq * (-2.7777776990e-04 + ysq * (4.724071696e-06 + ysq * (-9.1764954e-08 + 1.798670e-09 * ysq))))));
		return (z);
	}
	y = log (x);
	ysq = y * y;
	z = y * (1.0 + y * (-0.25 + y * (0.027777777777213 + ysq * (-2.7777776990e-04 + ysq * (4.724071696e-06 + ysq * (-9.1764954e-08 + 1.798670e-09 * ysq))))));
	return (-z - 0.5 * ysq);
}

#ifndef M_2_SQRTPI
#define  M_2_SQRTPI      1.12837916709551257390
#endif

double GMT_erfinv (struct GMT_CTRL *GMT, double y)
{
	double x = 0.0, fy, z;

	/*  Misc. efficients for expansion */

	static double a[4] = {0.886226899, -1.645349621,  0.914624893, -0.140543331};
	static double b[4] = {-2.118377725, 1.442710462, -0.329097515, 0.012229801};
	static double c[4] = {-1.970840454, -1.624906493, 3.429567803, 1.641345311};
	static double d[2] = {3.543889200, 1.637067800};

	fy = fabs (y);

	if (fy > 1.0) return (GMT->session.d_NaN);	/* Outside domain */

	if (doubleAlmostEqual (fy, 1.0))
		return (copysign (DBL_MAX, y));	/* Close to +- Inf */

	if (y > 0.7) {		/* Near upper range */
		z = sqrt (-log (0.5 * (1.0 - y)));
		x = (((c[3] * z + c[2]) * z + c[1]) * z + c[0]) / ((d[1] * z + d[0]) * z + 1.0);
	}
	else if (y < -0.7) {	/* Near lower range */
		z = sqrt (-log (0.5 * (1.0 + y)));
		x = -(((c[3] * z + c[2]) * z + c[1]) * z + c[0]) / ((d[1] * z + d[0]) * z + 1.0);
	}
	else {			/* Central range */
		z = y * y;
		x = y * (((a[3] * z + a[2]) * z + a[1]) * z + a[0]) / ((((b[3] * z + b[2]) * z + b[1]) * z + b[0]) * z + 1.0);
	}

	/* Apply two steps of Newton-Raphson correction to improve accuracy */

	x -= (erf (x) - y) / (M_2_SQRTPI * exp (-x * x));
	x -= (erf (x) - y) / (M_2_SQRTPI * exp (-x * x));

	return (x);
}

int GMT_f_q (struct GMT_CTRL *GMT, double chisq1, uint64_t nu1, double chisq2, uint64_t nu2, double *prob)
{
	/* Routine to compute Q(F, nu1, nu2) = 1 - P(F, nu1, nu2), where nu1
		and nu2 are positive integers, chisq1 and chisq2 are random
		variables having chi-square distributions with nu1 and nu2
		degrees of freedom, respectively (chisq1 and chisq2 >= 0.0),
		F = (chisq1/nu1)/(chisq2/nu2) has the F-distribution, and
		P(F, nu1, nu2) is the cumulative F-distribution, that is,
		the integral from 0 to (chisq1/nu1)/(chisq2/nu2) of the F-
		distribution.  Q = 1 - P is small when (chisq1/nu1)/(chisq2/nu2)
		is large with respect to 1.  That is, the value returned by
		this routine is the likelihood that an F >= (chisq1/nu1)/
		(chisq2/nu2) would occur by chance.

		Follows Abramowitz and Stegun.
		This is different from the method in Numerical Recipes, which
		uses the incomplete beta function but makes no use of the fact
		that nu1 and nu2 are known to be integers, and thus there is
		a finite limit on the sum for their expression.

		W H F Smith, August, 1999.

		REVISED by W H F Smith, October 27, 2000 after GMT 3.3.6 release.
		I found that the A&S methods overflowed for large nu1 and nu2, so
		I decided to go back to the gmt_inc_beta way of doing things.

	*/

	/* Check range of arguments:  */

	if (nu1 <= 0 || nu2 <= 0 || chisq1 < 0.0 || chisq2 < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT_f_q:  Bad argument(s).\n");
		return (-1);
	}

	/* Extreme cases evaluate immediately:  */

	if (chisq1 == 0.0) {
		*prob = 1.0;
		return (0);
	}
	if (chisq2 == 0.0) {
		*prob = 0.0;
		return (0);
	}

	/* REVISION of Oct 27, 2000:  This inc beta call here returns
		the value.  All subsequent code is not used.  */

	if (gmt_inc_beta (GMT, 0.5*nu2, 0.5*nu1, chisq2/(chisq2+chisq1), prob) ) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT_q_p:  Trouble in gmt_inc_beta call.\n");
		return (-1);
	}
	return (0);
}

int GMT_student_t_a (struct GMT_CTRL *GMT, double t, uint64_t n, double *prob)
{
	/* Probability integral called A(t,n) by Abramowitz &
	Stegun for the student's t distribution with n degrees
	of freedom.  Uses expressions A&S 26.7.3 and 26.7.4

	If X is distributed N(0,1) and V is distributed chi-
	square with n degrees of freedom, then
	tau = X / sqrt(V/n) is said to have Student's t-
	distribution with n degrees of freedom.  For example,
	tau could be the sample mean divided by the sample
	standard deviation, for a sample of N points; then
	n = N - 1.

	This function sets *prob = GMT->session.d_NaN and returns (-1)
	if t < 0.  Otherwise it sets *prob = the probability
	fabs(tau) <= t and returns (0).

	As n -> oo, we can replace this function with
	erf (t / M_SQRT2).  However, it isn't clear how large
	n has to be to make this a good approximation.  I
	consulted six books; one of them suggested this
	approximation for n >= 30, but all the others did not
	say when to use this approximation (A&S, in particular,
	does not say).  I tried some numerical experiments
	which suggested that the relative error in this
	approximation would be < 0.01 for n > 30, all t, but
	I also found that the expression here is stable to
	large n and large t, so I decided to leave it as is.

	W H F Smith, August 1999.
*/

	double	theta, s, c, csq, term, sum;
	int64_t	k, kstop, kt, kb;

	if (t < 0.0 || n == 0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT_student_t_a:  Bad argument(s).\n");
		*prob = GMT->session.d_NaN;
		return (-1);
	}

	if (t == 0.0) {
		*prob = 0.0;
		return (0);
	}

	theta = atan (t/sqrt ((double)n));

	if (n == 1) {
		*prob = 2.0 * theta / M_PI;
		return (0);
	}

	sincos (theta, &s, &c);

	csq = c * c;

	kstop = n-2;
	if (n%2 == 1) {
		kt = 0;
		kb = 1;
		k = 1;
		term = c;
	}
	else {
		kt = -1;
		kb = 0;
		k = 0;
		term = 1.0;
	}
	sum = term;
	while (k < kstop) {
		k += 2;
		kt += 2;
		kb += 2;
		term *= (kt * csq)/kb;
		sum += term;
	}

	sum *= s;

	if (n%2 == 1)
		*prob = 2.0 * (theta + sum) / M_PI;
	else
		*prob = sum;

	/* Adjust in case of roundoff:  */

	if (*prob < 0.0) *prob = 0.0;
	if (*prob > 1.0) *prob = 1.0;

	return (0);
}

double GMT_zdist (struct GMT_CTRL *GMT, double x)
{
	/* Cumulative Normal (z) distribution */
	GMT_UNUSED(GMT);
	return (0.5 * (erf (x / M_SQRT2) + 1.0));
}

double GMT_zcrit (struct GMT_CTRL *GMT, double alpha)
{
	double sign;
	/* Critical values for Normal (z) distribution */

	/* Simple since z_a = M_SQRT2 * erf^-1 (1-a) */

	if (alpha > 0.5) {	/* right tail */
		alpha = (1.0 - alpha) * 2.0;
		sign = 1.0;
	}
	else {			/* left tail */
		alpha *= 2.0;
		sign = -1.0;
	}

	return (sign * M_SQRT2 * GMT_erfinv (GMT, 1.0 - alpha));
}

double GMT_tcrit (struct GMT_CTRL *GMT, double alpha, double nu)
{
	/* Critical values for Student t-distribution */

	int NU;
	bool done;
	double t_low, t_high, t_mid = 0.0, p_high, p_mid, p, sign;

	if (alpha > 0.5) {	/* right tail */
		p = 1 - (1 - alpha) * 2.0;
		sign = 1.0;
	}
	else {
		p = 1 - alpha * 2.0;
		sign = -1.0;
	}
	t_low = GMT_zcrit (GMT, alpha);
	t_high = 5.0;
	NU = irint(nu);
	GMT_student_t_a (GMT, t_high, NU, &p_high);
	while (p_high < p) {	/* Must pick higher starting point */
		t_high *= 2.0;
		GMT_student_t_a (GMT, t_high, NU, &p_high);
	}

	/* Now, (t_low, p_low) and (t_high, p_high) are bracketing the desired (t,p) */

	done = false;
	while (!done) {
		t_mid = 0.5 * (t_low + t_high);
		GMT_student_t_a (GMT, t_mid, NU, &p_mid);
		if (doubleAlmostEqualZero (p_mid, p)) {
			done = true;
		}
		else if (p_mid > p) {	/* Too high */
			t_high = t_mid;
		}
		else { /* p_mid < p */
			t_low = t_mid;
		}
	}
	return (sign * t_mid);
}

double GMT_chi2crit (struct GMT_CTRL *GMT, double alpha, double nu)
{
	/* Critical values for Chi^2-distribution */

	bool done;
	double chi2_low, chi2_high, chi2_mid = 0.0, p_high, p_mid, p;

	p = 1.0 - alpha;
	chi2_low = 0.0;
	chi2_high = 5.0;
	GMT_chi2 (GMT, chi2_high, nu, &p_high);
	while (p_high > p) {	/* Must pick higher starting point */
		chi2_high *= 2.0;
		GMT_chi2 (GMT, chi2_high, nu, &p_high);
	}

	/* Now, (chi2_low, p_low) and (chi2_high, p_high) are bracketing the desired (chi2,p) */

	done = false;
	while (!done) {
		chi2_mid = 0.5 * (chi2_low + chi2_high);
		GMT_chi2 (GMT, chi2_mid, nu, &p_mid);
		if (doubleAlmostEqualZero (p_mid, p)) {
			done = true;
		}
		else if (p_mid < p) {	/* Too high */
			chi2_high = chi2_mid;
		}
		else { /* p_mid > p */
			chi2_low = chi2_mid;
		}
	}
	return (chi2_mid);
}

void gmt_F_to_ch1_ch2 (struct GMT_CTRL *GMT, double F, double nu1, double nu2, double *chisq1, double *chisq2)
{	/* Silly routine to break F up into parts needed for GMT_f_q */
	GMT_UNUSED(GMT);
	*chisq2 = 1.0;
	*chisq1 = F * nu1 / nu2;
}

double GMT_Fcrit (struct GMT_CTRL *GMT, double alpha, double nu1, double nu2)
{
	/* Critical values for F-distribution */

	int NU1, NU2;
	bool done;
	double F_low, F_high, F_mid = 0.0, p_high, p_mid, p, chisq1, chisq2;

	F_high = 5.0;
	p = 1.0 - alpha;
	F_low = 0.0;
	gmt_F_to_ch1_ch2 (GMT, F_high, nu1, nu2, &chisq1, &chisq2);
	NU1 = irint (nu1);
	NU2 = irint (nu2);
	GMT_f_q (GMT, chisq1, NU1, chisq2, NU2, &p_high);
	while (p_high > p) {	/* Must pick higher starting point */
		F_high *= 2.0;
		gmt_F_to_ch1_ch2 (GMT, F_high, nu1, nu2, &chisq1, &chisq2);
		GMT_f_q (GMT, chisq1, NU1, chisq2, NU2, &p_high);
	}

	/* Now, (F_low, p_low) and (F_high, p_high) are bracketing the desired (F,p) */

	done = false;
	while (!done) {
		F_mid = 0.5 * (F_low + F_high);
		gmt_F_to_ch1_ch2 (GMT, F_mid, nu1, nu2, &chisq1, &chisq2);
		GMT_f_q (GMT, chisq1, NU1, chisq2, NU2, &p_mid);
		if (doubleAlmostEqualZero (p_mid, p)) {
			done = true;
		}
		else if (p_mid < p) {	/* Too high */
			F_high = F_mid;
		}
		else { /* p_mid > p */
			F_low = F_mid;
		}
	}
	return (F_mid);
}

static inline uint64_t mix64 (uint64_t a, uint64_t b, uint64_t c) {
	/* Mix 3 64-bit values, from lookup8.c by Bob Jenkins
	 * (http://burtleburtle.net/bob/index.html) */
	a -= b; a -= c; a ^= (c>>43);
	b -= c; b -= a; b ^= (a<<9);
	c -= a; c -= b; c ^= (b>>8);
	a -= b; a -= c; a ^= (c>>38);
	b -= c; b -= a; b ^= (a<<23);
	c -= a; c -= b; c ^= (b>>5);
	a -= b; a -= c; a ^= (c>>35);
	b -= c; b -= a; b ^= (a<<49);
	c -= a; c -= b; c ^= (b>>11);
	a -= b; a -= c; a ^= (c>>12);
	b -= c; b -= a; b ^= (a<<18);
	c -= a; c -= b; c ^= (b>>22);
	return c;
}

double GMT_rand (struct GMT_CTRL *GMT) {
	/* Uniform random number generator.  Will return values
	 * x so that 0.0 < x < 1.0 occurs with equal probability. */
	static unsigned seed = 0;
	double random_val;

	while (seed == 0) { /* repeat in case of unsigned overflow */
		/* Initialize random seed, idea from Jonathan
		 * Wright (http://stackoverflow.com/q/322938) */
		seed = (unsigned) mix64 (clock(), time(NULL), getpid());
		srand (seed);
	}

	/* generate random number */
	random_val = rand () / (double) RAND_MAX;

	if (random_val == 0.0 || random_val >= 1.0)
		/* Ensure range (0.0,1.0) */
		return GMT_rand (GMT);

	return random_val;
}

double GMT_nrand (struct GMT_CTRL *GMT) {
	/* Gaussian random number generator based on gasdev of
	 * Press et al, Numerical Recipes, 2nd edition.  Will
	 * return values that have zero mean and unit variance.
	 */

	static int iset = 0;
	static double gset;
	double fac, r, v1, v2;

	if (iset == 0) {	/* We don't have an extra deviate handy, so */
		do {
			v1 = 2.0 * GMT_rand (GMT) - 1.0;	/* Pick two uniform numbers in the -1/1/-1/1 square */
			v2 = 2.0 * GMT_rand (GMT) - 1.0;
			r = v1 * v1 + v2 * v2;
		} while (r >= 1.0 || r == 0.0);	/* Keep trying until v1,v2 is inside unit circle */

		fac = sqrt (-2.0 * log (r) / r);

		/* Now make Box-Muller transformation to get two normal deviates.  Return
		 * one and save the other for the next time GMT_nrand is called */

		gset = v1 * fac;
		iset = 1;	/* Set flag for next time */
		return (v2 * fac);
	}
	else {
		iset = 0;	/* Take old value, reset flag */
		return (gset);
	}
}

double GMT_lrand (struct GMT_CTRL *GMT) {
	/* Laplace random number generator.  As nrand, it will
	 * return values that have zero mean and unit variance.
	 */

	double rand_0_to_1;

	rand_0_to_1 = GMT_rand (GMT);	/* Gives uniformly distributed random values in 0-1 range */
	return (((rand_0_to_1 <= 0.5) ? log (2.0 * rand_0_to_1) : -log (2.0 * (1.0 - rand_0_to_1))) / M_SQRT2);
}

void GMT_chi2 (struct GMT_CTRL *GMT, double chi2, double nu, double *prob) {
	/* Evaluate probability that chi2 will exceed the
	 * theoretical chi2 by chance. */

	*prob = gmt_gammq (GMT, 0.5 * nu, 0.5 * chi2);
}

void GMT_cumpoisson (struct GMT_CTRL *GMT, double k, double mu, double *prob) {
	/* evaluate Cumulative Poisson Distribution */

	*prob = (k == 0.0) ? exp (-mu) : gmt_gammq (GMT, k, mu);
}

double GMT_mean_and_std (struct GMT_CTRL *GMT, double *x, uint64_t n, double *std)
{	/* Return the standard deviation of the non-NaN values in x */
	uint64_t k, m = 0;
	double dx, mean = 0.0, sum2 = 0.0;
	for (k = 0; k < n; k++) {	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
		if (GMT_is_dnan (x[k])) continue;
		m++;
		dx = x[k] - mean;
		mean += dx / m;
		sum2 += dx * (x[k] - mean);
	}
	*std = (m > 1) ? sqrt (sum2 / (m-1.0)) : GMT->session.d_NaN;
	return ((m) ? mean : GMT->session.d_NaN);
}

int GMT_median (struct GMT_CTRL *GMT, double *x, uint64_t n, double xmin, double xmax, double m_initial, double *med)
{
	double lower_bound, upper_bound, m_guess, t_0, t_1, t_middle;
	double lub, glb, xx, temp;
	uint64_t i;
	int64_t n_above, n_below, n_equal, n_lub, n_glb, one;	/* These must be signed integers (PW: Why? -> (unsigned)x - (unsigned)y will never become negative) */
	int iteration = 0;
	bool finished = false;

	if (n == 0) {	/* No data, so no defined median */
		*med = GMT->session.d_NaN;
		return (1);
	}
	if (n == 1) {
		*med = x[0];
		return (1);
	}
	if (n == 2) {
		*med = 0.5 * (x[0] + x[1]);
		return (1);
	}

	m_guess = m_initial;
	lower_bound = xmin;
	upper_bound = xmax;
	one = 1;
	t_0 = 0.0;
	t_1 = (double)(n - one);
	t_middle = 0.5 * t_1;

	do {

		n_above = n_below = n_equal = n_lub = n_glb = 0;
		lub = xmax;
		glb = xmin;

		for (i = 0; i < n; i++) {

			xx = x[i];
			if (xx == m_guess)
				n_equal++;
			else if (xx > m_guess) {
				n_above++;
				if (xx < lub) {
					lub = xx;
					n_lub = one;
				}
				else if (xx == lub)
					n_lub++;
			}
			else {
				n_below++;
				if (xx > glb) {
					glb = xx;
					n_glb = one;
				}
				else if (xx == glb)
					n_glb++;
			}
		}

		iteration++;

		/* Now test counts, watch multiple roots, think even/odd:  */

		if ((int64_abs (n_above - n_below)) <= n_equal) {
			*med = (n_equal) ? m_guess : 0.5 * (lub + glb);
			finished = true;
		}
		else if ((int64_abs ((n_above - n_lub) - (n_below + n_equal))) < n_lub) {
			*med = lub;
			finished = true;
		}
		else if ((int64_abs ((n_below - n_glb) - (n_above + n_equal))) < n_glb) {
			*med = glb;
			finished = true;
		}
		/* Those cases found the median; the next two will forecast a new guess:  */

		else if (n_above > (n_below + n_equal)) {  /* Guess is too low  */
			lower_bound = m_guess;
			t_0 = (double)(n_below + n_equal - one);
			temp = lower_bound + (upper_bound - lower_bound) * (t_middle - t_0) / (t_1 - t_0);
			m_guess = (temp > lub) ? temp : lub;	/* Move guess at least to lub  */
		}
		else if (n_below > (n_above + n_equal)) {  /* Guess is too high  */
			upper_bound = m_guess;
			t_1 = (double)(n_below + n_equal - one);
			temp = lower_bound + (upper_bound - lower_bound) * (t_middle - t_0) / (t_1 - t_0);
			m_guess = (temp < glb) ? temp : glb;	/* Move guess at least to glb  */
		}
		else {	/* If we get here, I made a mistake!  */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Internal goof in GMT_median; please report to developers!\n");
			GMT_exit (GMT, EXIT_FAILURE); return EXIT_FAILURE;
		}

	} while (!finished);

	/* That's all, folks!  */
	return (iteration);
}

int compare_observation (const void *a, const void *b)
{
	const struct OBSERVATION *obs_1 = a, *obs_2 = b;

	/* Sorts observations into ascending order based on obs->value */
	if (obs_1->value < obs_2->value)
		return -1;
	if (obs_1->value > obs_2->value)
		return 1;
	return 0;
}

double GMT_median_weighted (struct GMT_CTRL *GMT, struct OBSERVATION *data, uint64_t n, double quantile)
{
	uint64_t k;
	double weight_half = 0.0, weight_count;

	if (n == 0) return (GMT->session.d_NaN);	/* No data, so no defined mode */

	/* First sort data on z */

	qsort (data, n, sizeof (struct OBSERVATION), compare_observation);

	/* Find weight sum, then get half-value */

	for (k = 0; k < n; k++) weight_half += data[k].weight;
	weight_half *= quantile;	/* Normally quantile = 0.5 hence the name "half" */

	/* Determine the point where we hit the desired quantile */

	k = 0;	weight_count = data[k].weight;
	while (weight_count < weight_half) weight_count += data[++k].weight;	/* Wind up until weight_count hits the mark */

	return ((double)((weight_count == weight_half) ? 0.5 * (data[k].value + data[k+1].value) : data[k].value));
}

double GMT_mode_weighted (struct GMT_CTRL *GMT, struct OBSERVATION *data, uint64_t n)
{
	/* Looks for the "shortest 50%". This means that when the cumulative weight
	   (y) is plotted against the value (x) then the line between (xi,yi) and
	   (xj,yj) should be the steepest for any combination where (yj-yi) is 50%
	   of the total sum of weights */

	double top, bottom, wsum, p, p_max, mode;
	uint64_t i, j;

	if (n == 0) return (GMT->session.d_NaN);	/* No data, so no defined mode */

	/* First sort data on z */
	qsort (data, n, sizeof (struct OBSERVATION), compare_observation);

	/* Compute the total weight */
	for (wsum = 0.0, i = 0; i < n; i++) wsum += data[i].weight;

	/* Do some initializations */
	wsum = 0.5 * wsum;	/* Sets the 50% range */

	/* First check if any single point has 50% or more of the total weights; if so we are done */
	for (i = 0; i < n; i++) if (data[i].weight >= wsum) return data[i].value;
	
	/* Some more initializations */
	top = p_max = 0.0;
	mode = 0.5 * (data[0].value + data[n-1].value);

	for (i = j = 0; j < n; j++) {
		top += data[j].weight;
		if (top < wsum) continue;
		while (top > wsum && i < j) top -= data[i++].weight;
		bottom = data[j].value - data[i].value;

		/* If all is comprised in one point or if a lot of values are the same,
		   then we have a spike. Maybe another logic is needed to handle
		   multiple spikes in the data */
		if (bottom == 0.0) return (data[i].value);

		p = top / bottom;
		if (p > p_max) {
			p_max = p;
			mode = 0.5 * (data[i].value + data[j].value);
		}
	}
	return (mode);
}

int GMT_mode (struct GMT_CTRL *GMT, double *x, uint64_t n, uint64_t j, bool sort, unsigned int mode_selection, unsigned int *n_multiples, double *mode_est)
{
	uint64_t i, istop;
	unsigned int multiplicity;
	double mid_point_sum = 0.0, length, short_length = DBL_MAX, this_mode;

	if (n == 0) {	/* No data, so no defined mode */
		*mode_est = GMT->session.d_NaN;
		return (0);
	}
	if (n == 1) {
		*mode_est = x[0];
		return (0);
	}

	if (sort) GMT_sort_array (GMT, x, n, GMT_DOUBLE);

	istop = n - j;
	multiplicity = 0;

	for (i = 0; i < istop; i++) {
		length = x[i + j] - x[i];
		if (length < 0.0) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT_mode: Array not sorted in non-decreasing order.\n");
			return (-1);
		}
		else if (length == short_length) {	/* Possibly multiple mode */
			switch (mode_selection) {
				case -1:	/* Always pick lowest mode */
					this_mode = 0.5 * (x[i + j] + x[i]);
					if (this_mode < mid_point_sum) mid_point_sum = this_mode;
					break;
				case 0:		/* Return average of all modes */
					multiplicity++;
					mid_point_sum += (0.5 * (x[i + j] + x[i]));
					break;
				case +1:	/* Always pick highest mode */
					this_mode = 0.5 * (x[i + j] + x[i]);
					if (this_mode > mid_point_sum) mid_point_sum = this_mode;
					break;
			}
		}
		else if (length < short_length) {	/* Update current best mode estimate */
			multiplicity = 1;
			mid_point_sum = (0.5 * (x[i + j] + x[i]));
			short_length = length;
		}
	}

	if (multiplicity > 1) {	/* Found more than 1 mode; return mean of them all */
		mid_point_sum /= multiplicity;
		(*n_multiples) += multiplicity;
	}

	*mode_est = mid_point_sum;
	return (0);
}

int GMT_mode_f (struct GMT_CTRL *GMT, float *x, uint64_t n, uint64_t j, bool sort, unsigned int mode_selection, unsigned int *n_multiples, double *mode_est)
{
	uint64_t i, istop;
	unsigned int multiplicity;
	double mid_point_sum = 0.0, length, short_length = FLT_MAX, this_mode;

	if (n == 0) {	/* No data, so no defined mode */
		*mode_est = GMT->session.d_NaN;
		return (0);
	}
	if (n == 1) {
		*mode_est = x[0];
		return (0);
	}
	if (sort) GMT_sort_array (GMT, x, n, GMT_FLOAT);

	istop = n - j;
	multiplicity = 0;

	for (i = 0; i < istop; i++) {
		length = x[i + j] - x[i];
		if (length < 0.0) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT_mode_f: Array not sorted in non-decreasing order.\n");
			return (-1);
		}
		else if (length == short_length) {	/* Possibly multiple mode */
			switch (mode_selection) {
				case -1:	/* Always pick lowest mode */
					this_mode = 0.5 * (x[i + j] + x[i]);
					if (this_mode < mid_point_sum) mid_point_sum = this_mode;
					break;
				case 0:		/* Return average of all modes */
					multiplicity++;
					mid_point_sum += (0.5 * (x[i + j] + x[i]));
					break;
				case +1:	/* Always pick highest mode */
					this_mode = 0.5 * (x[i + j] + x[i]);
					if (this_mode > mid_point_sum) mid_point_sum = this_mode;
					break;
			}
		}
		else if (length < short_length) {
			multiplicity = 1;
			mid_point_sum = (0.5 * (x[i + j] + x[i]));
			short_length = length;
		}
	}

	if (multiplicity > 1) {	/* Found more than 1 mode; return mean of them all */
		mid_point_sum /= multiplicity;
		(*n_multiples) += multiplicity;
	}

	*mode_est = mid_point_sum;
	return (0);
}

/* Replacement slower functions until we figure out the problem with the algorithm */

void GMT_getmad (struct GMT_CTRL *GMT, double *x, uint64_t n, double location, double *scale)
{
	uint64_t i;
	double med, *dev = NULL;

	if (n == 0) {	/* No data, so cannot define MAD */
		*scale = GMT->session.d_NaN;
		return;
	}
	dev = GMT_memory (GMT, NULL, n, double);
	for (i = 0; i < n; i++) dev[i] = fabs (x[i] - location);
	GMT_sort_array (GMT, dev, n, GMT_DOUBLE);
	for (i = n; i > 1 && GMT_is_dnan (dev[i-1]); i--);
	if (i)
		med = (i%2) ? dev[i/2] : 0.5 * (dev[(i-1)/2] + dev[i/2]);
	else
		med = GMT->session.d_NaN;
	GMT_free (GMT, dev);
	*scale = 1.4826 * med;
}

void GMT_getmad_f (struct GMT_CTRL *GMT, float *x, uint64_t n, double location, double *scale)
{
	uint64_t i;
	float *dev = NULL;
	double med;

	if (n == 0) {	/* No data, so cannot define MAD */
		*scale = GMT->session.d_NaN;
		return;
	}
	dev = GMT_memory (GMT, NULL, n, double);
	for (i = 0; i < n; i++) dev[i] = (float) fabs (x[i] - location);
	GMT_sort_array (GMT, dev, n, GMT_FLOAT);
	for (i = n; i > 1 && GMT_is_fnan (dev[i-1]); i--);
	if (i)
		med = (i%2) ? dev[i/2] : 0.5 * (dev[(i-1)/2] + dev[i/2]);
	else
		med = GMT->session.d_NaN;
	GMT_free (GMT, dev);
	*scale = 1.4826 * med;
}

double GMT_extreme (struct GMT_CTRL *GMT, double x[], uint64_t n, double x_default, int kind, int way)
{
	/* Returns the extreme value in the x array according to:
	*  kind: -1 means only consider negative values.
	*  kind:  0 means consider all values.
	*  kind: +1 means only consider positive values.
	*  way:  -1 means look for mimimum.
	*  way:  +1 means look for maximum.
	* If kind is non-zero we assign x_default is no values are found.
	* If kind == 0 we expect x_default to be set so that x[0] will reset x_select.
	*/

	uint64_t i, k;
	double x_select = GMT->session.d_NaN;

	if (n == 0) return (x_select);	/* No data, so no defined extreme value */
	for (i = k = 0; i < n; i++) {
		if (kind == -1 && x[i] > 0.0) continue;
		if (kind == +1 && x[i] < 0.0) continue;
		if (k == 0) x_select = x[i];
		if (way == -1 && x[i] < x_select) x_select = x[i];
		if (way == +1 && x[i] > x_select) x_select = x[i];
		k++;
	}

	return ((k) ? x_select : x_default);
}

int GMT_chebyshev (struct GMT_CTRL *GMT, double x, int n, double *t)
{
	/* Calculates the n'th Chebyshev polynomial at x */

	double x2, a, b;

	if (n < 0) GMT_err_pass (GMT, GMT_CHEBYSHEV_NEG_ORDER, "");
	if (fabs (x) > 1.0) GMT_err_pass (GMT, GMT_CHEBYSHEV_BAD_DOMAIN, "");

	switch (n) {	/* Testing the order of the polynomial */
		case 0:
			*t = 1.0;
			break;
		case 1:
			*t = x;
			break;
		case 2:
			*t = 2.0 * x * x - 1.0;
			break;
		case 3:
			*t = x * (4.0 * x * x - 3.0);
			break;
		case 4:
			x2 = x * x;
			*t = 8.0 * x2 * (x2 - 1.0) + 1.0;
			break;
		default:	/* For higher degrees we do the recursion */
			GMT_chebyshev (GMT, x, n-1, &a);
			GMT_chebyshev (GMT, x, n-2, &b);
			*t = 2.0 * x * a - b;
			break;
	}

	return (GMT_NOERROR);
}

double GMT_corrcoeff (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, unsigned int mode)
{
	/* Returns plain correlation coefficient, r.
	 * If mode = 1 we assume mean(x) = mean(y) = 0.
	 */

	uint64_t i, n_use;
	double xmean = 0.0, ymean = 0.0, dx, dy, vx, vy, vxy, r;

	if (n == 0) return (GMT->session.d_NaN);	/* No data, so no defined correlation */
	if (mode == 0) {
		for (i = n_use = 0; i < n; i++) {
			if (GMT_is_dnan (x[i]) || GMT_is_dnan (y[i])) continue;
			xmean += x[i];
			ymean += y[i];
			n_use++;
		}
		if (n_use == 0) return (GMT->session.d_NaN);
		xmean /= (double)n_use;
		ymean /= (double)n_use;
	}

	vx = vy = vxy = 0.0;
	for (i = n_use = 0; i < n; i++) {
		if (GMT_is_dnan (x[i]) || GMT_is_dnan (y[i])) continue;
		dx = x[i] - xmean;
		dy = y[i] - ymean;
		vx += dx * dx;
		vy += dy * dy;
		vxy += dx * dy;
	}

	r = vxy / sqrt (vx * vy);
	return (r);
}

double GMT_corrcoeff_f (struct GMT_CTRL *GMT, float *x, float *y, uint64_t n, unsigned int mode)
{
	/* Returns plain correlation coefficient, r.
	 * If mode = 1 we assume mean(x) = mean(y) = 0.
	 */

	uint64_t i, n_use;
	double xmean = 0.0, ymean = 0.0, dx, dy, vx, vy, vxy, r;

	if (n == 0) return (GMT->session.d_NaN);	/* No data, so no defined correlation */
	if (mode == 0) {
		for (i = n_use = 0; i < n; i++) {
			if (GMT_is_fnan (x[i]) || GMT_is_fnan (y[i])) continue;
			xmean += (double)x[i];
			ymean += (double)y[i];
			n_use++;
		}
		if (n_use == 0) return (GMT->session.d_NaN);
		xmean /= (double)n_use;
		ymean /= (double)n_use;
	}

	vx = vy = vxy = 0.0;
	for (i = n_use = 0; i < n; i++) {
		if (GMT_is_fnan (x[i]) || GMT_is_fnan (y[i])) continue;
		dx = (double)x[i] - xmean;
		dy = (double)y[i] - ymean;
		vx += dx * dx;
		vy += dy * dy;
		vxy += dx * dy;
	}

	r = vxy / sqrt (vx * vy);
	return (r);
}

double GMT_quantile (struct GMT_CTRL *GMT, double *x, double q, uint64_t n)
{
	/* Returns the q'th (q in percent) quantile of x (assumed sorted).
	 * q is expected to be 0 < q < 100 */

	uint64_t i_f;
	double p, f, df;

	if (n == 0) return (GMT->session.d_NaN);	/* No data, so no defined quantile */
	if (q == 0.0) return (x[0]);			/* 0% quantile == min(x) */
	while (n > 1 && GMT_is_dnan (x[n-1])) n--;	/* Skip any NaNs at the end of x */
	if (n < 1) return (GMT->session.d_NaN);		/* Need at least 1 point to do something */
	if (q == 100.0) return (x[n-1]);		/* 100% quantile == max(x) */
	f = (n - 1) * q / 100.0;
	i_f = (uint64_t)floor (f);
	if ((df = (f - (double)i_f)) > 0.0)		/* Must interpolate between the two neighbors */
		p = x[i_f+1] * df + x[i_f] * (1.0 - df);
	else						/* Exactly on a node */
		p = x[i_f];

	return (p);
}

double GMT_quantile_f (struct GMT_CTRL *GMT, float *x, double q, uint64_t n)
{
	/* Returns the q'th (q in percent) quantile of x (assumed sorted).
	 * q is expected to be 0 < q < 100 */

	uint64_t i_f;
	double p, f, df;

	if (n == 0) return (GMT->session.d_NaN);	/* No data, so no defined quantile */
	if (q == 0.0) return ((double)x[0]);		/* 0% quantile == min(x) */
	while (n > 1 && GMT_is_fnan (x[n-1])) n--;	/* Skip any NaNs at the end of x */
	if (n < 1) return (GMT->session.d_NaN);		/* Need at least 1 point to do something */
	if (q == 100.0) return ((double)x[n-1]);	/* 100% quantile == max(x) */
	f = (n - 1) * q / 100.0;
	i_f = (uint64_t)floor (f);
	if ((df = (f - (double)i_f)) > 0.0)		/* Must interpolate between the two neighbors */
		p = (double)(x[i_f+1] * df + x[i_f] * (1.0 - df));
	else						/* Exactly on a node */
		p = (double)x[i_f];

	return (p);
}

void gmt_Cmul (double A[], double B[], double C[])
{	/* Complex multiplication */
	C[GMT_RE] = A[GMT_RE]*B[GMT_RE] - A[GMT_IM]*B[GMT_IM];
	C[GMT_IM] = A[GMT_RE]*B[GMT_IM] + A[GMT_IM]*B[GMT_RE];
}

void gmt_Cdiv (double A[], double B[], double C[])
{	/* Complex division */
	double denom;
	denom = B[GMT_RE]*B[GMT_RE] + B[GMT_IM]*B[GMT_IM];
	C[GMT_RE] = (A[GMT_RE]*B[GMT_RE] + A[GMT_IM]*B[GMT_IM])/denom;
	C[GMT_IM] = (A[GMT_IM]*B[GMT_RE] - A[GMT_RE]*B[GMT_IM])/denom;
}

void gmt_Ccot (double Z[], double cotZ[])
{	/* Complex cot(z) */
	double sx, cx, e, A[2], B[2];

	sincos (2.0*Z[0], &sx, &cx);
	e = exp (-2.0*Z[1]);
	A[0] = -e * sx;		A[1] = B[0] = e * cx;
	A[1] += 1.0;	B[0] -= 1.0;	B[1] = -A[0];
	gmt_Cdiv (A, B, cotZ);
}

double gmt_Cabs (double A[])
{
	return (hypot (A[GMT_RE], A[GMT_IM]));
}

double GMT_psi (struct GMT_CTRL *P, double zz[], double p[])
{
/* Psi     Psi (or Digamma) function for complex arguments z.
*
*                 d
*        Psi(z) = --log(Gamma(z))
*                 dz
*
* zz[GMT_RE] is real and zz[GMT_IM] is imaginary component; same for p on output (only if p != NULL).
* We also return the real component as function result.
*/
	static double c[15] = { 0.99999999999999709182, 57.156235665862923517, -59.597960355475491248,
			14.136097974741747174, -0.49191381609762019978, 0.33994649984811888699e-4,
			0.46523628927048575665e-4, -0.98374475304879564677e-4, 0.15808870322491248884e-3,
			-0.21026444172410488319e-3, 0.21743961811521264320e-3, -0.16431810653676389022e-3,
			0.84418223983852743293e-4, -0.26190838401581408670e-4, 0.36899182659531622704e-5};
	double z[2], g[2], dx[2], dd[2], d[2], n[2], gg[2], f[2], x0, A[2], B[2], C[2], sx, cx, e;
	int k;

	if (zz[GMT_IM] == 0.0 && rint(zz[GMT_RE]) == zz[GMT_RE] && zz[GMT_RE] <= 0.0) {
		if (p) { p[GMT_RE] = P->session.d_NaN; p[GMT_IM] = 0.0;}
		return (P->session.d_NaN);	/* Singular points */
	}

	z[GMT_RE] = zz[GMT_RE];	z[GMT_IM] = zz[GMT_IM];
	if ((x0 = z[GMT_RE]) < 0.5) {	/* reflection point */
		z[GMT_RE] = 1.0 - z[GMT_RE];
		z[GMT_IM] = -z[GMT_IM];
	}

	/* Lanczos approximation */

	g[GMT_RE] = 607.0/128.0;	g[GMT_IM] = 0.0; /* best results when 4<=g<=5 */
	n[GMT_RE] = d[GMT_RE] = n[GMT_IM] = d[GMT_IM] = 0.0;
	for (k = 14; k > 0; k--) {
		A[GMT_RE] = 1.0;	A[GMT_IM] = 0.0;
		B[GMT_RE] = z[GMT_RE] + k - 1.0;	B[GMT_IM] = z[GMT_IM];
		gmt_Cdiv (A, B, dx);
		dd[GMT_RE] = c[k] * dx[GMT_RE];	dd[GMT_IM] = c[k] * dx[GMT_IM];
		d[GMT_RE] += dd[GMT_RE];	d[GMT_IM] += dd[GMT_IM];
		gmt_Cmul (dd, dx, B);
		n[GMT_RE] -= B[GMT_RE];	n[GMT_IM] -= B[GMT_IM];
	}
	d[GMT_RE] += c[GMT_RE];
	gg[GMT_RE] = z[GMT_RE] + g[GMT_RE] - 0.5;	gg[GMT_IM] = z[GMT_IM];
	gmt_Cdiv (n, d, A);
	gmt_Cdiv (g, gg, B);
	f[GMT_RE] = log (hypot(gg[GMT_RE], gg[GMT_IM])) + A[GMT_RE] - B[GMT_RE];
	f[GMT_IM] = atan2 (gg[GMT_IM], gg[GMT_RE])  + A[GMT_IM] - B[GMT_IM];
	if (x0 < 0.5) {
		C[GMT_RE] = M_PI * zz[GMT_RE];	C[GMT_IM] = M_PI * zz[GMT_IM];
		e = exp (-2*C[GMT_IM]);	sx = sin (2*C[GMT_RE]);	cx = cos (2*C[GMT_RE]);
		A[GMT_RE] = -e * sx;	A[GMT_IM] = e * cx + 1.0;
		B[GMT_RE] = e * cx - 1.0;	B[GMT_IM] = e * sx;
		gmt_Cdiv (A, B, C);
		f[GMT_RE] -= M_PI * C[GMT_RE];	f[GMT_IM] -= M_PI * C[GMT_IM];
	}
	if (p) {
		p[GMT_RE] = f[GMT_RE];
		p[GMT_IM] = f[GMT_IM];
	}
	return (f[GMT_RE]);
}

#ifndef M_SQRT_PI
#define M_SQRT_PI 1.772453850905516
#endif

#define PV_RE 0
#define PV_IM 1
#define QV_RE 2
#define QV_IM 3

void GMT_PvQv (struct GMT_CTRL *GMT, double x, double v_ri[], double pq[], unsigned int *iter)
{
	/* Here, -1 <= x <= +1, v_ri is an imaginary number [r,i], and we return
	 * the real amd imaginary parts of Pv(x) and Qv(x) in the pq array.
	 * Based on recipe in An Atlas of Functions */

	bool p_set, q_set;
	double M, L, K, Xn, x2, k, k1, ep, em, sx, cx, fact;
	double a[2], v[2], vp1[2], G[2], g[2], u[2], t[2], f[2];
	double R[2], r[2], z[2], s[2], c[2], w[2], tmp[2], X[2], A[2], B[2];

	*iter = 0;
	pq[PV_RE] = pq[QV_RE] = pq[PV_IM] = pq[QV_IM] = 0.0;	/* Initialize all answers to zero first */
	p_set = q_set = false;
	if (x == -1 && v_ri[GMT_IM] == 0.0) {	/* Check special values for real nu when x = -1 */
			/* Special Pv(-1) values */
		if ((v_ri[GMT_RE] > 0.0 && fmod (v_ri[GMT_RE], 2.0) == 1.0) || (v_ri[GMT_RE] < 0.0 && fmod (v_ri[GMT_RE], 2.0) == 0.0)) {	/* v = 1,3,5,.. or -2, -4, -6 */
			pq[PV_RE] = -1.0; p_set = true;
		}
		else if ((v_ri[GMT_RE] >= 0.0 && fmod (v_ri[GMT_RE], 2.0) == 0.0) || (v_ri[GMT_RE] < 0.0 && fmod (v_ri[GMT_RE]+1.0, 2.0) == 0.0)) {	/* v = 0,2,4,6 or -1,-3,-5,-7 */
			pq[PV_RE] = 1.0; p_set = true;
		}
		else if (v_ri[GMT_RE] > 0.0 && ((fact = v_ri[GMT_RE]-2.0*floor(v_ri[GMT_RE]/2.0)) < 1.0 && fact > 0.0)) { /* 0 < v < 1, 2 < v < 3, 4 < v < 5, ... */
			pq[PV_RE] = GMT->session.d_NaN; p_set = true;	/* -inf */
		}
		else if (v_ri[GMT_RE] < 1.0 && ((fact = v_ri[GMT_RE]-2.0*floor(v_ri[GMT_RE]/2.0)) < 1.0 && fact > 0.0)) {	/* -2 < v < -1, -4 < v < -3, -6 < v < -5 */
			pq[PV_RE] = GMT->session.d_NaN; p_set = true;	/* -inf */
		}
		else if (v_ri[GMT_RE] > 1.0 && (v_ri[GMT_RE]-2.0*floor(v_ri[GMT_RE]/2.0)) > 1.0) {	/* 1 < v < 2, 3 < v < 4, 5 < v < 6, .. */
			pq[PV_RE] = GMT->session.d_NaN; p_set = true;	/* +inf */
		}
		else if (v_ri[GMT_RE] < 2.0 && ( v_ri[GMT_RE]-2.0*floor(v_ri[GMT_RE]/2.0)) > 1.0) {	/* -3 < v < -2, -5 < v < -4, -7 < v < -6, .. */
			pq[PV_RE] = GMT->session.d_NaN; p_set = true;	/* +inf */
		}
		/* Special Qv(-1) values */
		if (v_ri[GMT_RE] > 0.0 && fmod (2.0 * v_ri[GMT_RE] - 1.0, 4.0) == 0.0) {	/* v = 1/2, 5/2, 9/2, ... */
			pq[QV_RE] = -M_PI_2; q_set = true;
		}
		else if (v_ri[GMT_RE] > -1.0 && fmod (2.0 * v_ri[GMT_RE] + 1.0, 4.0) == 0.0) {	/* v = -1/2, 3/2, 7/2, ... */
			pq[QV_RE] = M_PI_2; q_set = true;
		}
		else if (v_ri[GMT_RE] < -1.0 && fmod (2.0 * v_ri[GMT_RE] - 1.0, 4.0) == 0.0) {	/* v = -3/2, -7/2, -11/2, ... */
			pq[QV_RE] = -M_PI_2; q_set = true;
		}
		else if (v_ri[GMT_RE] < -2.0 && fmod (2.0 * v_ri[GMT_RE] + 1.0, 4.0) == 0.0) {	/* v = -5/2, -9/2, -13/2, ... */
			pq[QV_RE] = M_PI_2; q_set = true;
		}
		else {	/* Either -inf or +inf */
			pq[QV_RE] = GMT->session.d_NaN; q_set = true;
		}
	}
	else if (x == +1 && v_ri[GMT_IM] == 0.0) {	/* Check special values for real nu when x = +1 */
		pq[PV_RE] = 1.0; p_set = true;
		pq[QV_RE] = GMT->session.d_NaN; q_set = true;
	}
	if (p_set && q_set) return;

	/* General case of |x| < 1 */

	a[0] = a[1] = R[GMT_RE] = 1.0;	R[GMT_IM] = 0.0;
	v[GMT_RE] = v_ri[GMT_RE];	v[GMT_IM] = v_ri[GMT_IM];
	gmt_Cmul (v, v, z);
	z[GMT_RE] = v[GMT_RE] - z[GMT_RE];	z[GMT_IM] = v[GMT_IM] - z[GMT_IM];
	K = 4.0 * sqrt (gmt_Cabs(z));
	vp1[GMT_RE] = v[GMT_RE] + 1.0;	vp1[GMT_IM] = v[GMT_IM];
	if ((gmt_Cabs(vp1) + floor(vp1[GMT_RE])) == 0.0) {
		a[0] = GMT->session.d_NaN;
		a[1] = 0.0;
		v[GMT_RE] = -1 - v[GMT_RE];
		v[GMT_IM] = -v[GMT_IM];
	}
	z[GMT_RE] = 0.5 * M_PI * v[GMT_RE];	z[GMT_IM] = 0.5 * M_PI * v[GMT_IM];
	ep = exp (z[GMT_IM]);	em = exp (-z[GMT_IM]);
	sincos (z[GMT_RE], &sx, &cx);
	s[GMT_RE] = 0.5 * sx * (em + ep);
	s[GMT_IM] = -0.5 * cx * (em - ep);
	c[GMT_RE] = 0.5 * cx * (em + ep);
	c[GMT_IM] = 0.5 * sx * (em - ep);
	tmp[GMT_RE] = 0.5 + v[GMT_RE];	tmp[GMT_IM] = v[GMT_IM];
	gmt_Cmul (tmp, tmp, w);
	z[GMT_IM] = v[GMT_IM];
	while (v[GMT_RE] <= 6.0) {
		v[GMT_RE] = v[GMT_RE] + 2.0;
		z[GMT_RE] = v[GMT_RE] - 1.0;
		gmt_Cdiv (z, v, tmp);
		gmt_Cmul (R,tmp,r);
		R[GMT_RE] = r[GMT_RE];	R[GMT_IM] = r[GMT_IM];
	}
	z[GMT_RE] = v[GMT_RE] + 1.0;
	tmp[GMT_RE] = 0.25;	tmp[GMT_IM] = 0.0;
	gmt_Cdiv (tmp, z, X);
	tmp[GMT_RE] = 0.35 + 6.1 * X[GMT_RE];	tmp[GMT_IM] = 6.1*X[GMT_IM];
	gmt_Cmul (X, tmp, z);
	z[GMT_RE] = 1.0 - 3.0*z[GMT_RE];	z[GMT_IM] = -3.0*z[GMT_IM];
	gmt_Cmul (X, z, tmp);
	G[GMT_RE] = 1.0 + 5.0 * tmp[GMT_RE];	G[GMT_IM] = 5.0 * tmp[GMT_IM];
	z[GMT_RE] = 8.0 * X[GMT_RE];	z[GMT_IM] = 8.0 * X[GMT_IM];
	M = sqrt(hypot(z[GMT_RE], z[GMT_IM]));
	L = 0.5 * atan2 (z[GMT_IM], z[GMT_RE]);
	tmp[GMT_RE] = M * cos(L);	tmp[GMT_IM] = M * sin(L);
	gmt_Cmul (G, X, z);
	z[GMT_RE] = 1.0 - 0.5*z[GMT_RE];	z[GMT_IM] = -0.5*z[GMT_IM];
	gmt_Cmul (X, z, r);
	r[GMT_RE] = 1.0 - r[GMT_RE];	r[GMT_IM] = -r[GMT_IM];
	gmt_Cmul (R, r, z);
	gmt_Cdiv (z, tmp, R);
	u[GMT_RE] = g[GMT_RE] = 2.0 * x;	u[GMT_IM] = g[GMT_IM] = f[GMT_IM] = t[GMT_IM] = 0.0;
	f[GMT_RE] = t[GMT_RE] = 1.0;
	k = 0.5;
	x2 = x * x;
	Xn = 1.0 + (1e8/(1 - x2));
	k1 = k + 1.0;
	fact = x2 / (k1*k1 - 0.25);
	t[GMT_RE] = (k*k - w[GMT_RE]) * fact;	t[GMT_IM] = -w[GMT_IM] * fact;
	k += 1.0;
	f[GMT_RE] += t[GMT_RE];	f[GMT_IM] += t[GMT_IM];
	k1 = k + 1.0;
	fact = u[GMT_RE] * x2 / (k1*k1 - 0.25);
	u[GMT_RE] = (k*k - w[GMT_RE]) * fact;	u[GMT_IM] = -w[GMT_IM] * fact;
	k += 1;
	g[GMT_RE] += u[GMT_RE];	g[GMT_IM] += u[GMT_IM];
	tmp[GMT_RE] = Xn * t[GMT_RE];	tmp[GMT_IM] = Xn * t[GMT_IM];
	while (k < K || gmt_Cabs (tmp) > gmt_Cabs(f)) {
		(*iter)++;
		k1 = k + 1.0;
		tmp[GMT_RE] = k*k - w[GMT_RE];	tmp[GMT_IM] = -w[GMT_IM];	fact = x2 / (k1*k1 - 0.25);
		gmt_Cmul (t, tmp, A);
		t[GMT_RE] = A[GMT_RE] * fact;	t[GMT_IM] = A[GMT_IM] * fact;
		k += 1.0;
		k1 = k + 1.0;
		f[GMT_RE] += t[GMT_RE];	f[GMT_IM] += t[GMT_IM];
		tmp[GMT_RE] = k*k - w[GMT_RE];	tmp[GMT_IM] = -w[GMT_IM];	fact = x2 / (k1*k1 - 0.25);
		gmt_Cmul (u, tmp, B);
		u[GMT_RE] = B[GMT_RE] * fact;	u[GMT_IM] = B[GMT_IM] * fact;
		k += 1.0;
		g[GMT_RE] += u[GMT_RE];	g[GMT_IM] += u[GMT_IM];
		tmp[GMT_RE] = Xn * t[GMT_RE];	tmp[GMT_IM] = Xn * t[GMT_IM];
	}
	fact = x2 / (1.0 - x2);
	f[GMT_RE] += t[GMT_RE] * fact;	f[GMT_IM] += t[GMT_IM] * fact;
	g[GMT_RE] += u[GMT_RE] * fact;	g[GMT_IM] += u[GMT_IM] * fact;
	if (!p_set) {
		gmt_Cmul(s,R,z);
		gmt_Cdiv(c,R,tmp);
		gmt_Cmul (g, z, A);	gmt_Cmul (f, tmp, B);
		pq[PV_RE] = (A[GMT_RE] + B[GMT_RE])/M_SQRT_PI;
		pq[PV_IM] = (A[GMT_IM] + B[GMT_IM])/M_SQRT_PI;
	}
	if (!q_set) {
		gmt_Cmul(c,R,z);
		gmt_Cdiv(s,R,tmp);
		gmt_Cmul (g, z, A);	gmt_Cmul (f, tmp, B);
		pq[QV_RE] = a[0]*M_SQRT_PI*(A[GMT_RE] - B[GMT_RE])/2.0;
		pq[QV_IM] = a[1]*M_SQRT_PI*(A[GMT_IM] - B[GMT_IM])/2.0;
	}
}
