// RemapTests_demo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../../../HBTK/GaussLegendre.h"
#include "../../../HBTK/Integrators.h"
#include "../../../HBTK/Remaps.h"
#include "../../../HBTK/GnuPlot.h"

#include <iostream>
#include <functional>
#include <cassert>
#include <cmath>


void add_to_plot(std::function<double(double)> func, std::function<void(double&, double&)> remapper,
	HBTK::GnuPlot & plt, double true_sol, std::string l_spec)
{
	assert(true_sol != 0);
	int pts = 39;
	std::vector<double> points, weights, Xs, Ys;
	points.resize(pts);
	weights.resize(pts);
	Xs.resize(pts);
	Ys.resize(pts);

	for (auto idx = 0; idx < pts; idx++) {
		Xs[idx] = 1 + idx;
		HBTK::gauss_legendre<double, std::vector<double>>( idx + 1, points, weights);
		for (auto idx2 = 0; idx2 < idx + 1; idx2++) {
			remapper(points[idx2], weights[idx2]);
		}

		Ys[idx] = abs((HBTK::static_integrate(func, points, weights, idx + 1) - true_sol) / true_sol);
	}

	plt.hold_on();
	plt.plot(Xs, Ys, l_spec);
	return;
}


void singular_plot_1(std::function<double(double)> func, double true_int, std::string name) {

	auto no_op = [](double & p, double & w) { return; };
	auto t2 = [](double & p, double & w) { HBTK::telles_quadratic_remap(p, w, -1.); };
	auto t3 = [](double & p, double & w) { HBTK::telles_cubic_remap(p, w, -1.); };
	auto s3 = [](double & p, double & w) { HBTK::sato_remap<3>(p, w, -1.); };
	auto s4 = [](double & p, double & w) { HBTK::sato_remap<4>(p, w, -1.); };
	auto s5 = [](double & p, double & w) { HBTK::sato_remap<5>(p, w, -1.); };
	auto s6 = [](double & p, double & w) { HBTK::sato_remap<6>(p, w, -1.); };

	HBTK::GnuPlot plt;
	plt.hold_on();
	plt.replot_off();
	add_to_plot(func, no_op, plt, true_int, "r-+");
	add_to_plot(func, t2, plt, true_int, "b-o");
	add_to_plot(func, t3, plt, true_int, "g-d");
	add_to_plot(func, s3, plt, true_int, "y-v");
	add_to_plot(func, s4, plt, true_int, "c->");
	add_to_plot(func, s5, plt, true_int, "k-<");
	add_to_plot(func, s6, plt, true_int, "m-x");
	plt.title("Effect of integral remaps on " + name);
	plt.ylabel("Rel error");
	plt.xlabel("Gauss points");
	plt.logscale("yx");
	plt.legend({ "No remap", "Telles2", "Telles3", "Sato3", "Sato4", "Sato5", "Sato6" });
	plt.replot();
	
	system("pause");
	return;
}


void inf_limit_plot_1(std::function<double(double)> func, double true_int, std::string name)
{
	auto exp = [](double & p, double & w) { HBTK::exponential_remap(p, w, 1.); };

	HBTK::GnuPlot plt;
	plt.logscale("xy");
	plt.title("Error of integral remap on " + name);
	plt.xlabel("Number of GL points");
	plt.ylabel("Relative error");
	add_to_plot(func, exp, plt, true_int, "r-");
	system("pause");
	return;
}



int main()
{
	std::cout << "Remap tests demo\n";

	// -1, 1
	auto logf = [](double x) { return log(x + 1); };
	auto logk10f = [](double x) { return log(10 * (x + 1)); };
	auto rsqrtf = [](double x) { return 1. / sqrt(1 + x); };
	auto logv = -0.6137056388801094;
	auto logk10v = 3.991464547107982;
	auto rsqrtv = 2.828427124746191;

	singular_plot_1(logf, logv, "log(x+1)");
	singular_plot_1(logk10f, logk10v, "log(10x+10)");
	singular_plot_1(rsqrtf, rsqrtv, "1 / sqrt(1+x)");

	// 1 to infty
	auto sincf = [](double x) { return sin(x-1) / (x-1); };
	auto exppolyf = [](double x) { return exp(-x)*((sqrt(x*x - 1) - x) / x); };
	auto sincv = 1.570796326794897;
	auto exppolyv = -0.09425868914448465;

	inf_limit_plot_1(sincf, sincv, "sin(x) / x");
	inf_limit_plot_1(exppolyf, exppolyv, "Sclavounos 3.21 term 1 on RHS");

    return 0;
}

