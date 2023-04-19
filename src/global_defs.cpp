/*

This file should include all global definitions of standard objects.
This does not necessarily include definitions of the type
                constexpr double PI = 3.14;
but should be reserved for non-constexpr'able objects like materials or objects.
Any global should be defined in its header following the scheme
                extern TYPE GLOBAL_NAME;
and then in here following
                TYPE GLOBAL_NAME (initializer list);
The linker will do the rest.

*/

#include <memory>

#include <material.hpp>
#include <baseobject.hpp>

unsigned Raytracing::Material::_matid = 0;

// Default material if no other is provided
std::shared_ptr<Raytracing::Material> Raytracing::BASE_MATERIAL
    ((new Raytracing::Material(0.1, 0.2, 0.2, 0.6, 0.4, 1.5, 1.0, Utility::Vec3(1., 1., 1.))));

// Base sphere object
std::shared_ptr<Raytracing::Object> Raytracing::SPHERE
    (new Raytracing::BaseObject(Raytracing::BaseTypes::Sphere, Utility::Vec3(), 1., Raytracing::BASE_MATERIAL));
// Base halfplane object
std::shared_ptr<Raytracing::Object> Raytracing::HALFPLANE
    (new Raytracing::BaseObject(Raytracing::BaseTypes::HalfPlane, Utility::Vec3(), 1., Raytracing::BASE_MATERIAL));