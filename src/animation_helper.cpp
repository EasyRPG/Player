/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

#include "animation_helper.h"
#include <cmath>
#include <vector>
#include <sstream>


// references for cubic bezier:
// https://matthewlein.com/tools/ceaser
// https://cubic-bezier.com/
double Animation_Helper::CubicBezier(float t, const double& p0, const double& p1, const double& p2, const double& p3) {

	float u = 1 - t;
	float tt = t * t;
	float uu = u * u;
	float uuu = uu * u;
	float ttt = tt * t;

	//Point2d p = {0,0};
	//p.x = uuu * 0 + 3 * uu * t * p0 + 3 * u * tt * p2 + ttt * 1;
	return uuu * 0 + 3 * uu * t * p1 + 3 * u * tt * p3 + ttt * 1;

	//return p.y;
}

double Animation_Helper::GetEasedTime(const std::string& easing_type, double t, double b, double c, double d) {
	if (easing_type == "linear")  return Animation_Helper::CubicBezier(t, 0.250, 0.250, 0.750, 0.750);

	else if (easing_type == "ease")  return Animation_Helper::CubicBezier(t, 0.250, 0.100, 0.250, 1.000);
	else if (easing_type == "easeIn")  return Animation_Helper::CubicBezier(t, 0.420, 0.000, 1.000, 1.000);
	else if (easing_type == "easeOut")  return Animation_Helper::CubicBezier(t, 0.000, 0.000, 0.580, 1.000);
	else if (easing_type == "easeInOut")  return Animation_Helper::CubicBezier(t, 0.420, 0.000, 0.580, 1.000);

	else if (easing_type == "quadIn")  return Animation_Helper::CubicBezier(t, 0.550, 0.085, 0.680, 0.530);
	else if (easing_type == "quadOut")  return Animation_Helper::CubicBezier(t, 0.250, 0.460, 0.450, 0.940);
	else if (easing_type == "quadInOut")  return Animation_Helper::CubicBezier(t, 0.455, 0.030, 0.515, 0.955);

	else if (easing_type == "cubicIn")  return Animation_Helper::CubicBezier(t, 0.550, 0.055, 0.675, 0.190);
	else if (easing_type == "cubicOut")  return Animation_Helper::CubicBezier(t, 0.215, 0.610, 0.355, 1.000);
	else if (easing_type == "cubicInOut")  return Animation_Helper::CubicBezier(t, 0.645, 0.045, 0.355, 1.000);

	else if (easing_type == "quartIn")  return Animation_Helper::CubicBezier(t, 0.895, 0.030, 0.685, 0.220);
	else if (easing_type == "quartOut")  return Animation_Helper::CubicBezier(t, 0.165, 0.840, 0.440, 1.000);
	else if (easing_type == "quartInOut")  return Animation_Helper::CubicBezier(t, 0.770, 0.000, 0.175, 1.000);

	else if (easing_type == "quintIn")  return Animation_Helper::CubicBezier(t, 0.755, 0.050, 0.855, 0.060);
	else if (easing_type == "quintOut")  return Animation_Helper::CubicBezier(t, 0.230, 1.000, 0.320, 1.000);
	else if (easing_type == "quintInOut")  return Animation_Helper::CubicBezier(t, 0.860, 0.000, 0.070, 1.000);

	else if (easing_type == "sineIn")  return Animation_Helper::CubicBezier(t, 0.470, 0.000, 0.745, 0.715);
	else if (easing_type == "sineOut")  return Animation_Helper::CubicBezier(t, 0.390, 0.575, 0.565, 1.000);
	else if (easing_type == "sineInOut")  return Animation_Helper::CubicBezier(t, 0.445, 0.050, 0.550, 0.950);

	else if (easing_type == "ExpoIn")  return Animation_Helper::CubicBezier(t, 0.950, 0.050, 0.795, 0.035);
	else if (easing_type == "expoOut")  return Animation_Helper::CubicBezier(t, 0.190, 1.000, 0.220, 1.000);
	else if (easing_type == "expoInOut")  return Animation_Helper::CubicBezier(t, 1.000, 0.000, 0.000, 1.000);

	else if (easing_type == "circIn")  return Animation_Helper::CubicBezier(t, 0.600, 0.040, 0.980, 0.335);
	else if (easing_type == "circOut")  return Animation_Helper::CubicBezier(t, 0.075, 0.820, 0.165, 1.000);
	else if (easing_type == "circInOut")  return Animation_Helper::CubicBezier(t, 0.785, 0.135, 0.150, 0.860);

	else if (easing_type == "backIn")  return Animation_Helper::CubicBezier(t, 0.600, -0.280, 0.735, 0.045);
	else if (easing_type == "backOut")  return Animation_Helper::CubicBezier(t, 0.175, 0.885, 0.320, 1.275);
	else if (easing_type == "backInOut")  return Animation_Helper::CubicBezier(t, 0.680, -0.550, 0.265, 1.550);

	else if (easing_type == "elasticIn") {
		if (t == 0) {
			return b;
		}
		if ((t /= d) == 1) {
			return b + c;
		}

		double p = d * 0.3;
		double a = c;
		double s = p / 4;

		double post_increment_fix = a * pow(2, 10 * (t -= 1));
		return -(post_increment_fix * sin((t * d - s) * (2 * M_PI) / p)) + b;
	}
	else if (easing_type == "elasticOut") {
		if (t == 0) {
			return b;
		}
		if ((t /= d) == 1) {
			return b + c;
		}

		double p = d * 0.3;
		double a = c;
		double s = p / 4;

		return (a * pow(2, -10 * t) * sin((t * d - s) * (2 * M_PI) / p) + c + b);
	}
	else if (easing_type == "elasticInOut") {
		if (t == 0) {
			return b;
		}
		if ((t /= d / 2) == 2) {
			return b + c;
		}

		double p = d * (0.3 * 1.5);
		double a = c;
		double s = p / 4;

		if (t < 1) {
			double post_increment_fix = a * pow(2, 10 * (t -= 1));
			return -0.5 * (post_increment_fix * sin((t * d - s) * (2 * M_PI) / p)) + b;
		}

		double post_increment_fix = a * pow(2, -10 * (t -= 1));
		return post_increment_fix * sin((t * d - s) * (2 * M_PI) / p) * 0.5 + c + b;
	}

	else if (easing_type == "bounceIn") {
		return c - Animation_Helper::GetEasedTime("bounceOut", d - t, 0, c, d) + b;
	}
	else if (easing_type == "bounceOut") {
		if ((t /= d) < (1 / 2.75)) {
			return c * (7.5625 * t * t) + b;
		}
		else if (t < (2 / 2.75)) {
			t -= (1.5 / 2.75);
			return c * (7.5625 * t * t + 0.75) + b;
		}
		else if (t < (2.5 / 2.75)) {
			t -= (2.25 / 2.75);
			return c * (7.5625 * t * t + 0.9375) + b;
		}
		else {
			t -= (2.625 / 2.75);
			return c * (7.5625 * t * t + 0.984375) + b;
		}
	}
	else if (easing_type == "bounceInOut") {
		if (t < d / 2) {
			return Animation_Helper::GetEasedTime("bounceIn", t * 2, 0, c, d) * 0.5 + b;
		}
		else {
			return Animation_Helper::GetEasedTime("bounceOut", t * 2 - d, 0, c, d) * 0.5 + c * 0.5 + b;
		}
	}

	if (easing_type.substr(0, 6) == "bezier") {
		std::vector<double> bezier_params;

		size_t start_pos = easing_type.find("(") + 1;
		size_t end_pos = easing_type.find(")");
		std::string value_string = easing_type.substr(start_pos, end_pos - start_pos);

		std::istringstream iss(value_string);
		double value;

		while (iss >> value) bezier_params.push_back(value), iss.ignore();

		if (bezier_params.size() == 4)
			return Animation_Helper::CubicBezier(t, bezier_params[0], bezier_params[1], bezier_params[2], bezier_params[3]);
	}

	return c * t / d + b; // Default to linear easing if the easing type is not recognized
}
