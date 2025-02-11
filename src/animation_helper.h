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

#ifndef ANIMATION_HELPER_H
#define ANIMATION_HELPER_H

#include <string>
#include <string_view>

namespace Animation_Helper {
    /**
     * Supported easing types for animations.
     * These match CSS animation timing functions for familiarity.
     */
    enum class EaseType {
        Linear,
        // Basic
        Ease,
        EaseIn,
        EaseOut,
        EaseInOut,
        // Quadratic
        QuadIn,
        QuadOut,
        QuadInOut,
        // Cubic
        CubicIn,
        CubicOut,
        CubicInOut,
        // Quartic
        QuartIn,
        QuartOut,
        QuartInOut,
        // Quintic
        QuintIn,
        QuintOut,
        QuintInOut,
        // Sinusoidal
        SineIn,
        SineOut,
        SineInOut,
        // Exponential
        ExpoIn,
        ExpoOut,
        ExpoInOut,
        // Circular
        CircIn,
        CircOut,
        CircInOut,
        // Back
        BackIn,
        BackOut,
        BackInOut,
        // Elastic
        ElasticIn,
        ElasticOut,
        ElasticInOut,
        // Bounce
        BounceIn,
        BounceOut,
        BounceInOut
    };

    /**
     * Converts a string to an EaseType enum value
     * @param type_name The name of the easing type
     * @return The corresponding EaseType enum value
     */
    EaseType StringToEaseType(std::string_view type_name);

    /**
     * Calculates a point on a cubic Bezier curve for animation timing
     * @param progress Current animation progress (0 to 1)
     * @param x1 X coordinate of first control point, controls when early changes occur
     * @param y1 Y coordinate of first control point, controls magnitude of early changes
     * @param x2 X coordinate of second control point, controls when later changes occur
     * @param y2 Y coordinate of second control point, controls magnitude of later changes
     * @return The calculated animation progress value
     * 
     * Control points affect the animation as follows:
     * - x values control timing (when changes happen)
     * - y values control magnitude (how much change occurs)
     * - Values > current progress create overshoots
     * - Values < current progress create undershoots
     */
    double CubicBezier(double progress, double x1, double y1, double x2, double y2);

    /**
     * Gets the eased time value based on the specified easing type
     * @param easing_type Type of easing to apply
     * @param t Current time (0 to 1)
     * @param b Start value
     * @param c Change in value (end - start)
     * @param d Duration
     * @return The eased value
     */
    double GetEasedTime(const std::string& easing_type, double t, double b, double c, double d);

    /**
     * Gets the eased time value using enum-based easing type
     * @param type The easing type enum
     * @param t Current time (0 to 1)
     * @param b Start value
     * @param c Change in value (end - start)
     * @param d Duration
     * @return The eased value
     */
    double GetEasedTime(EaseType type, double t, double b, double c, double d);
}

#endif
