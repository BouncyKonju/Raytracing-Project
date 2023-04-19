#pragma once

#include <cmath>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <array>
#include <memory>

#include <utility.hpp>
#include <object.hpp>
#include <lightsource.hpp>
#include <material.hpp>
#include <ray.hpp>

namespace Raytracing {

    // Lists all variables required for rendering
    const std::string RequiredVariables[] = {
        "width", "height",
        "lookat_x", "lookat_y", "lookat_z",
        "eyepos_x", "eyepos_y", "eyepos_z",
        "ambient_r", "ambient_g", "ambient_b",
        "ambient_int_r", "ambient_int_g", "ambient_int_r",
        "raydepth"
    };

    // Lists all different non-var types recognizable by the interpreter
    enum class CreationSigns { Object, Light, Material, Comment };
    // Explicit non-var markers
    constexpr char CreationChars[] = { '!', '*', '?', '/' };

    // Lists all different assignment operators recognizable by the interpreter
    enum class Operators { Assignment, MatSet, Scale, RotateX, RotateY, RotateZ, Transform, Submit };
    // Explicit assignment operators
    const std::string OperatorStrings[] = {
        ":=", "?=", "*=", "#x=", "#y=", "#z=", "+=", "<="
    };

    // Object base type strings
    const std::string BasetypeStrings[] = {
        "sphere", "hp"
    };

    /**
     * @brief Interpreter to read render-data
     * 
     */
    class Interpreter 
    {
    private:
        // Field of view of the 'camera'
        static constexpr double fov = Utility::PI / 2.;
    public:
        // The top object in the scene tree,
        // All relevant objects are sub-objects of this one
        std::shared_ptr<Object> topObject;
        // Information about the amount of base objects
        unsigned base_objs;
        // All read light sources
        std::map<std::string, std::shared_ptr<LightSource>> lightSources;
        // All read materials
        std::map<std::string, std::shared_ptr<Material>> materials;
        // All required variable values
        std::map<std::string, double> variables;
        // All rays on screen
        std::vector<std::shared_ptr<Ray>> rays;

        // Position of the eye in 3D space
        Utility::Vec3 EyePos;
        // Position the eye is looking at
        Utility::Vec3 Lookat;

        // The height of the csg tree
        unsigned tree_height;
        // The amount of complex operations in the tree
        unsigned cmpOps;

        /**
         * @brief Construct a new Interpreter object
         * 
         */
        Interpreter();
        /**
         * @brief Interprets an input file and makes it ready for rendering
         * 
         * @param path Specifies the path to the input file for the program
         */
        void interpretFile(const std::string& path);
    private:
        // Actual interpretation method
        void interpret(std::ifstream& f);
        // Creates rays after reading required values from input file
        void createRays();
        // Shortens the object tree to Complex -> ... -> Complex -> Transform -> Base
        std::shared_ptr<Raytracing::Object> shorten(std::shared_ptr<Raytracing::Object>& obj, unsigned depth);
    };
}
