#pragma once

#include <utility.hpp>

namespace Raytracing {
    /**
     * @brief A ray as used in calculations.
     * 
     */
    class Ray
    {
    public:
        // Vector start, corresponds to eyepos
        const Utility::Vec3& start;
        // Vector direction
        Utility::Vec3 dir;
        // Corresponding pixel position on screen
        int i, j;

        /**
         * @brief Constructs a ray with a given start and a direction vector
         * 
         * @param start Start position of the ray
         * @param dir Direction vector
         */
        Ray(const Utility::Vec3& start, const Utility::Vec3& dir) : start(start), dir(dir) {}
        /**
         * @brief Construct a ray with a given start and a moved vector
         * 
         * @param start Start position of the ray
         * @param dir Direction vector (Rvalue)
         */
        Ray(const Utility::Vec3& start, const Utility::Vec3&& dir) : start(start), dir(dir) {}
        /**
         * @brief Construct a ray with a given start and three direction values
         * 
         * @param start Start position of the ray
         * @param dx \
         * @param dy |--> Direction vector
         * @param dz /
         */
        Ray(const Utility::Vec3& start, double dx, double dy, double dz) : start(start), dir(dx, dy, dz) {}
    };
}
