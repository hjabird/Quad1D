#pragma once
/*////////////////////////////////////////////////////////////////////////////
Integrators.h

Methods to perform integration of a function in one dimension.

Copyright 2017 HJA Bird

Permission is hereby granted, free of charge, to any person obtaining a copy 
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights 
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
copies of the Software, and to permit persons to whom the Software is 
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.
*/////////////////////////////////////////////////////////////////////////////


#include <cassert>
#include <cmath>
#include <type_traits>
#include <stack>

#include "Tolerances.h"

namespace HBTK {

	// DECLARATIONS

	template< typename Tf, typename Tp, typename Tw>
	auto static_integrate(Tf & func, Tp & points, Tw & weights, int n_points)
		 -> decltype(func(points[0]) * weights[0]);

	template<int n_points, typename Tf, typename Tp, typename Tw>
	auto static_integrate(Tf & func, Tp & points, Tw & weights)
		 -> decltype(func(points[0]) * weights[0]) ;

	template<typename Tf_in, typename Tf, typename Ttol>
	auto adaptive_trapezoidal_integrate(Tf & func, Ttol tolerance, 
				Tf_in lower_limit, Tf_in upper_limit)
		->decltype(func(lower_limit));

	template<typename Tf_in, typename Tf, typename Ttol>
	auto adaptive_simpsons_integrate(Tf & func, Ttol tolerance, 
		Tf_in lower_limit, Tf_in upper_limit)
		->decltype(func(lower_limit));

	template<typename Tf_in, typename Tf, typename Ttol>
	auto adaptive_gauss_lobatto_integrate(Tf & func, Ttol tolerance,
		Tf_in lower_limit, Tf_in upper_limit)
		->decltype(func(lower_limit));

	// DEFINITIONS

	/// \param func a function/lambda which accepts the value in points as its 
	/// only argument.
	/// \param points quadrature points. Indexable.
	/// \param weights quadrature weights. Indexable.
	/// \param n_ponts number of quadrature points.
	/// \returns the same type as returned by func.
	///
	/// \brief Integrate a function with a given quadrature.
	/// 
	/// Evaluates
	/// \f[ I = \sum\limits_{i=0}^{\texttt{n_points}} \texttt{func}(\texttt{points}_i) \times \texttt{weights}_i \f]
	///
	/// For example, using a six node gauss_legendre quadrature (overkill) - so 
	/// on interval [-1, 1] on a lambda my_fun:
	/// \code
	/// #include "HBTK/Integrators.h"
	/// const int num_points = 6;
	/// std::array<double, num_points> points, weights;
	/// HBTK::gauss_legendre<num_points, double>(points, weights);
	///
	/// auto my_fun = [](double x)->double
	/// {
	/// 	return x*x*x + x*x + 3;
	/// };
	///
	/// double result;
	/// result = HBTK::static_integrate(my_fun, points, weights, num_points);
	/// \endcode
	template< typename Tf, typename Tp, typename Tw>
	auto static_integrate(Tf & func, Tp & points, Tw & weights, int n_points)
		 -> decltype(func(points[0]) * weights[0]) 
	{
		assert(n_points > 0);
		int idx = 0;

		auto accumulator = func(points[idx]) * weights[idx];
		for (idx = 1; idx < n_points; idx++)
		{
			accumulator = accumulator + func(points[idx]) * weights[idx];
		}
		return accumulator;
	}



	/// \param func a function/lambda which accepts the value in points as its 
	/// only argument.
	/// \param points quadrature points. Indexable.
	/// \param weights quadrature weights. Indexable.
	/// \param n_points number of quadrature points.
	/// \returns the same type as returned by func.
	///
	/// \brief A specialised integrator templated with set number of points.
	/// 
	/// By specifying the number of integration points at compile time, it may be
	/// possible for the compiler to unroll the loop. It may or may not be
	/// different to normal...
	///
	/// It can be used as
	/// \code
	/// #include "HBTK/Integrators.h"
	/// const int num_points = 6;
	/// std::array<double, num_points> points, weights;
	/// HBTK::gauss_legendre<num_points, double>(points, weights);
	/// 
	/// auto my_fun = [](double x)->double
	/// {
	/// 	return x*x*x + x*x + 3;
	/// };
	/// 
	/// double result;
	/// result = HBTK::static_integrate<num_points>(my_fun, points, weights);
	/// \endcode
	template<int n_points, typename Tf, typename Tp, typename Tw>
	auto static_integrate(Tf & func, Tp & points, Tw & weights)
		 -> decltype(func(points[0]) * weights[0]) 
	{
		return static_integrate(func, points, weights, n_points);
	}


	/// \param func a function that takes a single argument of type Tf_in (ie. 
	/// that of lower and upper limit) and returns a floating point type.
	/// \param tolerance a floating point relative tolerance.
	/// \param lower_limit the lower limit of integration.
	/// \param upper_limit the upper limit of integration.
	///
	/// \brief Evaluate an integral using an adaptive trapezium rule method.
	template<typename Tf_in, typename Tf, typename Ttol>
	auto adaptive_trapezoidal_integrate(Tf & func, Ttol tolerance, 
				Tf_in lower_limit, Tf_in upper_limit)
		->decltype(func(lower_limit))
	{
		assert(tolerance > 0.0);
		assert(upper_limit > lower_limit);

		using R_Type = typename std::result_of<Tf(Tf_in)>::type;
		R_Type result = 0;
		R_Type coarse, fine;
		R_Type v_sub;
		Tf_in p_sub;

		int stack_size = 1;

		auto trap = [&](Tf_in x0, Tf_in x1, R_Type f0, R_Type f1)->R_Type {
			return (x1 - x0)*(f0 + f1) / 2.0;
		};

		typedef struct stack_frame {
			Tf_in l_lim, u_lim;
			R_Type l, u;
		} stack_frame;

		stack_frame tmp;

		std::stack<stack_frame> stack;

		stack.emplace(stack_frame{ lower_limit, upper_limit, 
									func(lower_limit), func(upper_limit) });

		while (!stack.empty())
		{
			tmp = stack.top();
			assert(tmp.u_lim != tmp.l_lim);
			p_sub = (tmp.l_lim + tmp.u_lim)*0.5;
			v_sub = func(p_sub);

			coarse = trap(tmp.l_lim, tmp.u_lim, tmp.l, tmp.u);
			fine = trap(tmp.l_lim, p_sub, tmp.l, v_sub)
				+ trap(p_sub, tmp.u_lim, v_sub, tmp.u);

            if (std::abs(fine - coarse) > (tmp.u_lim - tmp.l_lim) * tolerance)
			{
				stack.pop();
				stack.emplace(stack_frame{ tmp.l_lim, p_sub, tmp.l, v_sub });
				stack.emplace(stack_frame{ p_sub, tmp.u_lim, v_sub, tmp.u, });
				stack_size += 1;
			}
			else
			{
				stack.pop();
				stack_size--;
				result += fine;
			}
		}
		assert(stack_size == 0);
		return result;
	}


	/// \param func a function that takes a single argument of type Tf_in (ie. 
	/// that of lower and upper limit) and returns a floating point type.
	/// \param tolerance a floating point relative tolerance.
	/// \param lower_limit the lower limit of integration.
	/// \param upper_limit the upper limit of integration.
	///
	/// \brief Evaluate an integral using an adaptive Simpson's rule method.
	///
	/// \f[I = \int^\texttt{upper_limit}_\texttt{lower_limit} \texttt{func}(x)  dx = \texttt{adaptive_simpsons_integrate}(\texttt{func}, \texttt{tol}, \texttt{lower_limit},\texttt{upper_limit}) + \epsilon \f]
	/// where \f$|\epsilon| < \texttt{tol}\f$.
	///
	/// The code could be used as follows:
	/// \code
	/// #include "HBTK/Integrators.h"
	/// auto my_f = [](double x)->double
	/// { x*x*x*x*x*x*x*x*x; };
	/// auto result = HBTK::adaptive_simpsons_integrate(my_f, 1e-10, 0.0, 1.0);
	/// \endcode
	///	Uses a simple adaptive composite simpson's rule to evaluated to a given 
	/// tolerance.	
	template<typename Tf_in, typename Tf, typename Ttol>
	auto adaptive_simpsons_integrate(Tf & func, Ttol tolerance, 
		Tf_in lower_limit, Tf_in upper_limit)
		->decltype(func(lower_limit))
	{
		assert(tolerance > 0.0);
		assert(lower_limit < upper_limit);

		using R_Type = typename std::result_of<Tf(Tf_in)>::type;
		R_Type result = 0;
		R_Type coarse, fine;
		// 1/4 and 3/4 points coordinates and values:
		R_Type v_sub_l, v_sub_u;
		Tf_in p_sub_l, p_sub_u, p_cent;
		auto simp = [&](Tf_in x0, Tf_in x2, R_Type f0, R_Type f1, R_Type f2)->R_Type 
		{
			return (x2 - x0)*(f0 + 4.0 * f1 + f2) / 6.0;
		};

		typedef struct stack_frame {
			Tf_in l_lim, u_lim;	// l(ower)_lim(it), u(pper)_lim(it)
			R_Type l, u, c;		// l(ower), u(pper), c(entre)
		} stack_frame;

		stack_frame tmp;
		int stack_size = 1;
		std::stack<stack_frame> stack;

		stack.emplace(stack_frame{ lower_limit, upper_limit, func(lower_limit), 
			func(upper_limit), func((upper_limit - lower_limit) / 2.0) });

		R_Type is = (upper_limit - lower_limit) / 8 * (stack.top().l + stack.top().u + stack.top().c
			+ (func(lower_limit + 0.9501) + func(lower_limit + 0.2311) + func(lower_limit + 0.6068)
				+ func(lower_limit + 0.4860) + func(lower_limit + 0.8913)) * (upper_limit - lower_limit));
		is = (std::abs(is) == 0 ? upper_limit - lower_limit : is);
		is = is * tolerance / HBTK::tolerance<R_Type>();

		while (!stack.empty())
		{
			tmp = stack.top();
			p_cent = (tmp.l_lim + tmp.u_lim) * 0.5;
			p_sub_l = tmp.l_lim + (tmp.u_lim - tmp.l_lim) * 0.25;
			p_sub_u = tmp.l_lim + (tmp.u_lim - tmp.l_lim) * 0.75;
			v_sub_l = func(p_sub_l);
			v_sub_u = func(p_sub_u);

			coarse = simp(tmp.l_lim, tmp.u_lim, tmp.l, tmp.c, tmp.u);
			fine = simp(tmp.l_lim, p_cent, tmp.l, v_sub_l, tmp.c)
				+ simp(p_cent, tmp.u_lim, tmp.c, v_sub_u, tmp.u);
			coarse = (16.0 * fine - coarse) / 15.0;

			if ((is + (coarse - fine) != is) || (p_cent <= tmp.l_lim) || (p_cent >= tmp.u_lim))
			{
				stack.pop();
				stack.emplace(stack_frame{ tmp.l_lim, p_cent, tmp.l, tmp.c, v_sub_l });
				stack.emplace(stack_frame{ p_cent, tmp.u_lim, tmp.c, tmp.u, v_sub_u });
				stack_size += 1;
			}
			else
			{
				stack.pop();
				stack_size--;
				result += coarse;
			}
		}
		assert(stack_size == 0);
		return result;
	}


	/// \param func a function that takes a single argument of type Tf_in (ie. 
	/// that of lower and upper limit) and returns a floating point type.
	/// \param tolerance a floating point relative tolerance.
	/// \param lower_limit the lower limit of integration.
	/// \param upper_limit the upper limit of integration.
	///
	/// \brief Evaluate an integral using an adaptive Gauss-Lobatto method.
	///
	/// The code could be used as follows:
	/// \code
	/// #include "HBTK/Integrators.h"
	/// auto my_f = [](double x)->double
	/// { x*x*x*x*x*x*x*x*x; };
	/// auto result = HBTK::adaptive_gauss_lobatto_integrate(my_f, 1e-10, 0.0, 1.0);
	/// \endcode
	///	Uses an adaptive Gauss-Lobatto quadrature to integrate my func over
	/// given range. Based on Gander and Gautschi, BIT Numer. Math. 2000
	template<typename Tf_in, typename Tf, typename Ttol>
	auto adaptive_gauss_lobatto_integrate(Tf & func, Ttol tolerance,
		Tf_in lower_limit, Tf_in upper_limit)
		->decltype(func(lower_limit))
	{
		// Gander, W. & Gautschi, W. BIT Numerical Mathematics (2000) 40: 84. 
		// https://doi.org/10.1023/A:1022318402393
		assert(tolerance > 0.0);
		assert(lower_limit < upper_limit);

		using R_Type = typename std::result_of<Tf(Tf_in)>::type;
		R_Type result = 0;
		R_Type coarse, fine;

		Tf_in alpha = (Tf_in)sqrt(2. / 3.);
		Tf_in beta = (Tf_in)(1. / sqrt(5.));
		Tf_in x1 = (Tf_in) 0.942882415695480; 
		Tf_in x2 = (Tf_in) 0.641853342345781;
		Tf_in x3 = (Tf_in) 0.236383199662150;
		Tf_in h0 = (upper_limit - lower_limit) / 2;
		Tf_in m0 = (upper_limit + lower_limit) / 2;
		std::array<R_Type, 13> y0 = { func(lower_limit),
			func(m0 - x1 * h0), func(m0 - alpha * h0), func(m0 - x2 * h0),
			func(m0 - beta * h0), func(m0 - x3 * h0), func(m0), func(m0 + x3 * h0),
			func(m0 + beta * h0), func(m0 + x2 * h0), func(m0 + alpha * h0),
			func(m0 + x1 * h0), func(upper_limit) };

		R_Type fa(y0[0]), fb(y0[12]);
		coarse = (h0 / 6) * (y0[0] + y0[12] + 5 * (y0[4] + y0[8]));
		fine = (h0 / 1470)*(77 * (y0[0] + y0[12]) + 432 * (y0[2] + y0[10]) + 
			625 * (y0[4] + y0[8]) + 672 * y0[6]);
		R_Type is = h0 * (0.0158271919734802*(y0[0] + y0[12]) + 0.0942738402188500
			*(y0[1] + y0[11]) + 0.155071987336585*(y0[2] + y0[10]) + 
			0.188821573960182*(y0[3] + y0[9]) + 0.199773405226859
			*(y0[4] + y0[8]) + 0.224926465333340*(y0[5] + y0[7])
			+ 0.242611071901408*y0[6]);
		int s = (is >= 0.0 * R_Type() ? 1 : -1);
        R_Type err_fine = std::abs(fine - is);
        R_Type err_coarse = std::abs(coarse - is);
		R_Type R = (R_Type)1.0;
		if (err_coarse != 0.0 * R_Type()) R = err_fine / err_coarse;
		if ((R > 0) && (R < 1)) tolerance = tolerance / R;
        is = s * std::abs(is) * tolerance / HBTK::tolerance<R_Type>();
		if (is == 0) is = upper_limit - lower_limit;

		// And now onto the adaptive bit - adaptlobstp(...)

		typedef struct stack_frame {
			Tf_in l_lim, u_lim;	// l(ower)_lim(it), u(pper)_lim(it)
			R_Type l, u;		// l(ower), u(pper)
		} stack_frame;

		stack_frame tmp;
		int stack_size = 1;
		std::stack<stack_frame> stack;

		stack.emplace(stack_frame{ lower_limit, upper_limit, y0[0], y0[12] });

		while (!stack.empty())
		{
			tmp = stack.top();
			Tf_in ml, mll, m, mr, mrr, h;
			h = (tmp.u_lim - tmp.l_lim) / 2;
			m = (tmp.u_lim + tmp.l_lim) / 2;
			mll = m - alpha * h;
			mrr = m + alpha * h;
			ml = m - beta * h;
			mr = m + beta * h;
			R_Type fml, fmll, fm, fmr, fmrr;
			fml = func(ml);
			fmll = func(mll);
			fm = func(m);
			fmr = func(mr);
			fmrr = func(mrr);
			coarse = (h / 6.) * (tmp.l + tmp.u + 5.0 * (fml + fmr));
			fine = (h / 1470.) * (77 * (tmp.l + tmp.u) + 432 * (fmll + fmrr) 
				+ 625 * (fml + fmr) + 672 * fm);
			if ((is + (fine - coarse) != is) || (mll <= tmp.l_lim) || (mrr >= tmp.u_lim))
			{
				stack.pop();
				stack.emplace(stack_frame{ tmp.l_lim, mll, tmp.l, fmll });
				stack.emplace(stack_frame{ mll, ml, fmll, fml});
				stack.emplace(stack_frame{ ml, m, fml, fm });
				stack.emplace(stack_frame{ m, mr, fm, fmr });
				stack.emplace(stack_frame{ mr, mrr, fmr, fmrr });
				stack.emplace(stack_frame{ mrr, tmp.u_lim, fmrr, tmp.u });
				stack_size += 5;
			}
			else
			{
				stack.pop();
				stack_size -= 1;
				result += fine;
			}
		}
		assert(stack_size == 0);
		return result;
	}

}
