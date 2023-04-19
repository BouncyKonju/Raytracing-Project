#pragma once

#include <memory>

#include <utility.hpp>
#include <object.hpp>
#include <material.hpp>

namespace Raytracing {

    // The two basic types for an object
    enum class BaseTypes { Sphere, HalfPlane };

    /**
     * @brief A basic object that is either a sphere or a half-plane.
     * 
     */
    class BaseObject : public Object
    {
    public:
        // Position of the sphere OR normal of the half-plane
        Utility::Vec3 pos;
        // Radius of the sphere OR orientation of the half-plane
        double rd;
        // Type of the object
        BaseTypes bt;

        /**
         * @brief Base constructor with mat_id using Vec3
         * 
         * @param bt Specifies the type
         * @param pos Specifies the position in 3D space
         * @param rd Specifies a) radius if it's a sphere & b) orientation if it's a halfspace
         * @param mat_id The ID of the material this object is assigned to
         */
        BaseObject(BaseTypes bt, const Utility::Vec3& pos, double rd, const unsigned mat_id) : bt(bt), pos(pos), rd(rd) { this->mat_id = mat_id; this->_type = ObjectType::Base; }
        /**
         * @brief Base constructor with mat pointer using Vec3
         * 
         * @param bt Specifies the type
         * @param pos Specifies the position in 3D space
         * @param rd Specifies a) radius if it's a sphere & b) orientation if it's a halfspace
         * @param mat A reference to the material this object is assigned to. ID will be automatically extracted
         */
        BaseObject(BaseTypes bt, const Utility::Vec3& pos, double rd, std::shared_ptr<Material>& mat) : bt(bt), pos(pos), rd(rd) { this->mat_id = mat->mat_id; this->_type = ObjectType::Base; }
        /**
         * @brief Base constructor with mat_id using doubles
         * 
         * @param bt Specifies the type
         * @param x Specifies the x position
         * @param y Specifies the y position
         * @param z Specifies the z position
         * @param rd Specifies a) radius if it's a sphere & b) orientation if it's a halfspace
         * @param mat_id The ID of the material this object is assigned to
         */
        BaseObject(BaseTypes bt, double x, double y, double z, double rd, const unsigned mat_id) : bt(bt), pos(x, y, z), rd(rd) { this->mat_id = mat_id; this->_type = ObjectType::Base; }
        /**
         * @brief Base constructor with mat pointer using doubles
         * 
         * @param bt Specifies the type
         * @param x Specifies the x position
         * @param y Specifies the y position
         * @param z Specifies the z position
         * @param rd Specifies a) radius if it's a sphere & b) orientation if it's a halfspace
         * @param mat A reference to the material this object is assigned to. ID will be automatically extracted
         */
        BaseObject(BaseTypes bt, double x, double y, double z, double rd, std::shared_ptr<Material>& mat) : bt(bt), pos(x, y, z), rd(rd) { this->mat_id = mat->mat_id; this->_type = ObjectType::Base; }
    };

    extern std::shared_ptr<Object> SPHERE;
    extern std::shared_ptr<Object> HALFPLANE;

}
