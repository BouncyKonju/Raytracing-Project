#pragma once

#include <cmath>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <cstdlib>

#include <opencl.hpp>

namespace Utility {

    // A simple, run-time sized array that auto-allocs and auto-frees.
    template <typename T>
    struct AutoArray {
        T* array;
        /**
         * @brief Construct a new Auto Array object
         * @exception None This does not throw an exception because malloc never throws (compatibility with C) [https://cplusplus.com/reference/cstdlib/malloc/]
         * 
         * @param size The length of the array
         */
        inline AutoArray(size_t size) noexcept
        {
            array = (T*) malloc(size * sizeof(T));
        }
        /**
         * @brief Deallocate the Auto Array
         * 
         * @exception None This does not throw an exception because free never throws (compatibility with C)
         * 
         */
        inline ~AutoArray() noexcept
        {
            free(array);
        }
    };

    // Standard value for pi
    constexpr double PI = 3.14159265358979323846264338327950288;

    /**
     * @brief This function converts an enum to its underlying value.
     * 
     * @tparam T The enum type
     * @param e The enum value
     * @return The underlying type value. Use auto in your variable declaration except if you're really sure what you get.
     */
    template <typename T>
    constexpr auto to_underlying(T e) noexcept {
        return static_cast<std::underlying_type_t<T>>(e);
    }

    /**
     * @brief Removes the first character from a string
     * 
     * @param str The string to be cut
     * @return The same (content-wise) string with its first character missing
     */
    inline std::string remove_first(const std::string& str)
    {
        return str.substr(1, str.size());
    }

    /**
     * @brief An exception throwable by any function
     * 
     */
    struct Exception {
        const unsigned id;
        const std::string description;
        /**
         * @brief Construct a new Exception
         * 
         * @param id Exception ID. Not interdependent, choose what you want
         * @param desc Exception description. Choose something meaningful to convey what went wrong
         */
        Exception(unsigned id, std::string desc) : id(id), description(desc) {}
    };
    /**
     * @brief Outputs an exception to an outstream
     * 
     * @param out The outstream
     * @param e The exception
     * @return The same outstream. If the function fails, the ostream will remain unchanged as demanded by the standard
     */
    std::ostream& operator<<(std::ostream& out, const Exception& e); 
    // An input file encountered a faulty format
    const Exception WRONG_FORMAT_EXCEPTION(1, std::string("Wrong format for input file."));
    // The interpreter is missing a variable in its input file
    const Exception MISSING_VARIABLE_EXCEPTION(2, std::string("Missing a required variable."));
    // The object hierarchy has a wrong format
    const Exception WRONG_OBJECT_HIERARCHY_EXCEPTION(3, std::string("Wrong object hierarchy encountered."));

    /**
     * @brief Standard 3-dimensional vector
     * 
     */
    struct Vec3 {
        // The values in the vector
        double vals[3];
        // Modifiable x value
        constexpr double& x() noexcept { return vals[0]; }
        // Non-modifiable x value
        constexpr inline const double& x() const noexcept { return vals[0]; }
        // Modifiable y value
        constexpr double& y() noexcept { return vals[1]; }
        // Non-modifiable y value
        constexpr inline const double& y() const noexcept { return vals[1]; }
        // Modifiable z value
        constexpr double& z() noexcept { return vals[2]; }
        // Non-modifiable z value
        constexpr inline const double& z() const noexcept { return vals[2]; }

        /**
         * @brief Standard constructor.
         * 
         * @param x The x value of the vector (default: 0)
         * @param y The y value of the vector (default: 0)
         * @param z The z value of the vector (default: 0)
         * 
         */
        constexpr Vec3(double x = 0., double y = 0., double z = 0.) : vals{x, y, z} {}
        /**
         * @brief Copies a vector
         * 
         * @param other The vector to be copied. Remains unchanged
         * 
         */
        constexpr Vec3(const Vec3& other) = default;
        /**
         * @brief Moves a vector
         * 
         * @param other The vector to be moved
         * 
         */
        constexpr Vec3(Vec3&& other) = default;

        /**
         * @brief Dot product
         * 
         * @param other Vector that the dot product is computed with
         * @return The dot product
         */
        constexpr inline double operator*(const Vec3& other) const noexcept
        {
            double res = 0.;
            for (unsigned i = 0; i < 3; i++) res += this->vals[i] * other.vals[i];
            return res;
        }
        /**
         * @brief Stretches a vector
         * 
         * @param x Stretch factor
         * @return The stretched vector
         */
        constexpr inline Vec3 operator*(const double& x) const noexcept
        {
            return Vec3(this->vals[0] * x, this->vals[1] * x, this->vals[2] * x);
        }
        /**
         * @brief The cross product with another vector
         * 
         * @param other The other
         * @return The vector orthogonal to this and the given vector
         */
        constexpr inline Vec3 operator%(const Vec3& other) const noexcept
        {
            return Vec3(
                this->vals[1] * other.vals[2] - this->vals[2] * other.vals[1],
                this->vals[2] * other.vals[0] - this->vals[0] * other.vals[2],
                this->vals[0] * other.vals[1] - this->vals[1] * other.vals[0]
            );
        }
        /**
         * @brief Adds two vectors
         * 
         * @param other The vector to be added
         * @return Sum of both
         */
        constexpr inline Vec3 operator+(const Vec3& other) const noexcept
        {
            return Vec3(this->vals[0] + other.vals[0], this->vals[1] + other.vals[1], this->vals[2] + other.vals[2]);
        }
        /**
         * @brief Subtracts two vectors
         * 
         * @param other The vector to be subtracted
         * @return Difference between both
         */
        constexpr inline Vec3 operator-(const Vec3& other) const noexcept
        {
            return Vec3(this->vals[0] - other.vals[0], this->vals[1] - other.vals[1], this->vals[2] - other.vals[2]);
        }
        /**
         * @brief Divides the vector by a value
         * 
         * @param other The value to be divided with
         * @return The shortened vector
         */
        constexpr inline Vec3 operator/(const double& other) const noexcept
        {
            return Vec3(this->x() / other, this->y() / other, this->z() / other);
        }

        /**
         * @brief Gives the vector length
         * 
         * @return Length of this vector
         */
        constexpr inline double len() const
        {
            return sqrt(pow(this->x(), 2.) + pow(this->y(), 2.) + pow(this->z(), 2.));
        }
        /**
         * @brief Returns the normalised version of this vector
         * 
         * @return A vector with length 1 and the same direction as this one
         */
        constexpr inline Vec3 normalise() const
        {
            return (*this) / this->len();
        }
    };
    /**
     * @brief Outputs a Vec3 to an ostream
     * 
     * @param out The outstream
     * @param vec The Vector
     * @return The same outstream. If the function fails, the ostream will remain unchanged as demanded by the standard
     */
    std::ostream& operator<<(std::ostream& out, const Vec3& vec);

    /**
     * @brief Standard 4-dimensional vector
     * 
     */
    struct Vec4 {
        // The values in the vector.
        double vals[4];
        // Modifiable x-value
        constexpr double& x() noexcept { return vals[0]; }
        // Non-modifiable x-value
        constexpr inline const double& x() const noexcept { return vals[0]; }
        // Modifiable y-value
        constexpr double& y() noexcept { return vals[1]; }
        // Non-modifiable y-value
        constexpr inline const double& y() const noexcept { return vals[1]; }
        // Modifiable z-value
        constexpr double& z() noexcept { return vals[2]; }
        // Non-modifiable z-value
        constexpr inline const double& z() const noexcept { return vals[2]; }
        // Modifiable h-value
        constexpr double& h() noexcept { return vals[3]; }
        // Non-modifiable h-value
        constexpr inline const double& h() const noexcept { return vals[3]; }

        // Standard constructor that also takes no arguments.
        /**
         * @brief Constructs a Vec4
         * 
         * @param x \
         * @param y |
         * @param z |--> The position in 4D space (default for all: 0)
         * @param h /
         * 
         */
        constexpr Vec4(double x = 0., double y = 0., double z = 0., double h = 0.) : vals{x, y, z, h} {}
        /**
         * @brief Creates a Vec4 from a Vec3. Useful for transformation for calculations with 4x4 matrices.
         * 
         * @param t The vector from which this one is constructed
         * 
         */
        constexpr Vec4(const Vec3& t) : vals{t.x(), t.y(), t.z(), 0.} {}
    };
    /**
     * @brief Outputs a Vec4 to an ostream
     * 
     * @param out The outstream
     * @param vec The Vector
     * @return The same outstream. If the function fails, the ostream will remain unchanged as demanded by the standard
     */
    std::ostream& operator<<(std::ostream& out, const Vec4& vec);

    /**
     * @brief Standard 4-by-4 matrix
     * 
     */
    struct Matrix4x4 {
    public:
        // The values of the matrix
        double mat[4][4];

        /**
         * @brief Constructs a matrix with given values
         * 
         * @param m the values in array form
         */
        Matrix4x4(double m[4][4]) {
            for (unsigned i = 0; i < 4; i++)
                for (unsigned j = 0; j < 4; j++)
                    mat[i][j] = m[i][j];
        }

        /**
         * @brief Construct an identity matrix
         * 
         * @param def The value in the diagonal. Default value creates a normal identity matrix
         * 
         */
        Matrix4x4(double def = 1.)
        {
            for (int i = 0; i < 4; i++)
                for (int j = 0; j < 4; j++)
                    mat[i][j] = i == j? def : 0.;
        }

        /**
         * @brief Multiplies two matrices
         * 
         * @param other The matrix to be multiplied with
         * @return The product of both matrices
         */
        Matrix4x4 operator*(const Matrix4x4& other) noexcept
        {
            Matrix4x4 res {0.};
            for (int i = 0; i < 4; i++)
                for (int j = 0; j < 4; j++)
                    for (int k = 0; k < 4; k++)
                        res.mat[i][j] += this->mat[i][k] * other.mat[k][j];
            return res;
        }
        /**
         * @brief Applies a transformation to a 3d vector
         * 
         * @param other The vector to be transformed
         * @return A 4D vector
         */
        constexpr Vec4 operator*(const Vec3& other) noexcept
        {
            return (*this) * Vec4(other);
        }
        /**
         * @brief Applies a transformation to a 4d vector
         * 
         * @param other The vector to be transformed
         * @return A 4D vector
         */
        constexpr Vec4 operator*(const Vec4& other) noexcept
        {
            Vec4 result;
            for (int i = 0; i < 4; i++)
                for (int j = 0; j < 4; j++)
                    result.vals[i] += this->mat[i][j] * other.vals[j];
            return result;
        }
    };
    /**
     * @brief Outputs a matrix to an ostream
     * 
     * @param out The outstream
     * @param mat The matrix
     * @return The same outstream. If the function fails, the ostream will remain unchanged as demanded by the standard
     */
    std::ostream& operator<<(std::ostream& out, const Matrix4x4& mat);

    /**
     * @brief Prints out the id and text of an exception to the user on STDOUT.
     * 
     * @param e The exception that is to be printed
     */
    void printException(const Exception& e);

    /**
     * @brief Splits a string into tokens, delimited by ' '
     * 
     * @param str The string that is to be split
     * @param v A vector that will contain the tokens extracted by this function
     */
    void split(const std::string& str, std::vector<std::string>& v);

    /**
     * @brief Converts the output as returned from OpenCL to what is required by OpenCV
     * 
     * @param other The output gained after OpenCL calculations
     * @return An array that contains the same information, but in a format usable for OpenCV and with switched R and B channels
     */
    Utility::AutoArray<uint8_t> openclMemToArray(const Memory<cl_float3>& other);

}

#ifdef DEBUG
// partially taken from https://stackoverflow.com/a/2445569/9080511
namespace {
    // invoke set_terminate as part of global constant initialization
    static const bool SET_TERMINATE = std::set_terminate([](){
        std::cerr << "Exception encountered. Exciting." << std::endl;
        std::cin.get();
        exit(EXIT_FAILURE);
    });
}
#endif
