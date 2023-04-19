#pragma once

#include <memory>
#include <material.hpp>

namespace Raytracing {
    // Specifies which general type of object this is. Used for ordering in the interpreter.
    enum class ObjectType { None, Base, Transformed, Complex, Fulltransform };

    /**
     * @brief Base object without further specification
     * 
     */
    class Object
    {
    protected:
        // Object type used in rendering
        ObjectType _type;
    public:
        // Material of the object
        unsigned mat_id;

        /**
         * @brief  Get the type of this object
         * 
         * @return The type as Raytracing::ObjectType
         */
        constexpr ObjectType type() const noexcept { return this->_type; }

        /**
         * @brief Constructs a base object
         * 
         */
        constexpr Object() : _type(ObjectType::None), mat_id(0) {};

        // Necessary for std::dynamic_pointer_cast
        /**
         * @brief Destroy the Object (Necessary for std::dynamic_pointer_cast)
         * 
         */
        virtual ~Object() {}
    };
}