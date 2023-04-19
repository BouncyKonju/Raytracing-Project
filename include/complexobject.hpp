#pragma once

#include <memory>

#include <utility.hpp>
#include <object.hpp>

namespace Raytracing {

    // All operations that a complex object can express
    enum class ComplexOps { Union = 0, Intersection = 1, Subtraction = 2 };

    // All interpreter-strings that delineate which complex op is done
    const std::string ComplexStrings[] = {
        "|", "&", "-"
    };

    /**
     * @brief A complex object, combining one or more other objects
     * 
     */
    class ComplexObject : public Object
    {
    public:
        // Specifies what is done
        ComplexOps operation;
        // 'Left' object in the tree
        std::shared_ptr<Object> left;
        // 'Right' object in the tree
        std::shared_ptr<Object> right;

        /**
         * @brief Construct a new Complex Object
         * 
         * @param op Specifies the operation this object represents
         * @param left The left object below this one in the tree
         * @param right The right object below this one in the tree
         */
        ComplexObject(ComplexOps op, std::shared_ptr<Object>& left, std::shared_ptr<Object>& right)
            : operation(op), left(left), right(right) {
                this->_type = ObjectType::Complex;
            }
    };

}
