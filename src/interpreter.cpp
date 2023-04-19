#include <stdexcept>
#include <cmath>

#include <interpreter.hpp>
#include <baseobject.hpp>
#include <transformedobject.hpp>
#include <fulltransobject.hpp>
#include <complexobject.hpp>

using namespace Raytracing;
using Utility::to_underlying;

Interpreter::Interpreter()
: tree_height(0), base_objs(0), cmpOps(0)
{
    // Init required variables
    for (auto& i : Raytracing::RequiredVariables)
        this->variables[i] = 0.0;
}

void Interpreter::interpret(std::ifstream& f)
{
    bool object_tree_locked = false;
    std::map<std::string, double> stack; // Even though it behaves more like a heap
    std::map<std::string, std::shared_ptr<Raytracing::Object>> object_stack;
    auto get = [=] (const std::string& val) {
        try
        {
            return std::stod(val);
        }
        catch (std::invalid_argument e)
        {
            if (val == "pi") return Utility::PI;
            return stack.at(val);
        }
    };
    this->materials["null"] = Raytracing::BASE_MATERIAL;

    std::string line;
    while (std::getline(f, line))
    {
        std::vector<std::string> tokens;
        Utility::split(line, tokens);
        if (tokens.size() < 2) throw Utility::WRONG_FORMAT_EXCEPTION;
        auto& lvalue = tokens[0];
        auto& com = tokens[1];

        switch (lvalue[0]) {
        case CreationChars[to_underlying(CreationSigns::Comment)]: break;
        case CreationChars[to_underlying(CreationSigns::Object)]:
            if (object_tree_locked) break;
            // Yes, this is ugly, but splitting things into lambdas or the like wouldn't be much nicer
            // This interpreter doesn't need to be extremely extendable or anything, just functional
            lvalue = Utility::remove_first(lvalue);
            if (com == OperatorStrings[to_underlying(Operators::Assignment)]) // Creation, union, intersection, exclusion, negation
            {
                if (tokens.size() == 3) // Object creation
                {
                    if (tokens[2] == BasetypeStrings[to_underlying(BaseTypes::Sphere)])
                        object_stack[lvalue] = std::make_shared<Raytracing::BaseObject>(Raytracing::BaseObject(Raytracing::BaseTypes::Sphere, Utility::Vec3(), 1., Raytracing::BASE_MATERIAL));
                    else if (tokens[2] == BasetypeStrings[to_underlying(BaseTypes::HalfPlane)])
                        object_stack[lvalue] = std::make_shared<Raytracing::BaseObject>(Raytracing::BaseObject(Raytracing::BaseTypes::HalfPlane, Utility::Vec3(), 1., Raytracing::BASE_MATERIAL));
                    else
                        throw Utility::WRONG_FORMAT_EXCEPTION;
                    this->base_objs++;
                }
                else if (tokens.size() == 5) // Union, Intersection, Exclusion
                {
                    auto& op = tokens[3];
                    if (op == ComplexStrings[to_underlying(ComplexOps::Union)])
                        object_stack[lvalue] = std::shared_ptr<Object>
                            (new ComplexObject(ComplexOps::Union, object_stack[tokens[2]], object_stack[tokens[4]]));
                    else if (op == ComplexStrings[to_underlying(ComplexOps::Intersection)])
                        object_stack[lvalue] = std::shared_ptr<Object>
                            (new ComplexObject(ComplexOps::Intersection, object_stack[tokens[2]], object_stack[tokens[4]]));
                    else if (op == ComplexStrings[to_underlying(ComplexOps::Subtraction)])
                        object_stack[lvalue] = std::shared_ptr<Object>
                            (new ComplexObject(ComplexOps::Subtraction, object_stack[tokens[2]], object_stack[tokens[4]]));
                }
                else throw Utility::WRONG_FORMAT_EXCEPTION;
            }
            else if (com == OperatorStrings[to_underlying(Operators::MatSet)])
            {
                if (tokens.size() < 3) throw Utility::WRONG_FORMAT_EXCEPTION;
                object_stack[lvalue]->mat_id = this->materials[tokens[2]]->mat_id;
            }
            else if (com == OperatorStrings[to_underlying(Operators::Scale)])
            {
                if (tokens.size() < 5) throw Utility::WRONG_FORMAT_EXCEPTION;
                object_stack[lvalue] = std::shared_ptr<Object>(new TransformedObject(
                    TransformOps::Scale,
                    get(tokens[2]), get(tokens[3]), get(tokens[4]),
                    object_stack[lvalue]
                ));
            }
            else if (com == OperatorStrings[to_underlying(Operators::RotateX)])
            {
                if (tokens.size() < 3) throw Utility::WRONG_FORMAT_EXCEPTION;
                object_stack[lvalue] = std::shared_ptr<Object>(new TransformedObject(
                    TransformOps::Rotatex,
                    get(tokens[2]), 0., 0.,
                    object_stack[lvalue]
                ));
            }
            else if (com == OperatorStrings[to_underlying(Operators::RotateY)])
            {
                if (tokens.size() < 3) throw Utility::WRONG_FORMAT_EXCEPTION;
                object_stack[lvalue] = std::shared_ptr<Object>(new TransformedObject(
                    TransformOps::Rotatey,
                    get(tokens[2]), 0., 0.,
                    object_stack[lvalue]
                ));
            }
            else if (com == OperatorStrings[to_underlying(Operators::RotateZ)])
            {
                if (tokens.size() < 3) throw Utility::WRONG_FORMAT_EXCEPTION;
                object_stack[lvalue] = std::shared_ptr<Object>(new TransformedObject(
                    TransformOps::Rotatez,
                    get(tokens[2]), 0., 0.,
                    object_stack[lvalue]
                ));
            }
            else if (com == OperatorStrings[to_underlying(Operators::Transform)])
            {
                if (tokens.size() < 5) throw Utility::WRONG_FORMAT_EXCEPTION;
                object_stack[lvalue] = std::shared_ptr<Object>(new TransformedObject(
                    TransformOps::Transform,
                    get(tokens[2]), get(tokens[3]), get(tokens[4]),
                    object_stack[lvalue]
                ));
            }
            else if (com == OperatorStrings[to_underlying(Operators::Submit)])
            {
                this->topObject = object_stack[lvalue];
                object_tree_locked = true;
            }
            break;
        case CreationChars[to_underlying(CreationSigns::Light)]:
            if (com != OperatorStrings[to_underlying(Operators::Assignment)] || tokens.size() < 8) throw Utility::WRONG_FORMAT_EXCEPTION;
            lvalue = Utility::remove_first(lvalue);
            try {
                lightSources[lvalue] = std::shared_ptr<LightSource>(new LightSource(
                    get(tokens[2]),
                    get(tokens[3]),
                    get(tokens[4]),
                    get(tokens[5]),
                    get(tokens[6]),
                    get(tokens[7])
                ));
            } catch (std::logic_error e) {
                std::cout << "Invalid light creation at " << line << std::endl;
                throw Utility::WRONG_FORMAT_EXCEPTION;
            }
            break;
        case CreationChars[to_underlying(CreationSigns::Material)]:
            if (com != OperatorStrings[to_underlying(Operators::Assignment)] || tokens.size() < 10) throw Utility::WRONG_FORMAT_EXCEPTION;
            lvalue = Utility::remove_first(lvalue);
            try {
                materials[lvalue] = std::shared_ptr<Material>(new Material(
                    get(tokens[2]),
                    get(tokens[3]),
                    get(tokens[4]),
                    get(tokens[5]),
                    get(tokens[6]),
                    get(tokens[7]),
                    get(tokens[8]),
                    get(tokens[9]),
                    get(tokens[10]),
                    get(tokens[11])
                ));
            } catch (std::logic_error e) {
                std::cout << "Invalid material creation at " << line << std::endl;
                throw Utility::WRONG_FORMAT_EXCEPTION;
            }
            break;
        default: // Its a variable!
            if (com != OperatorStrings[to_underlying(Operators::Assignment)]) throw Utility::WRONG_FORMAT_EXCEPTION;
            try {
                stack[lvalue] = get(tokens[2]);
            } catch (std::out_of_range e) {
                continue;
            }
        }
    }
    try {
        for (auto& i : this->variables)
            i.second = stack[i.first];
    } catch (std::out_of_range) {
        throw Utility::MISSING_VARIABLE_EXCEPTION;
    }
    f.close();
    
    this->EyePos.vals[0] = variables["eyepos_x"];
    this->EyePos.vals[1] = variables["eyepos_y"];
    this->EyePos.vals[2] = variables["eyepos_z"];
    this->Lookat.vals[0] = variables["lookat_x"];
    this->Lookat.vals[1] = variables["lookat_y"];
    this->Lookat.vals[2] = variables["lookat_z"];

    this->topObject = shorten(this->topObject, 0);

    return;
}

void Interpreter::createRays()
{
    double m = variables["height"];
    double k = variables["width"];
    // Inverse Aspect ratio
    double iar = m / k;
    auto t = Lookat - EyePos;
    auto b = Utility::Vec3(0, -1, 0) % t;
    auto tn = t.normalise();
    auto bn = b.normalise();
    auto vn = tn % bn;
    auto gx = std::tan(fov / 2.);
    auto gy = gx * iar;
    auto qx = bn * ((2. * gx) / (k - 1.));
    auto qy = vn * ((2. * gy) / (m - 1.));
    auto p1m = tn - bn * gx - vn * gy;
    this->rays.resize(static_cast<size_t>(k) * static_cast<size_t>(m));
    for (unsigned i = 1; i <= static_cast<unsigned>(k); i++)
    {
        for (unsigned j = 1; j <= static_cast<unsigned>(m); j++)
        {
            this->rays[(i - 1) + (j - 1) * k] = std::shared_ptr<Ray>(new Ray(
                this->EyePos,
                (p1m + qx * (i - 1) + qy * (j - 1)).normalise()
            ));
        }
    }
}

std::shared_ptr<Raytracing::Object> Interpreter::shorten(std::shared_ptr<Raytracing::Object>& obj, unsigned depth)
{
    if (depth > this->tree_height) tree_height = depth;
    if (obj->type() == ObjectType::Complex)
    {
        this->cmpOps++;
        std::dynamic_pointer_cast<Raytracing::ComplexObject>(obj)->left =
            shorten(std::dynamic_pointer_cast<Raytracing::ComplexObject>(obj)->left, depth + 1);
        std::dynamic_pointer_cast<Raytracing::ComplexObject>(obj)->right =
            shorten(std::dynamic_pointer_cast<Raytracing::ComplexObject>(obj)->right, depth + 1);
        return obj;
    }
    else if (obj->type() == ObjectType::Transformed)
    {
        auto t = std::make_shared<Raytracing::Fulltransform>(Raytracing::Fulltransform(
            std::dynamic_pointer_cast<Raytracing::TransformedObject>(obj)->getMatrix(),
            std::dynamic_pointer_cast<Raytracing::TransformedObject>(obj)->getInverseMatrix()
        ));
        auto tmp = obj;
        while (tmp->type() == ObjectType::Transformed)
            tmp = std::dynamic_pointer_cast<Raytracing::TransformedObject>(tmp)->obj;
        t->obj = tmp;
        // Base object 
        this->tree_height++;
        return t;
    }
    else return obj;
}

void Interpreter::interpretFile(const std::string& path)
{
    std::ifstream inFile;
    inFile.open(path);
    if (!inFile.is_open())
    {
        std::cout << "Can't open file " << path << "! Aborting interpretation." << std::endl;
        return;
    }
    interpret(inFile);
    createRays();
    return;
}
