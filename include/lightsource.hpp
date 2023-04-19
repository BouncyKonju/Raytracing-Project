#pragma once

#include <utility.hpp>

namespace Raytracing {
    /**
     * @brief Represents a light source in a scene
     * 
     */
    class LightSource
    {
    public:
        // The position of the light source in 3d-space
        Utility::Vec3 pos;
        // The color the source emits
        Utility::Vec3 color;

        /**
         * @brief Construct a new Light Source object using Vec3s
         * 
         * @param pos The position in 3D space
         * @param color The color (r, g, b)
         */
        LightSource(const Utility::Vec3& pos, const Utility::Vec3& color) : pos(pos), color(color) {}
        /**
         * @brief Construct a new Light Source object using doubles
         * 
         * @param x \
         * @param y  |--> Position in 3D space
         * @param z /
         * @param r \
         * @param g  |--> Color
         * @param b /
         */
        LightSource(double x, double y, double z, double r, double g, double b) : pos(x, y, z), color(r, g, b) {}
    };
}