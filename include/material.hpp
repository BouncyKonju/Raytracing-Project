#pragma once

#include <utility.hpp>

namespace Raytracing {
    /**
     * @brief A material that specifies how an object behaves under ray influence
     * 
     */
    class Material
    {
    private:
        static unsigned _matid;
    public:
        // Material ID for transmission to computation device
        unsigned mat_id;
        // Ambient reflection component
        double ambref;
        // Diffuse reflection component
        double diffref;
        // Specular reflection component
        double specref;
        // Reflected component
        double rflec;
        // Refracted component
        double rfrac;
        // Refracted index
        double rfracind;
        // Shininess constant
        double shiny;
        // Color of the lightsource
        Utility::Vec3 color;

        /**
         * @brief Construct a new Material object using Vec3
         * 
         * @param ambref Ambient reflection component
         * @param diffref Diffuse reflection component
         * @param specref Specular reflection component
         * @param rflec Reflective component
         * @param rfrac Refractive component
         * @param rfracind Refractive index
         * @param shiny Shininess index (higher means less shiny)
         * @param color Color of the material (r, g, b)
         */
        Material(double ambref, double diffref, double specref, double rflec, double rfrac, double rfracind, double shiny, Utility::Vec3 color)
        : ambref(ambref), diffref(diffref), specref(specref), rflec(rflec), rfrac(rfrac), rfracind(rfracind), shiny(shiny), color(color), mat_id(_matid++) {}

        /**
         * @brief Construct a new Material object using doubles
         * 
         * @param ambref Ambient reflection component
         * @param diffref Diffuse reflection component
         * @param specref Specular reflection component
         * @param rflec Reflective component
         * @param rfrac Refractive component
         * @param rfracind Refractive index
         * @param shiny Shininess index (higher means less shiny)
         * @param r \
         * @param g |--> Color
         * @param b /
         */
        Material(double ambref, double diffref, double specref, double rflec, double rfrac, double rfracind, double shiny, double r, double g, double b)
        : ambref(ambref), diffref(diffref), specref(specref), rflec(rflec), rfrac(rfrac), rfracind(rfracind), shiny(shiny), color(r, g, b), mat_id(_matid++) {}
    };

    extern std::shared_ptr<Material> BASE_MATERIAL;
}