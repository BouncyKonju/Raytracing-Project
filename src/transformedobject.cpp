#include <cmath>
#include <transformedobject.hpp>

namespace Raytracing {

Utility::Matrix4x4 TransformedObject::getMatrix()
{
    Utility::Matrix4x4 res;
    switch(this->op)
    {
    case TransformOps::Scale:
        res.mat[0][0] = scale.x();
        res.mat[1][1] = scale.y();
        res.mat[2][2] = scale.z();
        break;
    case TransformOps::Rotatex:
        res.mat[1][1] =  std::cos(scale.x());
        res.mat[2][2] =  std::cos(scale.x());
        res.mat[1][2] = -std::sin(scale.x());
        res.mat[2][1] =  std::sin(scale.x());
        break;
    case TransformOps::Rotatey:
        res.mat[0][0] =  std::cos(scale.x());
        res.mat[0][2] = -std::sin(scale.x());
        res.mat[2][0] =  std::sin(scale.x());
        res.mat[2][2] =  std::cos(scale.x());
        break;
    case TransformOps::Rotatez:
        res.mat[0][0] =  std::cos(scale.x());
        res.mat[1][1] =  std::cos(scale.x());
        res.mat[1][0] =  std::sin(scale.x());
        res.mat[0][1] = -std::sin(scale.x());
        break;
    case TransformOps::Transform:
        res.mat[0][3] = scale.x();
        res.mat[1][3] = scale.y();
        res.mat[2][3] = scale.z();
        break;
    }
    if (obj->type() == ObjectType::Transformed)
        res = std::dynamic_pointer_cast<Raytracing::TransformedObject>(obj)->getMatrix() * res;

    return res;
}

Utility::Matrix4x4 TransformedObject::getInverseMatrix()
{
    Utility::Matrix4x4 res;
    switch(this->op)
    {
    case TransformOps::Scale:
        res.mat[0][0] = 1. / scale.x();
        res.mat[1][1] = 1. / scale.y();
        res.mat[2][2] = 1. / scale.z();
        break;
    case TransformOps::Rotatex:
        res.mat[1][1] =  std::cos(-scale.x());
        res.mat[2][2] =  std::cos(-scale.x());
        res.mat[1][2] = -std::sin(-scale.x());
        res.mat[2][1] =  std::sin(-scale.x());
        break;
    case TransformOps::Rotatey:
        res.mat[0][0] =  std::cos(-scale.x());
        res.mat[0][2] = -std::sin(-scale.x());
        res.mat[2][0] =  std::sin(-scale.x());
        res.mat[2][2] =  std::cos(-scale.x());
        break;
    case TransformOps::Rotatez:
        res.mat[0][0] =  std::cos(-scale.x());
        res.mat[1][1] =  std::cos(-scale.x());
        res.mat[1][0] =  std::sin(-scale.x());
        res.mat[0][1] = -std::sin(-scale.x());
        break;
    case TransformOps::Transform:
        res.mat[0][3] = -scale.x();
        res.mat[1][3] = -scale.y();
        res.mat[2][3] = -scale.z();
        break;
    }
    if (obj->type() == ObjectType::Transformed)
        res = res * std::dynamic_pointer_cast<Raytracing::TransformedObject>(obj)->getInverseMatrix();

    return res;
}

}