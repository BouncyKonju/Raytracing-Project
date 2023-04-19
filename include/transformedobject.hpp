#pragma once

#include <memory>

#include <utility.hpp>
#include <object.hpp>

namespace Raytracing {

    // Enumerates all transformations doable
    enum class TransformOps { Scale, Rotatex, Rotatey, Rotatez, Transform };

    /**
     * @brief Represents a single transformation of another object
     * 
     */
    class TransformedObject : public Object
    {
    public:
        // Specifies which transformation is done
        TransformOps op;
        // Represents a scale done, can be anything from actual scale to rotation to transformation
        Utility::Vec3 scale;
        // Points to the object transformed
        std::shared_ptr<Object> obj;

        /**
         * @brief Get the matrix this object and possible linked transformation objects represent(s)
         * 
         * @return The transformation matrix
         */
        Utility::Matrix4x4 getMatrix();
        /**
         * @brief Get the inverse matrix this object and possible linked transformation objects represent(s)
         * 
         * @return The inverse transformation matrix
         */
        Utility::Matrix4x4 getInverseMatrix();

        /**
         * @brief Construct a new Transformed Object using Vec3
         * 
         * @param op Specifies the operation represented by this object
         * @param scale The scale at which the operation is done. For rotations, only scale.x() is relevant, for scaling and transforming also y and z additionally.
         * @param obj The object affected.
         */
        TransformedObject(TransformOps op, Utility::Vec3 scale, std::shared_ptr<Object>& obj)
            : op(op), scale(scale) {
                this->_type = ObjectType::Transformed;
                if (obj->type() == ObjectType::Base || obj->type() == ObjectType::Transformed)
                    this->obj = obj;
                else
                    throw Utility::WRONG_OBJECT_HIERARCHY_EXCEPTION;
            }
        // Base constructor using doubles
        /**
         * @brief Construct a new Transformed Object using doubles
         * 
         * @param op Specifies the operation represented by this object
         * @param scalex \
         * @param scaley |--> The scale at which the operation is done. For rotations, only scale.x() is relevant, for scaling and transforming also y and z additionally.
         * @param scalez /
         * @param obj The object affected.
         */
        TransformedObject(TransformOps op, double scalex, double scaley, double scalez, std::shared_ptr<Object>& obj)
            : op(op), scale(scalex, scaley, scalez), obj(obj) {
                this->_type = ObjectType::Transformed;
                if (obj->type() == ObjectType::Base || obj->type() == ObjectType::Transformed)
                    this->obj = obj;
                else
                    throw Utility::WRONG_OBJECT_HIERARCHY_EXCEPTION;
            }
    };

}
