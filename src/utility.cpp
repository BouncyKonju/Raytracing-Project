#include <utility.hpp>

std::ostream& Utility::operator<<(std::ostream& out, const Utility::Exception& e)
{
    out <<
        "---" <<
        std::endl <<
        "Encountered exception No. " <<
        e.id <<
        ": " <<
        std::endl <<
        e.description <<
        " Aborting." <<
        std::endl <<
        "---" <<
    std::endl;
    return out;
}

void Utility::printException(const Utility::Exception& e)
{
    std::cout << e;
}

std::ostream& Utility::operator<<(std::ostream& out, const Utility::Vec3& vec)
{
    out << '(' << vec.x() << ", " << vec.y() << ", " << vec.z() << ')';
    return out;
}

std::ostream& Utility::operator<<(std::ostream& out, const Utility::Vec4& vec)
{
    out << '(' << vec.x() << ", " << vec.y() << ", " << vec.z()  << ", " << vec.h() << ')';
    return out;
}

std::ostream& Utility::operator<<(std::ostream& out, const Utility::Matrix4x4& mat)
{
    out << '[';
    for (auto& i : mat.mat)
    {
        out << '(';
        for (auto& j : i)
            out << j << ' ';
        out << ')' << std::endl;
    }
    out << ']';
    return out;
}

// https://stackoverflow.com/a/5888166
void Utility::split(const std::string& str, std::vector<std::string>& v) {
    std::stringstream ss(str);
    ss >> std::noskipws;
    std::string field;
    char ws_delim;
    while(1) {
        if( ss >> field )
            v.push_back(field);
        else if (ss.eof())
            break;
        else
            v.push_back(std::string());
        ss.clear();
        ss >> ws_delim;
    }
}

Utility::AutoArray<uint8_t> Utility::openclMemToArray(const Memory<cl_float3>& other)
{
	Utility::AutoArray<uint8_t> result(other.length() * 3);
	for (size_t i = 0; i < other.length(); i++)
	{
		// OpenCV pixel format is BGR instead of RGB so we also need to flip this
		result.array[3 * i + 0] = static_cast<uint8_t>(other[i].v4[2] * 255.);
		result.array[3 * i + 1] = static_cast<uint8_t>(other[i].v4[1] * 255.);
		result.array[3 * i + 2] = static_cast<uint8_t>(other[i].v4[0] * 255.);
	}
	return result;
}