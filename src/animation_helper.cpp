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
#include <unordered_map>
#include <algorithm>
#include <sstream>
#include <vector>

namespace {
    constexpr double PI = 3.14159265358979323846;

    // Forward declarations
    double HandleElasticEasing(Animation_Helper::EaseType type, double t, double b, double c, double d);
    double HandleBounceEasing(Animation_Helper::EaseType type, double t, double b, double c, double d);

    struct EasingData {
        std::string_view name;
        double x1, y1, x2, y2;
        Animation_Helper::EaseType type;
    };

    const EasingData EASING_DATA[] = {
        {"linear", 0.250, 0.250, 0.750, 0.750, Animation_Helper::EaseType::Linear},
        {"ease", 0.250, 0.100, 0.250, 1.000, Animation_Helper::EaseType::Ease},
        {"easeIn", 0.420, 0.000, 1.000, 1.000, Animation_Helper::EaseType::EaseIn},
        {"easeOut", 0.000, 0.000, 0.580, 1.000, Animation_Helper::EaseType::EaseOut},
        {"easeInOut", 0.420, 0.000, 0.580, 1.000, Animation_Helper::EaseType::EaseInOut},
        {"quadIn", 0.550, 0.085, 0.680, 0.530, Animation_Helper::EaseType::QuadIn},
        {"quadOut", 0.250, 0.460, 0.450, 0.940, Animation_Helper::EaseType::QuadOut},
        {"quadInOut", 0.455, 0.030, 0.515, 0.955, Animation_Helper::EaseType::QuadInOut},
        {"cubicIn", 0.550, 0.055, 0.675, 0.190, Animation_Helper::EaseType::CubicIn},
        {"cubicOut", 0.215, 0.610, 0.355, 1.000, Animation_Helper::EaseType::CubicOut},
        {"cubicInOut", 0.645, 0.045, 0.355, 1.000, Animation_Helper::EaseType::CubicInOut},
        {"quartIn", 0.895, 0.030, 0.685, 0.220, Animation_Helper::EaseType::QuartIn},
        {"quartOut", 0.165, 0.840, 0.440, 1.000, Animation_Helper::EaseType::QuartOut},
        {"quartInOut", 0.770, 0.000, 0.175, 1.000, Animation_Helper::EaseType::QuartInOut},
        {"quintIn", 0.755, 0.050, 0.855, 0.060, Animation_Helper::EaseType::QuintIn},
        {"quintOut", 0.230, 1.000, 0.320, 1.000, Animation_Helper::EaseType::QuintOut},
        {"quintInOut", 0.860, 0.000, 0.070, 1.000, Animation_Helper::EaseType::QuintInOut},
        {"sineIn", 0.470, 0.000, 0.745, 0.715, Animation_Helper::EaseType::SineIn},
        {"sineOut", 0.390, 0.575, 0.565, 1.000, Animation_Helper::EaseType::SineOut},
        {"sineInOut", 0.445, 0.050, 0.550, 0.950, Animation_Helper::EaseType::SineInOut},
        {"expoIn", 0.950, 0.050, 0.795, 0.035, Animation_Helper::EaseType::ExpoIn},
        {"expoOut", 0.190, 1.000, 0.220, 1.000, Animation_Helper::EaseType::ExpoOut},
        {"expoInOut", 1.000, 0.000, 0.000, 1.000, Animation_Helper::EaseType::ExpoInOut},
        {"circIn", 0.600, 0.040, 0.980, 0.335, Animation_Helper::EaseType::CircIn},
        {"circOut", 0.075, 0.820, 0.165, 1.000, Animation_Helper::EaseType::CircOut},
        {"circInOut", 0.785, 0.135, 0.150, 0.860, Animation_Helper::EaseType::CircInOut},
        {"backIn", 0.600, -0.280, 0.735, 0.045, Animation_Helper::EaseType::BackIn},
        {"backOut", 0.175, 0.885, 0.320, 1.275, Animation_Helper::EaseType::BackOut},
        {"backInOut", 0.680, -0.550, 0.265, 1.550, Animation_Helper::EaseType::BackInOut},
        // Special cases (no bezier points needed)
        {"elasticIn", 0, 0, 0, 0, Animation_Helper::EaseType::ElasticIn},
        {"elasticOut", 0, 0, 0, 0, Animation_Helper::EaseType::ElasticOut},
        {"elasticInOut", 0, 0, 0, 0, Animation_Helper::EaseType::ElasticInOut},
        {"bounceIn", 0, 0, 0, 0, Animation_Helper::EaseType::BounceIn},
        {"bounceOut", 0, 0, 0, 0, Animation_Helper::EaseType::BounceOut},
        {"bounceInOut", 0, 0, 0, 0, Animation_Helper::EaseType::BounceInOut}
    };

    double HandleElasticEasing(Animation_Helper::EaseType type, double t, double b, double c, double d) {
        constexpr double pi2 = 2.0 * PI;
        const double p = d * 0.3;
        const double s = p / 4.0;

        if (t <= 0.0) return b;
        if (t >= 1.0) return b + c;

        switch (type) {
            case Animation_Helper::EaseType::ElasticIn: {
                const double post_fix = c * std::pow(2.0, 10.0 * (t - 1.0));
                return -(post_fix * std::sin((t * d - s) * pi2 / p)) + b;
            }
            case Animation_Helper::EaseType::ElasticOut: {
                return (c * std::pow(2.0, -10.0 * t) * std::sin((t * d - s) * pi2 / p) + c + b);
            }
            case Animation_Helper::EaseType::ElasticInOut: {
                if (t < 0.5) {
                    const double post_fix = c * std::pow(2.0, 10.0 * (2.0 * t - 1.0));
                    return -0.5 * (post_fix * std::sin(((2.0 * t - 1.0) * d - s) * pi2 / p)) + b;
                }
                const double post_fix = c * std::pow(2.0, -10.0 * (2.0 * t - 1.0));
                return post_fix * std::sin(((2.0 * t - 1.0) * d - s) * pi2 / p) * 0.5 + c + b;
            }
            default:
                return b + c * t;
        }
    }

    double HandleBounceEasing(Animation_Helper::EaseType type, double t, double b, double c, double d) {
        switch (type) {
            case Animation_Helper::EaseType::BounceIn:
                return c - HandleBounceEasing(Animation_Helper::EaseType::BounceOut, 1.0 - t, 0, c, d) + b;
            case Animation_Helper::EaseType::BounceOut:
                if (t < (1.0 / 2.75)) {
                    return c * (7.5625 * t * t) + b;
                } else if (t < (2.0 / 2.75)) {
                    t -= (1.5 / 2.75);
                    return c * (7.5625 * t * t + 0.75) + b;
                } else if (t < (2.5 / 2.75)) {
                    t -= (2.25 / 2.75);
                    return c * (7.5625 * t * t + 0.9375) + b;
                }
                t -= (2.625 / 2.75);
                return c * (7.5625 * t * t + 0.984375) + b;
            case Animation_Helper::EaseType::BounceInOut:
                if (t < 0.5) {
                    return HandleBounceEasing(Animation_Helper::EaseType::BounceIn, t * 2.0, 0, c, d) * 0.5 + b;
                }
                return HandleBounceEasing(Animation_Helper::EaseType::BounceOut, t * 2.0 - 1.0, 0, c, d) * 0.5 + c * 0.5 + b;
            default:
                return b + c * t;
        }
    }
}

namespace Animation_Helper {

double CubicBezier(double progress, double x1, double y1, double x2, double y2) {
    // This is a cubic Bezier curve for animation timing:
    // P₀(0,0) - start point, always fixed
    // P₁(x1,y1) - first control point
    // P₂(x2,y2) - second control point
    // P₃(1,1) - end point, always fixed
    //
    // x coordinates (x1,x2) control the timing curve's shape
    // y coordinates (y1,y2) control the rate of change at those points
    
    ////progress = std::clamp(progress, 0.0, 1.0);
    
    // Since we need to find y for a given x (progress), 
    // we need to solve for t where the curve's x equals our progress
    // This is a rough approximation using a few iterations
    double t = progress;  // Initial guess
    
    // Newton's method to find better t value
    for (int i = 0; i < 5; i++) {  // Usually converges in 4-5 iterations
        const double currentT = t;
        const double oneMinusT = 1.0 - t;
        
        // Calculate x(t) - current x position on curve
        const double x = 3.0 * oneMinusT * oneMinusT * t * x1 +
                        3.0 * oneMinusT * t * t * x2 +
                        t * t * t;
        
        // If we're close enough to desired x, calculate final y
        if (std::abs(x - progress) < 0.001) {
            break;
        }
        
        // Calculate x'(t) - derivative of x with respect to t
        const double dx = 3.0 * oneMinusT * oneMinusT * x1 +
                         6.0 * oneMinusT * t * (x2 - x1) +
                         3.0 * t * t * (1 - x2);
        
        // Avoid division by zero
        if (std::abs(dx) < 0.0001) {
            break;
        }
        
        // Newton iteration
        t = t - (x - progress) / dx;
        //// t = std::clamp(t, 0.0, 1.0);
        
        // If t hasn't changed significantly, we're done
        if (std::abs(t - currentT) < 0.0001) {
            break;
        }
    }
    
    // Calculate final y value using found t
    const double oneMinusT = 1.0 - t;
    return 3.0 * oneMinusT * oneMinusT * t * y1 +
           3.0 * oneMinusT * t * t * y2 +
           t * t * t;
}

EaseType StringToEaseType(std::string_view type_name) {
    for (const auto& data : EASING_DATA) {
        if (data.name == type_name) {
            return data.type;
        }
    }
    return EaseType::Linear;
}

double GetEasedTime(EaseType type, double t, double b, double c, double d) {
    t = std::clamp(t, 0.0, d);
    const double normalized_t = t / d;

    // Handle special cases first
    switch (type) {
        case EaseType::ElasticIn:
        case EaseType::ElasticOut:
        case EaseType::ElasticInOut:
            return HandleElasticEasing(type, normalized_t, b, c, d);
        case EaseType::BounceIn:
        case EaseType::BounceOut:
        case EaseType::BounceInOut:
            return HandleBounceEasing(type, normalized_t, b, c, d);
        default:
            break;
    }

    // Find bezier points for the type
    for (const auto& data : EASING_DATA) {
        if (data.type == type) {
            return b + c * CubicBezier(normalized_t, data.x1, data.y1, data.x2, data.y2);
        }
    }

    return b + c * normalized_t; // Linear fallback
}

double GetEasedTime(const std::string& easing_type, double t, double b, double c, double d) {
    if (easing_type.substr(0, 6) == "bezier") {
        std::vector<double> params;
        size_t start = easing_type.find('(') + 1;
        size_t end = easing_type.find(')');
        if (start != std::string::npos && end != std::string::npos) {
            std::string values = easing_type.substr(start, end - start);
            std::istringstream iss(values);
            double val;
            while (iss >> val) {
                params.push_back(val);
                iss.ignore();
            }
            if (params.size() == 4) {
                return b + c * CubicBezier(t/d, params[0], params[1], params[2], params[3]);
            }
        }
    }

    return GetEasedTime(StringToEaseType(easing_type), t, b, c, d);
}

} // namespace Animation_Helper
