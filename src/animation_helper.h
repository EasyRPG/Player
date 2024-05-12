#ifndef ANIMATION_HELPER_H
#define ANIMATION_HELPER_H

#include <string>

namespace Animation_Helper {
	double CubicBezier(float t, const double& p0, const double& p1, const double& p2, const double& p3);
	double GetEasedTime(const std::string& easing_type, double t, double b, double c, double d);
}

#endif
