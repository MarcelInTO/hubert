/****************************************************************************
Copyright (c) 2021 Marcel A. Samek

The Hubert library and all its components are supplied under the terms of
the open source MIT License. The text immediately below, which can also 
be found at https://opensource.org/licenses/MIT, comprises the entirety
of the license.

---------------------------------------------------------------------------

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in 
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************/

#define HUBERT_REPORT_ERROR 0

#ifndef HUBERT_H_INCLUDED
#define HUBERT_H_INCLUDED

// system dependencies

#include <cmath>
#include <cstdint>
#include <limits>

// The Hubert library uses one single namespace for everything
namespace hubert
{

/////////////////////////////////////////////////////////////////////////////
// Return codes
/////////////////////////////////////////////////////////////////////////////
enum class ResultCode : uint32_t
{
    eOk = 0,
    eDegenerate,
    eCoplanar,
    eParallel,
    eNoIntersection,
    eOverflow
};

/////////////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////////////

// Epsilon, to be used for managing floating point precision, is defined
// correctly by the system for each type.
template <typename T> 
inline T epsilon() { return std::numeric_limits<T>::epsilon();} 

// Infinity is often used by Hubert to mark invalid values
template <typename T> 
inline T infinity() { return std::numeric_limits<T>::infinity();}


/////////////////////////////////////////////////////////////////////////////
// Epsilon comparisons
/////////////////////////////////////////////////////////////////////////////

template <typename T>
inline bool isEqualScaled(T v1, T v2, T scale)
{
    T eps = scale * epsilon<T>();

    if (v1 != 0.0 && v2 != 0.0)
    {
        return std::abs(v1 - v2) / std::abs(v1) <= eps && std::abs(v1 - v2) / std::abs(v2) <= eps;
    }
    else
    {
        return std::abs(v1 - v2) <= eps;
    }
}
template <typename T>
inline bool isEqual(T v1, T v2)
{
    T eps = epsilon<T>();

    if (v1 != 0.0 && v2 != 0.0)
    {
         return std::abs(v1 - v2) / std::abs(v1) <= eps &&  std::abs(v1 - v2) / std::abs(v2) <= eps;
    }
    else
    {
        return std::abs(v1 - v2) <= eps;
   }
}

template <typename T>
inline bool isGreaterOrEqual(T v1, T v2)
{
    return (v1 > v2) || isEqual(v1, v2);
}

template <typename T>
inline bool isLessOrEqual(T v1, T v2)
{
    return (v1 < v2) || isEqual(v1, v2);
}

template <typename T>
inline T difference(T v1, T v2)
{
    return std::abs(v1 - v2);
}



/////////////////////////////////////////////////////////////////////////////
// Validation checks on primitive types
/////////////////////////////////////////////////////////////////////////////

template <typename T>
inline bool isValid(const T & v)
{
    return std::isfinite(v);
}

template <typename T>
inline bool isSubnormal(const T & v)
{
    return std::isfinite(v) && !std::isnormal(v) && v != T(0.0);
}

/////////////////////////////////////////////////////////////////////////////
// Core type definitions
/////////////////////////////////////////////////////////////////////////////

// This class is the base class for all entities.
class HubertBase
{
    public:
        HubertBase() = default;
        ~HubertBase() = default;

        bool amValid() const { return !(flags & cInvalid); }
        bool amDegenerate() const { return (flags & (cDegenerate | cInvalid)); }
        bool amSubnormal() const { return (flags & cSubnormalData); }

    protected:
        static constexpr uint64_t cInvalid =          0x00001;        // entity is invalid (and data has been set to infinity)
        static constexpr uint64_t cDegenerate =       0x00002;        // entity is degenerate (may or may not be invalid)
        static constexpr uint64_t cSubnormalData =    0x00004;        // entity is valid but one of the defining data is non-zero subnormal
        static constexpr uint64_t cValidityMask =     0x0000F;        // entity is valid but one of the defining data is non-zero subnormal

        uint64_t getValidityFlags() const { return flags & cValidityMask; }
        void setValidityFlags(uint64_t f) { flags = (flags & ~cValidityMask) | (f & cValidityMask); }

    private:
        uint64_t    flags = 0;

};

//
// Point3.
//
// Is considered valid as long as the 3 components (x, y, z) are valid. 
// 
// There are no non-validity related degenerate cases for Point3.
// 
template <typename T>
class Point3 : public HubertBase
{
    public:
        // constructors
        Point3() : Point3(T(0.0), T(0.0), T(0.0)) {}
        Point3(T inX, T inY, T inZ) { _validate(inX, inY, inZ); }
        Point3(const Point3 &) = default;
        ~Point3() = default;

        // public operators 
        inline Point3<T>& operator=(const Point3<T>&) = default;

        // public methods
        inline T x() const {return _x;}
        inline T y() const {return _y;}
        inline T z() const {return _z;}

    private:
        // private methods
        void _validate(T inX, T inY, T inZ)
        {
            _x = inX;
            _y = inY;
            _z = inZ;

            uint64_t newFlags = 0;

            if (isSubnormal(_x) || isSubnormal(_y) || isSubnormal(_z))
            {
                newFlags |= cSubnormalData;
            }
             if (!(isValid(_x) && isValid(_y) && isValid(_z)))
            {
                newFlags |= cInvalid;
            }

            setValidityFlags(newFlags);
        }

        // private data
        T   _x;
        T   _y;
        T   _z;

};

//
// Vector3.
//
// Is considered valid as long as the 3 components (x, y, z) are valid. 
// 
// There are no non-validity related degenerate cases for Vector3. Note
// that while it may be possible to construct a valid Vector3 where the
// magnitude overflows and is invalid, that is not considered a degenerate
// vector.
// 
template <typename T>
class Vector3 : public HubertBase
{
    public:
        // constructors
        Vector3() : Vector3(T(0.0), T(0.0), T(0.0)) {}
        Vector3(T inX, T inY, T inZ) { _validate(inX, inY, inZ); }
        Vector3(const Vector3 &) = default;
        ~Vector3() = default;

        // public operators
        inline Vector3<T> & operator=(const Vector3<T> &) = default;

        // public methods
        inline T x() const {return _x;}
        inline T y() const {return _y;}
        inline T z() const {return _z;}
        inline T magnitude() const {return _mag; }

    private:
        // private methods
        void _validate(T inX, T inY, T inZ)
        {
            _x = inX;
            _y = inY;
            _z = inZ;

            uint64_t newFlags = 0;

            if (isSubnormal(_x) || isSubnormal(_y) || isSubnormal(_z))
            {
                newFlags |= cSubnormalData;
            }

            if (!(isValid(_x) && isValid(_y) && isValid(_z)))
            {
                newFlags |= cInvalid;
            }

            // only do degeneracy checks if valid
            if (!(newFlags & cInvalid))
            {
                _mag = std::hypot(_x, _y, _z);
            }
            else
            {
                _mag = infinity<T>();
            }

            setValidityFlags(newFlags);
        }

        // private data
        T   _x;
        T   _y;
        T   _z;
        T   _mag;
};

//
// UnitVector3.
//
// Is considered valid as long as the 3 components (x, y, z) are valid. 
// 
// A valid UnitVector3 is considered degenerate if a valid unit vector could
// not be computed from the input vector from which the UnitVector3 
// was constructed. This condition will happen if the input vector is
// zero length, or if calculation of the unit vector overflows the 
// available range of the specified floating point type.
// 
template <typename T>
class UnitVector3 : public HubertBase
{
    public:
        // constructors
        UnitVector3() : UnitVector3(T(0.0), T(1.0), T(0.0)) {}
        UnitVector3(T inX, T inY, T inZ){ _normalizeAndValidate(inX, inY, inZ); }
        UnitVector3(const UnitVector3 &) = default;
        ~UnitVector3() = default;

        // public operators
        inline UnitVector3<T> & operator=(const UnitVector3<T> &) = default;

        // public methods
        inline T x() const {return _x;}
        inline T y() const {return _y;}
        inline T z() const {return _z;}

    private:
        // private methods
        void _normalizeAndValidate(T inX, T inY, T inZ)
        {
            _x = inX;
            _y = inY;
            _z = inZ;

            uint64_t newFlags = 0;

           if (!(isValid(_x) && isValid(_y) && isValid(_z)))
            {
                newFlags |= cInvalid;
                _mag = infinity<T>();
            }

            // only do normalization and degeneracy checks if valid
            if (!(newFlags & cInvalid))
            {
                if (isSubnormal(_x) || isSubnormal(_y) || isSubnormal(_z))
                {
                    newFlags |= cSubnormalData;
                }

                _mag = std::hypot(_x, _y, _z);
           
                if (!isValid(_mag) || isEqual(_mag, T(0.0)))
                {
                    newFlags |= cDegenerate;
                }
                else
                {
                    _x /= _mag;
                    _y /= _mag;
                    _z /= _mag;

                    // becasue we have modified the numbers, check them for subnormality again
                    if (isSubnormal(_x) || isSubnormal(_y) || isSubnormal(_z))
                    {
                        newFlags |= cSubnormalData;
                    }
                }
             }

             setValidityFlags(newFlags);
        }

        // private data
        T   _x;
        T   _y;
        T   _z;
        T   _mag;
};

template <typename T>
class Matrix3 : public HubertBase
{
    public:
        // constructors
        Matrix3() : _m{} {}
        Matrix3(T r0c0, T r0c1, T r0c2, T r1c0, T r1c1, T r1c2, T r2c0, T r2c1, T r2c2) { _validate(r0c0, r0c1, r0c2, r1c0, r1c1, r1c2, r2c0, r2c1, r2c2); }
        Matrix3(const Matrix3&) = default;
        ~Matrix3() = default;

        // public operators
        inline Matrix3<T>& operator=(const Matrix3<T>&) = default;

        // public methods
        inline T get(uint32_t r, uint32_t c) const { return _m[r][c]; }
        inline T getDeterminantEpsilonScale() const { return T(12.0) * _maxVal; }

        inline Matrix3<T> transpose(void) const
        {
            Matrix3<T> mt(
                _m[0][0], _m[1][0], _m[2][0],
                _m[0][1], _m[1][1], _m[2][1],
                _m[0][2], _m[1][2], _m[2][2]
            );

            return mt;
        }

        inline Matrix3<T> multiply(const Matrix3<T> m2) const
        {
           T mt[3][3];

            for (int i = 0; i < 3; i++){
                for (int j = 0; j < 3; j++) {
                    mt[i][j] = T(0.0);
                    for (int u = 0; u < 3; u++) {
                        mt[i][j] += _m[i][u] * m2._m[u][j];
                    }
                }
            }

            return Matrix3(mt[0][0], mt[0][1], mt[0][2], mt[1][0], mt[1][1], mt[1][2], mt[2][0], mt[2][1], mt[2][2]);
        }

        inline bool isIdentity(T scale = T(1.0)) const 
        {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (!isEqualScaled(_m[i][j], (i == j) ? T(1.0) : T(0.0), scale)) {
                        return false;
                    }
                }
            }
            return true;
        }

        inline bool isIdentityTolerance(T tolerance) const
        {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (difference(_m[i][j], (i == j) ? T(1.0) : T(0.0)) > tolerance) {
                        return false;
                    }
                }
            }
            return true;
        }

        inline T determinant() const
        {
            return (
                _m[0][0] * (_m[1][1] * _m[2][2] - _m[2][1] * _m[1][2]) -
                _m[0][1] * (_m[1][0] * _m[2][2] - _m[2][0] * _m[1][2]) +
                _m[0][2] * (_m[1][0] * _m[2][1] - _m[2][0] * _m[1][1])
            );
        }


    protected:
        T   _m[3][3];


    private:
        // private methods
        void _validate(T r0c0, T r0c1, T r0c2, T r1c0, T r1c1, T r1c2, T r2c0, T r2c1, T r2c2)
        {
            _m[0][0] = r0c0;
            _m[1][0] = r1c0;
            _m[2][0] = r2c0;
            _m[0][1] = r0c1;
            _m[1][1] = r1c1;
            _m[2][1] = r2c1;
            _m[0][2] = r0c2;
            _m[1][2] = r1c2;
            _m[2][2] = r2c2;

            // we use the maximum value to scale epsilon
            T* v = &_m[0][0];
            for (int i = 0; i < 9; i++, v++)
            {
                if (std::abs(*v) > _maxVal)
                {
                    _maxVal = std::abs(*v);
                }
            }

            uint64_t newFlags = 0;

            // only reason for invalidity is the source data
            v = &_m[0][0];
            for (int i = 0; i < 9; i++, v++)
            {
                if (!isValid(*v))
                {
                    newFlags |= cInvalid;
                    break;
                }
            }

            // only do normalization checks if we are valid. There is no
            // degeneracy case unrelated to validity.
            if (!(newFlags & cInvalid))
            {
                v = &_m[0][0];
                for (int i = 0; i < 9; i++, v++)
                {
                    if (isSubnormal(T(*v)))
                    {
                        newFlags |= cSubnormalData;
                        break;
                    }
                }
            }

            setValidityFlags(newFlags);
        }

        // Don't allow direct access to the data
        T   _maxVal = T(0.0);
};


template <typename T>
class MatrixRotation3 : public Matrix3<T>
{
    public:
        // constructors
        MatrixRotation3() : MatrixRotation3(UnitVector3<T>(T(1.0), T(0.0), T(0.0)), UnitVector3<T>(T(0.0), T(1.0), T(0.0)), UnitVector3<T>(T(0.0), T(0.0), T(1.0)) ) {}
        MatrixRotation3(const UnitVector3<T> & inX, const UnitVector3<T>& inY, const UnitVector3<T>& inZ) : Matrix3<T>(inX.x(), inX.y(), inX.z(), inY.x(), inY.y(), inY.z(), inZ.x(), inZ.y(), inZ.z()) { _validate(inX, inY, inZ); }
        MatrixRotation3(const MatrixRotation3&) = default;
        ~MatrixRotation3() = default;

        // public operators
        inline MatrixRotation3<T>& operator=(const MatrixRotation3<T>&) = default;

        // public methods
        inline MatrixRotation3<T> transpose(void) const 
        {
            MatrixRotation3<T> mt(
                UnitVector3<T>(this->_m[0][0], this->_m[1][0], this->_m[2][0]),
                UnitVector3<T>(this->_m[0][1], this->_m[1][1], this->_m[2][1]),
                UnitVector3<T>(this->_m[0][2], this->_m[1][2], this->_m[2][2])
                );

            return mt;
        }

        inline MatrixRotation3<T> multiply(const MatrixRotation3<T> m2) const
        {
            T mt[3][3];

            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    mt[i][j] = T(0.0);
                    for (int u = 0; u < 3; u++) {
                        mt[i][j] += this->_m[i][u] * m2._m[u][j];
                    }
                }
            }

            return MatrixRotation3(
                UnitVector3<T>(mt[0][0], mt[0][1], mt[0][2]),
                UnitVector3<T>(mt[1][0], mt[1][1], mt[1][2]),
                UnitVector3<T>(mt[2][0], mt[2][1], mt[2][2])
            );
        }

    private:
        // Rotation matrix specific validation
        void _validate(const UnitVector3<T>& inX, const UnitVector3<T>& inY, const UnitVector3<T>& inZ)
        {
            uint64_t newFlags = 0;

            // the validity & subnormality flags will already have been set by the parent class. 
            // All we have to worry about is degeneracy checks specific to the Rotation matrix
            if (!(newFlags & HubertBase::cInvalid))
            {
                // the incoming vectors must be legitimate unit vectors, otherwise the matrix
                // does not stand a chance of being a rotation matrix.
                if (isDegenerate(inX) || isDegenerate(inY) || isDegenerate(inZ))
                {
                    newFlags |= HubertBase::cDegenerate;
                }

                // Multiplied by its transpose must be an identity. Because we did
                // a matrix multiply we have to scale the epsilon that will be used.
                // Since we know all the columns are unit vectors, we don't worry
                // about the magnitude of the values, but rather just on the number of
                // operations performed
                if (!(newFlags & HubertBase::cDegenerate))
                {
                    // Force using the parent class so that we don't do this validation
                    // routine recursively
                    if (!(Matrix3<T>::multiply(Matrix3<T>::transpose()).isIdentityTolerance(T(.00001))))
                    {
                        newFlags |= HubertBase::cDegenerate;
                    }
                }

                // determinant must be 1
                if (!(newFlags & HubertBase::cDegenerate))
                {
                    if (!isEqualScaled(this->determinant(), T(1.0), this->getDeterminantEpsilonScale()))
                    {
                        newFlags |= HubertBase::cDegenerate;
                    }
                }
            }

            this->setValidityFlags(newFlags);
        }
};




//
// Line3.
//
// Is considered valid as long as both Point3 input values used to 
// define it are valid. 
// 
// A valid Line3 is considered degenerate if a valid unit vector could
// not be computed from the input Vector3 from which the UnitVector3 
// was constructed. This condition would happen if the input vcctor is
// zero length, or if calculation of the unit vector overflowed the 
// available range of the specified floating point type.
// 
template <typename T>
class Line3 : public HubertBase
{
    public:
        // constructors
        Line3() : Line3(Point3<T>(T(0.0), T(0.0), T(0.0)), Point3<T>(T(1.0), T(1.0), T(1.0))) {}
        Line3(const Point3<T> & inP1, const Point3<T> & inP2) { _normalizeAndValidate(inP1, inP2); }
        Line3(const Line3 &) = default;
        ~Line3() = default;

        // public operators
        inline Line3<T> & operator=(const Line3<T> &) = default;

        // public methods
        inline const Point3<T> & base() const { return _base; }
        inline const Point3<T> & target() const { return _target; }
        inline const UnitVector3<T>& unitDirection() const { return _unitDirection; }
        inline const Vector3<T>& fullDirection() const { return _fullDirection; }


    private:
        // private methods
        void _normalizeAndValidate(const Point3<T> & p1, const Point3<T> & p2)
        {
            uint64_t newFlags = 0;

            // non calculated data is preserved as is
            _base = p1;
            _target = p2;

            // only do normalization and degeneracy checks if valid
            if (!(isValid(p1) && isValid(p2)))
            {
                newFlags |= cInvalid;
                _fullDirection = Vector3<T>(infinity<T>(), infinity<T>(), infinity<T>());
                _unitDirection = UnitVector3<T>(infinity<T>(), infinity<T>(), infinity<T>());
            }
            else {
                // if subnormal, we will do the calculations, but set the subnormal flag
                // so that everyone knows that the calculations are reduced precision
                if (isSubnormal(p1) || isSubnormal(p2))
                {
                    newFlags |= cSubnormalData;
                }

                // if the two poits are too close, we are degenerate
                if (isEqual(distance(p1, p2), T(0.0)))
                {
                    newFlags |= cDegenerate;
                    _fullDirection = Vector3<T>(infinity<T>(), infinity<T>(), infinity<T>());
                    _unitDirection = UnitVector3<T>(infinity<T>(), infinity<T>(), infinity<T>());
                }
                else
                {
                    _base = p1;
                    _target = p2;
                    _fullDirection = Vector3<T>(p2.x() - p1.x(), p2.y() - p1.y(), p2.z() - p1.z());
                    _unitDirection = makeUnitVector3(_fullDirection);

                    if (isDegenerate(_fullDirection) || isDegenerate(_unitDirection))
                    {
                        // if the vector calculation overflows, call the line degenerate because
                        // there is not much we can do with it since the points are too far apart
                        // to use in any calculation
                        newFlags |= cDegenerate;
                    }
                }
            }

            setValidityFlags(newFlags);
        }
        
        Point3<T>       _base;
        Point3<T>       _target;
        Vector3<T>  _fullDirection;
        UnitVector3<T>  _unitDirection;
};

//
// Plane.
//
// Is considered valid as long as the base Point3 input values and the
// up UnitVector3 used to define it are valid
// 
// A valid Plane is considered degenerate if a non-degenerate up UnitVector3
// is not provided.
// 
template <typename T>
class Plane : public HubertBase
{
    public:
        // constructors
        Plane() : Plane(Point3<T>(T(0.0), T(0.0), T(0.0)), UnitVector3<T>(T(0.0), T(0.0), T(1.0))) {}
        Plane(const Point3<T> & p, const UnitVector3<T>& v) { _validate(p, v);  }
        Plane(const Plane &) = default;
        ~Plane() = default;

        // public methods
        inline Plane<T> & operator=(const Plane<T> &) = default;

        // public medthods
        inline const Point3<T>& base() const { return _base; }
        inline const UnitVector3<T>& up() const { return _up; }

    private:
        void _validate(const Point3<T>& p, const UnitVector3<T>& v)
        {
            _base = p;
            _up = v;

            uint64_t newFlags = 0;

            // only do normalization and degeneracy checks if valid
            if (!(isValid(_base) && isValid(_up)))
            {
                newFlags |= cInvalid;
            }
            else {
                // if subnormal, we will do the calculations, but set the subnormal flag
                // so that everyone knows that the calculations are reduced precision
                if (isSubnormal(_base) || isSubnormal(_up))
                {
                    newFlags |= cSubnormalData;
                }

                if (isDegenerate(_up))
                {
                    newFlags |= cDegenerate;
                }
            }

            setValidityFlags(newFlags);
        }


        // public data
        Point3<T>       _base;
        UnitVector3<T>  _up;
};

//
// Ray3.
//
// Is considered valid as long as the base Point3 input values and the
// direction UnitVector3 used to define it are valid
// 
// A valid Ray3 is considered degenerate if a non-degenerate direction UnitVector3
// is not provided.
// 
template <typename T>
class Ray3 : public HubertBase
{
    public:
        // constructors
        Ray3() : Ray3(Point3<T>(T(0.0), T(0.0), T(0.0)), UnitVector3<T>(T(0.0), T(0.0), T(1.0))) {}
        Ray3(const Point3<T>& p, const UnitVector3<T>& v) { _validate(p, v); }
        Ray3(const Ray3 &) = default;
        ~Ray3() = default;

        // public operators
        inline Ray3<T> & operator=(const Ray3<T> &) = default;

        // public medthods
        inline const Point3<T>& base() const { return _base; }
        inline const UnitVector3<T> & unitDirection() const { return _direction; }

    private:
        void _validate(const Point3<T>& p, const UnitVector3<T>& v)
        {
            _base = p;
            _direction = v;

            uint64_t newFlags = 0;

            // only do normalization and degeneracy checks if valid
            if (!(isValid(_base) && isValid(_direction)))
            {
                newFlags |= cInvalid;
            }
            else {
                // if subnormal, we will do the calculations, but set the subnormal flag
                // so that everyone knows that the calculations are reduced precision
                if (isSubnormal(_base) || isSubnormal(_direction))
                {
                    newFlags |= cSubnormalData;
                }

                if (isDegenerate(_direction))
                {
                    newFlags |= cDegenerate;
                }
            }

            setValidityFlags(newFlags);
        }

        Point3<T>       _base;
        UnitVector3<T>  _direction;
};

//
// Segment3.
//
// Is considered valid as long as both Point3 input values used to 
// define it are valid. 
// 
// A valid Segment3 is considered degenerate if a valid unit vector could
// not be computed from the input Vector3 from which the UnitVector3 
// was constructed. This condition would happen if the input vector is
// zero length, or if calculation of the unit vector overflowed the 
// available range of the specified floating point type.
// 
template <typename T>
class Segment3 : public HubertBase
{
    public:
        // constructors
        Segment3() : Segment3(Point3<T>(T(0.0), T(0.0), T(0.0)), Point3<T>(T(1.0), T(1.0), T(1.0))) {}
        Segment3(const Point3<T>& inP1, const Point3<T>& inP2) { _validate(inP1, inP2); }
        Segment3(const Segment3 &) = default;
        ~Segment3() = default;

        // public operators
        inline Segment3<T> & operator=(const Segment3<T> &) = default;

        // public methods
        inline const Point3<T> & base() const { return _base; }
        inline const Point3<T> & target() const { return _target; }

    private:
        void _validate(const Point3<T>& p1, const Point3<T>& p2)
        {
            _base = p1;
            _target = p2;

            uint64_t newFlags = 0;

            // only do subnormal and degeneracy checks if valid
            if (!(isValid(_base) && isValid(_target)))
            {
                newFlags |= cInvalid;
            }
            else {
                // if subnormal, we will do the calculations, but set the subnormal flag
                // so that everyone knows that the calculations are reduced precision
                if (isSubnormal(_base) || isSubnormal(_target))
                {
                    newFlags |= cSubnormalData;
                }

                T dist = distance(p1, p2);
                if (isEqual(dist, T(0.0)) || !isValid(dist))
                {
                    newFlags |= cDegenerate;
                }
            }

            setValidityFlags(newFlags);
        }

        //private data
        Point3<T>   _base;
        Point3<T>   _target;
};

//
// Triangle3.
//
// Is considered valid as long as all 3 Point3 input values used to 
// define it are valid. 
// 
// A valid Triangle3 is considered degenerate For a number of reasons:
//    * Any of the 3 edges is collapsed to 0 length
//    * Any of the 3 edges overflows when its length is computed
//    * Are of the triangle is computed to be 0
//    * Cross product of edge1 & edge2 creates a vector of 0 length
//    * Cross product of edge1 & edge3 creates a vector of 0 length
//    * Cross product of edge2 & edge3 creates a vector of 0 length
// 
// 
template <typename T>
class Triangle3 : public HubertBase
{
    public:
        // constructors
        Triangle3() : Triangle3(Point3<T>(T(0.0), T(0.0), T(0.0)), Point3<T>(T(1.0), T(0.0), T(0.0)), Point3<T>(T(0.0), T(1.0), T(0.0))) {}
        Triangle3(const Point3<T>& inP1, const Point3<T>& inP2, const Point3<T>& inP3) { _validate(inP1, inP2, inP3); }
        Triangle3(const Triangle3 &) = default;
        ~Triangle3() = default;

        // public operators
        inline Triangle3<T> & operator=(const Triangle3<T> &) = default;

        // public methods
        inline const Point3<T>& p1() const { return _p1; }
        inline const Point3<T>& p2() const { return _p2; }
        inline const Point3<T>& p3() const { return _p3; }

    private:
        void _validate(const Point3<T>& p1, const Point3<T>& p2, const Point3<T>& p3)
        {
            _p1 = p1;
            _p2 = p2;
            _p3 = p3;

            uint64_t newFlags = 0;

            // only do subnormal and degeneracy checks if valid
            if (!(isValid(_p1) && isValid(_p2) && isValid(_p3)))
            {
                newFlags |= cInvalid;
            }
            else {
                // if subnormal, we will do the calculations, but set the subnormal flag
                // so that everyone knows that the calculations are reduced precision
                if (isSubnormal(_p1) || isSubnormal(_p2) || isSubnormal(_p3))
                {
                    newFlags |= cSubnormalData;
                }

                // we do multiple degeneracy tests because we need to fail if any of them fail. If we did not
                // other calculations down stream might fail because we reported a non-degenerate triangle. 
                // Because of numerical  instability depending on the scale of the numbers, it is possible for 
                // some approaches to differ in their response.

                // first degeneracy test - collapsed edge or overflow edge. Overflow edges basically mean that
                // even though the points are valid, the length of a side overflows what is representable in the
                // floating point unit being used. Allowing that to be considered a non-degenerate triangle causes
                // many downstream calculations to fail, thus prompting our decision to call it degenerate.
                T dist1 = distance(_p1, _p2);
                T dist2 = distance(_p2, _p3);
                T dist3 = distance(_p3, _p1);
                if (isEqual(dist1, T(0.0)) || !isValid(dist1) || isEqual(dist2, T(0.0)) || !isValid(dist2) || isEqual(dist3, T(0.0)) || !isValid(dist3))
                {
                    newFlags |= cDegenerate;
                }

                // 2nd degeneracy check - the area
                if (!(newFlags & cDegenerate))
                {
                    if (isEqual(area(*this), T(0.0)))
                    {
                        newFlags |= cDegenerate;
                    }
                }

                // 3rd degneracy check - cross product one way
                if (!(newFlags & cDegenerate))
                {
                    Vector3<T> v = crossProduct(p2 - p1, p3 - p1);
                    if (isEqual(v.x(), T(0.0)) && isEqual(v.y(), T(0.0)) && isEqual(v.z(), T(0.0)))
                    {
                        newFlags |= cDegenerate;
                    }
                }
                // 3rd degneracy check - cross product second way
                if (!(newFlags & cDegenerate))
                {
                    Vector3<T> v = crossProduct(p2 - p1, p3 - p2);
                    if (isEqual(v.x(), T(0.0)) && isEqual(v.y(), T(0.0)) && isEqual(v.z(), T(0.0)))
                    {
                        newFlags |= cDegenerate;
                    }
                }

                // 3rd degneracy check - cross product third way
                if (!(newFlags & cDegenerate))
                {
                    Vector3<T> v = crossProduct(p3 - p1, p3 - p2);
                    if (isEqual(v.x(), T(0.0)) && isEqual(v.y(), T(0.0)) && isEqual(v.z(), T(0.0)))
                    {
                        newFlags |= cDegenerate;
                    }
                }

            }

            setValidityFlags(newFlags);
        }

        //private data
        Point3<T>   _p1;
        Point3<T>   _p2;
        Point3<T>   _p3;
};

/////////////////////////////////////////////////////////////////////////////
// Special invalid entity instances
/////////////////////////////////////////////////////////////////////////////

template <typename T>
inline T invalidValue()
{
    return infinity<T>();
}

template <typename T>
inline Point3<T> invalidPoint3()
{
    return Point3<T>(infinity<T>(), infinity<T>(), infinity<T>());
}

template <typename T>
inline Vector3<T> invalidVector3()
{
    return Vector3<T>(infinity<T>(), infinity<T>(), infinity<T>());
}

template <typename T>
inline UnitVector3<T> invalidUnitVector3()
{
    return UnitVector3<T>(infinity<T>(), infinity<T>(), infinity<T>());
}

/////////////////////////////////////////////////////////////////////////////
// Creation functions - (alternates to provided constructor)
/////////////////////////////////////////////////////////////////////////////

template <typename T>
inline Vector3<T> makeVector3(const Point3<T> & from, const Point3<T> to)
{
    return Vector3<T>(to.x() - from.x(), to.y() - from.y(), to.z() - from.z());
}

template <typename T>
inline Vector3<T> makeVector3(const UnitVector3<T> & u)
{
    return Vector3<T>(u.x(), u.y(), u.z());
}

template <typename T>
inline UnitVector3<T> makeUnitVector3(const Vector3<T> & v)
{
    return UnitVector3<T>(v.x(), v.y(), v.z());
}

template <typename T>
inline UnitVector3<T> makeUnitVector3(const Point3<T> & from, const Point3<T> to)
{
    return UnitVector3<T>(to.x() - from.x(), to.y() - from.y(), to.z() - from.z());
}


template <typename T>
inline Line3<T> makeLine3(const Point3<T> & p, const Vector3<T> v)
{
    return Line3<T>(p, p + v);
}

template <typename T>
inline Line3<T> makeLine3(const Point3<T> & p, const UnitVector3<T> v)
{
    return Line3<T>(p, p + makeVector3(v));
}


template <typename T>
inline Plane<T> makePlane( const Point3<T> & p1, const Point3<T> & p2, const Point3<T> & p3)
{
    return Plane<T>(p1, makeUnitVector3(crossProduct(p2 - p1, p3 - p1)));
}

template <typename T>
inline Ray3<T> makeRay3(const Point3<T> & p1, const Point3<T> & p2)
{
    return Ray3<T>(p1, makeUnitVector3(p2 - p1));
}


/////////////////////////////////////////////////////////////////////////////
// Validity checks
//
// These are checks for numerical validity, rather than any sort of
// contextual checks. They simly care whether the floating point 
// numbers are valid.
/////////////////////////////////////////////////////////////////////////////

template <typename T>
inline bool isValid(const Point3<T> & v)
{
    return v.amValid();
}

template <typename T>
inline bool isValid(const Vector3<T> & v)
{
    return v.amValid();
}

template <typename T>
inline bool isValid(const UnitVector3<T> & v)
{
    return v.amValid();
}

template <typename T>
inline bool isValid(const Matrix3<T>& v)
{
    return v.amValid();
}

template <typename T>
inline bool isValid(const MatrixRotation3<T>& v)
{
    return v.amValid();
}

template <typename T>
inline bool isValid(const Line3<T> & v)
{
    return v.amValid();
}

template <typename T>
inline bool isValid(const Plane<T>& v)
{
    return v.amValid();
}

template <typename T>
inline bool isValid(const Ray3<T>& v)
{
    return v.amValid();
}

template <typename T>
inline bool isValid(const Segment3<T>& v)
{
    return v.amValid();
}

template <typename T>
inline bool isValid(const Triangle3<T>& v)
{
    return v.amValid();
}


/////////////////////////////////////////////////////////////////////////////
// Degeneracy checks
//
// These are checks for degenerate forms of the entities we support. This
// is a superset of the validity checks in that if one of the components
// of an entity is invalid. Degeneracy, however, by no means implies that
// andy of the individual components are invalid.
/////////////////////////////////////////////////////////////////////////////

template <typename T>
inline bool isDegenerate(const Point3<T> & v)
{
     return v.amDegenerate();
}

template <typename T>
inline bool isDegenerate(const Vector3<T> & v)
{
   return v.amDegenerate();
}

template <typename T>
inline bool isDegenerate(const UnitVector3<T> & v)
{
   return v.amDegenerate();
}

template <typename T>
inline bool isDegenerate(const Matrix3<T>& v)
{
    return v.amDegenerate();
}

template <typename T>
inline bool isDegenerate(const MatrixRotation3<T>& v)
{
    return v.amDegenerate();
}

template <typename T>
inline bool isDegenerate(const Line3<T> & v)
{
    return v.amDegenerate();
}

template <typename T>
inline bool isDegenerate(const Plane<T> & v)
{
    return v.amDegenerate();
}

template <typename T>
inline bool isDegenerate(const Triangle3<T> & v)
{
    return v.amDegenerate();
}

template <typename T>
inline bool isDegenerate(const Segment3<T> & v)
{
    return v.amDegenerate();
}

template <typename T>
inline bool isDegenerate(const Ray3<T>& v)
{
    return v.amDegenerate();
}

/////////////////////////////////////////////////////////////////////////////
// Subnormal checks
//
// 
/////////////////////////////////////////////////////////////////////////////

template <typename T>
inline bool isSubnormal(const Point3<T> & v)
{
    return v.amSubnormal();
}

template <typename T>
inline bool isSubnormal(const Vector3<T> & v)
{
    return v.amSubnormal();
}

template <typename T>
inline bool isSubnormal(const UnitVector3<T> & v)
{
    return v.amSubnormal();
}

template <typename T>
inline bool isSubnormal(const Matrix3<T>& v)
{
    return v.amSubnormal();
}

template <typename T>
inline bool isSubnormal(const MatrixRotation3<T>& v)
{
    return v.amSubnormal();
}

template <typename T>
inline bool isSubnormal(const Line3<T> & v)
{
    return v.amSubnormal();
}

template <typename T>
inline bool isSubnormal(const Plane<T>& v)
{
    return v.amSubnormal();
}

template <typename T>
inline bool isSubnormal(const Ray3<T>& v)
{
    return v.amSubnormal();
}

template <typename T>
inline bool isSubnormal(const Segment3<T>& v)
{
    return v.amSubnormal();
}

template <typename T>
inline bool isSubnormal(const Triangle3<T>& v)
{
    return v.amSubnormal();
}


/////////////////////////////////////////////////////////////////////////////
// Vector Math
/////////////////////////////////////////////////////////////////////////////

// dot product

template <typename T>
inline T dotProduct(const Vector3<T> & v1, const Vector3<T> & v2)
{
    if (v1.amDegenerate() || v2.amDegenerate())
    {
        return infinity<T>();
    }
    return v1.x() * v2.x() + v1.y() * v2.y() + v1.z() * v2.z();
}

template <typename T>
inline T dotProduct(const UnitVector3<T> & v1, const Vector3<T> & v2)
{
    if (v1.amDegenerate() || v2.amDegenerate())
    {
        return infinity<T>();
    }
    return v1.x() * v2.x() + v1.y() * v2.y() + v1.z() * v2.z();
}

template <typename T>
inline T dotProduct(const Vector3<T> & v1, const UnitVector3<T> & v2)
{
    if (v1.amDegenerate() || v2.amDegenerate())
    {
        return infinity<T>();
    }
    return v1.x() * v2.x() + v1.y() * v2.y() + v1.z() * v2.z();
}

template <typename T>
inline T dotProduct(const UnitVector3<T> & v1, const UnitVector3<T> & v2)
{
    if (v1.amDegenerate() || v2.amDegenerate())
    {
        return infinity<T>();
    }
    return v1.x() * v2.x() + v1.y() * v2.y() + v1.z() * v2.z();
}

// scalar multiply

template <typename T>
inline Vector3<T> multiply(const Vector3<T>  &v , T m)
{
    return Vector3<T>(v.x() * m, v.y() * m, v.z() * m);
}

template <typename T>
inline Vector3<T> operator*(const Vector3<T>& v, T m)
{
    return multiply(v, m);
}

template <typename T>
inline Vector3<T> multiply(const UnitVector3<T> & v, T m)
{
    return Vector3<T>(v.x() * m, v.y() * m, v.z() * m);
}

template <typename T>
inline Vector3<T> operator*(const UnitVector3<T>& v, T m)
{
    return multiply(v, m);
}

// cross product

template <typename T>
inline Vector3<T> crossProduct(const Vector3<T>& v1, const Vector3<T>& v2)
{
    if (v1.amDegenerate() || v2.amDegenerate())
    {
        return invalidVector3<T>();
    }

    return Vector3<T>(
        v1.y() * v2.z() - v1.z() * v2.y(),
        v1.z() * v2.x() - v1.x() * v2.z(),
        v1.x() * v2.y() - v1.y() * v2.x()
    );
}

template <typename T>
inline Vector3<T> crossProduct(const Vector3<T>& v1, const UnitVector3<T>& v2)
{
    if (v1.amDegenerate() || v2.amDegenerate())
    {
        return invalidVector3<T>();
    }

    return Vector3<T>(
        v1.y() * v2.z() - v1.z() * v2.y(),
        v1.z() * v2.x() - v1.x() * v2.z(),
        v1.x() * v2.y() - v1.y() * v2.x()
        );
}

template <typename T>
inline Vector3<T> crossProduct(const UnitVector3<T>& v1, const Vector3<T>& v2)
{
    if (v1.amDegenerate() || v2.amDegenerate())
    {
        return invalidVector3<T>();
    }

    return Vector3<T>(
        v1.y() * v2.z() - v1.z() * v2.y(),
        v1.z() * v2.x() - v1.x() * v2.z(),
        v1.x() * v2.y() - v1.y() * v2.x()
        );
}

template <typename T>
inline Vector3<T> crossProduct(const UnitVector3<T>& v1, const UnitVector3<T>& v2)
{
    if (v1.amDegenerate() || v2.amDegenerate())
    {
        return invalidVector3<T>();
    }

    return Vector3<T>(
        v1.y() * v2.z() - v1.z() * v2.y(),
        v1.z() * v2.x() - v1.x() * v2.z(),
        v1.x() * v2.y() - v1.y() * v2.x()
        );
}

/////////////////////////////////////////////////////////////////////////////
// Distance functions
/////////////////////////////////////////////////////////////////////////////

template <typename T>
inline T distance(const Point3<T> & p1, const Point3<T> & p2)
{
    T dx = p1.x() - p2.x();
    T dy = p1.y() - p2.y();
    T dz = p1.z() - p2.z();

    T tot = std::hypot(dx, dy, dz);

    return tot;
 }

template <typename T>
inline T distance(const Point3<T> & thePoint, const Plane<T> & thePlane)
{
    return dotProduct(thePlane.up(), thePoint - thePlane.base());
}

template <typename T>
inline T distance(const Plane<T> & thePlane, const Point3<T> & thePoint)
{
    return distance(thePoint, thePlane);
}

/////////////////////////////////////////////////////////////////////////////
// vector magnitude functions
/////////////////////////////////////////////////////////////////////////////

template <typename T>
inline T magnitude(const Vector3<T>& v)
{
    if (!isValid(v))
    {
        return infinity<T>();
    }

    return std::hypot(v.x(), v.y(), v.z());
}

template <typename T>
inline T magnitude(const UnitVector3<T>& v)
{
    if (v.amDegenerate())
    {
        if (v.amValid())
        {
            return T(0.0);
        }
        else
        {
            return infinity<T>();
        }
    }
    return T(1.0);
}

/////////////////////////////////////////////////////////////////////////////
// Transforms
/////////////////////////////////////////////////////////////////////////////

template <typename T>
inline Matrix3<T> transpose(const Matrix3<T>& v)
{
    return v.transpose();
}

template <typename T>
inline MatrixRotation3<T> transpose(const MatrixRotation3<T>& v)
{
    return v.transpose();
}

template <typename T>
inline Matrix3<T> multiply(const Matrix3<T>& v1, const Matrix3<T>& v2)
{
    return v1.multiply(v2);
}

// two valid rotation matrices are guaranteed to result in a rotation matrix
template <typename T>
inline MatrixRotation3<T> multiply(const MatrixRotation3<T> & v1, const MatrixRotation3<T> & v2)
{
    return v1.multiply(v2);
}

template <typename T>
inline Vector3<T> multiply(const Vector3<T>& v, const Matrix3<T>& m)
{
    Vector3<T> ret(
        v.x() * m.get(0, 0) + v.y() * m.get(1, 0) + v.z() * m.get(2, 0),
        v.x() * m.get(0, 1) + v.y() * m.get(1, 1) + v.z() * m.get(2, 1),
        v.x() * m.get(0, 2) + v.y() * m.get(1, 2) + v.z() * m.get(2, 2)
    );

    return ret;
}


/////////////////////////////////////////////////////////////////////////////
// normal functions
/////////////////////////////////////////////////////////////////////////////

template <typename T>
inline UnitVector3<T> unitNormal(const Triangle3<T> & tri)
{
    // degenerate triangles do not have normals
    if (isDegenerate(tri))
    {
        return invalidUnitVector3<T>();
    }

    // Ordering of points is not arbitrary. This follows from the hubert
    // convention for a triangle
    Vector3<T> v1 = tri.p2() - tri.p1();
    Vector3<T> v2 = tri.p3() - tri.p1();

    Vector3<T> norm = crossProduct(v1, v2);
    if (!isValid(norm))
    {
        return invalidUnitVector3<T>();
    }

    // the unit normal could be degenerate (e.g. zero length or overflow)
    // but it is up to the caller to detect that and deal with it.
    UnitVector3<T> unorm = makeUnitVector3(norm);

    return unorm;
}


/////////////////////////////////////////////////////////////////////////////
// Closest point functions
/////////////////////////////////////////////////////////////////////////////

template <typename T>
inline Point3<T> closestPoint(const Line3<T> & theLine, const Point3<T> & thePoint)
{
    Vector3<T> v2 = makeVector3(theLine.base(), thePoint);

    T f = dotProduct(theLine.unitDirection(), v2) ;

    return theLine.base() + multiply(theLine.unitDirection(), f);
}

template <typename T>
inline Point3<T> closestPoint(const Plane<T>& thePlane, const Point3<T>& thePoint)
{
    Vector3<T> v = makeVector3(thePlane.base(), thePoint);
    T distance = dotProduct(v, thePlane.up());
    return (thePoint - multiply(thePlane.up(), distance));
}

/////////////////////////////////////////////////////////////////////////////
// Addition functions
/////////////////////////////////////////////////////////////////////////////

// Vector3 + Vector3 -> Vector3

template <typename T>
inline Vector3<T> add(const Vector3<T> & v1, const Vector3<T> & v2)
{
    return Vector3<T>(v1.x() + v2.x(), v1.y() + v2.y(), v1.z() + v2.z());
}

template <typename T>
inline Vector3<T> operator+(const Vector3<T> & v1, const Vector3<T> & v2)
{
    return add(v1, v2);
}

// Point3 + Vector3 -> Point3

template <typename T>
inline Point3<T> add(const Point3<T> & p1, const Vector3<T> & v1)
{
    return Point3<T>(v1.x() + p1.x(), v1.y() + p1.y(), v1.z() + p1.z());
}

template <typename T>
inline Point3<T> operator+(const Point3<T> & p1, const Vector3<T> & v1)
{
    return add(p1, v1);
}

/////////////////////////////////////////////////////////////////////////////
// Subtraction functions
/////////////////////////////////////////////////////////////////////////////

// Vector3 - Vector3 -> Vector3

template <typename T>
inline Vector3<T> subtract(const Vector3<T> & v1, const Vector3<T> & v2)
{
    return Vector3<T>(v1.x() - v2.x(), v1.y() - v2.y(), v1.z() - v2.z());
}

template <typename T>
inline Vector3<T> operator-(const Vector3<T> & v1, const Vector3<T> & v2)
{
    return subtract(v1, v2);
}

// Point3 - Vector3 -> Point3

template <typename T>
inline Point3<T> subtract(const Point3<T> & v1, const Vector3<T> & v2)
{
    return Point3<T>(v1.x() - v2.x(), v1.y() - v2.y(), v1.z() - v2.z());
}

template <typename T>
inline Point3<T> operator-(const Point3<T> & v1, const Vector3<T> & v2)
{
    return subtract(v1, v2);
}

// Point3 - Point3 -> Vector3

template <typename T>
inline Vector3<T> subtract(const Point3<T> & v1, const Point3<T> & v2)
{
    return Vector3<T>(v1.x() - v2.x(), v1.y() - v2.y(), v1.z() - v2.z());
}

template <typename T>
inline Vector3<T> operator-(const Point3<T> & v1, const Point3<T> & v2)
{
    return subtract(v1, v2);
}

/////////////////////////////////////////////////////////////////////////////
// area functions
/////////////////////////////////////////////////////////////////////////////

template <typename T>
inline T area(const Triangle3<T>& tri)
{
    // if we are invalid due to an invalid nubmer, there is nothing we can do.
    // However the same is not true if we are degenerate. A degenerate triangle
    // that is valid has an area of 0.
    if (!isValid(tri))
    {
        return infinity<T>();
    }
    if (isDegenerate(tri))
    {
        return T(0.0);
    }

    T dx1 = tri.p2().x() - tri.p1().x();
    T dy1 = tri.p2().y() - tri.p1().y();
    T dz1 = tri.p2().z() - tri.p1().z();

    T dx2 = tri.p3().x() - tri.p1().x();
    T dy2 = tri.p3().y() - tri.p1().y();
    T dz2 = tri.p3().z() - tri.p1().z();

    T cx = dy1 * dz2 - dy2 * dz1;
    T cy = dx2 * dz1 - dx1 * dz2;
    T cz = dx1 * dy2 - dx2 * dy1;

    // the above calculations can overflow
    if (!(isValid(cx) && isValid(cy) && isValid(cz)))
    {
        return infinity<T>();
    }

    // hypot() will return inifinity if it overflows
    return std::hypot(cx, cy, cz) * T(0.5);
}

/////////////////////////////////////////////////////////////////////////////
// centroid functions
/////////////////////////////////////////////////////////////////////////////

template <typename T>
inline Point3<T> centroid(const Triangle3<T>& tri)
{
    // if we are invalid due to an invalid nubmer, there is nothing we can do.
    // However the same is not true if we are degenerate. A degenerate triangle
    // will  have a valid centroid which we just calculate.
    if (!isValid(tri))
    {
        return invalidPoint3<T>();
    }

    return Point3<T>((tri.p1().x() + tri.p2().x() + tri.p3().x()) / 3.0, (tri.p1().y() + tri.p2().y() + tri.p3().y()) / 3.0, (tri.p1().z() + tri.p2().z() + tri.p3().z()) / 3.0);
}

/////////////////////////////////////////////////////////////////////////////
// intersection functions
/////////////////////////////////////////////////////////////////////////////

template <typename T>
inline ResultCode intersect(const Plane<T> & thePlane, const Line3<T> & theLine, Point3<T> & intersection)
{
    // Check for degnerate inputs
    if (isDegenerate(thePlane) || isDegenerate(theLine))
    {
        intersection = invalidPoint3<T>();
        return ResultCode::eDegenerate;
    }

    // check for parallel or coplanar
    T dp = dotProduct(theLine.unitDirection(), thePlane.up());
    if (isEqual(dp, T(0.0)))
    {
        if (isEqual(distance(theLine.base(), thePlane), T(0.0)))
        {
            intersection = invalidPoint3<T>();
            return ResultCode::eCoplanar;
        }
        else
        {
            intersection = invalidPoint3<T>();
            return ResultCode::eParallel;
        }
    }

    // inputs are valid and we are not parallel
    // from https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection
    T d = dotProduct(thePlane.base() - theLine.base(), thePlane.up()) / dotProduct(theLine.unitDirection(), thePlane.up());
    intersection = theLine.base() + multiply(theLine.unitDirection(), d);

    if (!isValid(intersection))
    {
        return ResultCode::eOverflow;
    }

    return ResultCode::eOk;
}

template <typename T>
inline ResultCode intersect(const Line3<T> & theLine, const Plane<T> & thePlane, Point3<T> & intersection)
{
    return intersect(thePlane, theLine, intersection);
}


template <typename T>
inline ResultCode intersect(const Plane<T>& thePlane, const Ray3<T>& theRay, Point3<T>& intersection)
{
    // Check for degnerate inputs
    if (isDegenerate(thePlane) || isDegenerate(theRay))
    {
        intersection = invalidPoint3<T>();
        return ResultCode::eDegenerate;
    }

    // check for parallel or coplanar
    T dp = dotProduct(theRay.unitDirection(), thePlane.up());
    if (isEqual(dp, T(0.0)))
    {
        if (isEqual(distance(theRay.base(), thePlane), T(0.0)))
        {
            intersection = invalidPoint3<T>();
            return ResultCode::eCoplanar;
        }
        else
        {
            intersection = invalidPoint3<T>();
            return ResultCode::eParallel;
        }
    }

    // inputs are valid and we are not parallel
    // from https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection
    T d = dotProduct(thePlane.base() - theRay.base(), thePlane.up()) / dotProduct(theRay.unitDirection(), thePlane.up());

    // 0.0 would be touching, which is an interseciton, so we don't need to do a epsilon compare
    if (d < 0.0)
    {
        intersection = invalidPoint3<T>();
        return ResultCode::eNoIntersection;
    }

    intersection = theRay.base() + multiply(theRay.unitDirection(), d);

    if (!isValid(intersection))
    {
        return ResultCode::eOverflow;
    }

    return ResultCode::eOk;
}

template <typename T>
inline ResultCode intersect(const Ray3<T>& theRay, const Plane<T>& thePlane, Point3<T>& intersection)
{
    return intersect(thePlane, theRay, intersection);
}


template <typename T>
inline ResultCode intersect(const Plane<T>& thePlane, const Segment3<T>& theSegment, Point3<T>& intersection)
{
    // Check for degnerate inputs
    if (isDegenerate(thePlane) || isDegenerate(theSegment))
    {
        intersection = invalidPoint3<T>();
        return ResultCode::eDegenerate;
    }

    UnitVector3<T> segDir = makeUnitVector3(theSegment.target() - theSegment.base());

    // check for parallel or coplanar
    T dp = dotProduct(segDir, thePlane.up());
    if (isEqual(dp, T(0.0)))
    {
        if (isEqual(distance(theSegment.base(), thePlane), T(0.0)))
        {
            intersection = invalidPoint3<T>();
            return ResultCode::eCoplanar;
        }
        else
        {
            intersection = invalidPoint3<T>();
            return ResultCode::eParallel;
        }
    }

    // inputs are valid and we are not parallel
    // from https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection
    T d = dotProduct(thePlane.base() - theSegment.base(), thePlane.up()) / dotProduct(segDir, thePlane.up());

    // 0.0 would be touching, which is an interseciton, so we don't need to do a epsilon compare
    if (d < 0.0 || d > distance(theSegment.base(), theSegment.target()))
    {
        intersection = invalidPoint3<T>();
        return ResultCode::eNoIntersection;
    }

    intersection = theSegment.base() + multiply(segDir, d);

    if (!isValid(intersection))
    {
        return ResultCode::eOverflow;
    }

    return ResultCode::eOk;
}

template <typename T>
inline ResultCode intersect(const Segment3<T>& theSegment, const Plane<T>& thePlane, Point3<T>& intersection)
{
    return intersect(thePlane, theSegment, intersection);
}


template <typename T>
inline ResultCode intersect(const Triangle3<T> & theTri,  const Ray3<T> & theRay, Point3<T> & intersection)
{
    // Check for degnerate inputs
    if (isDegenerate(theTri) || isDegenerate(theRay))
    {
        intersection = invalidPoint3<T>();
        return ResultCode::eDegenerate;
    }


    // from Moller: https://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/raytri/raytri.c

    Vector3<T> edge1 = theTri.p2() - theTri.p1();
    Vector3<T> edge2 = theTri.p3() - theTri.p1();

    Vector3<T> pvec = crossProduct(theRay.unitDirection(), edge2);

    T det = dotProduct(edge1, pvec);
    if (isEqual(det, T(0.0)))
    {
        intersection = invalidPoint3<T>();
        return ResultCode::eCoplanar;
    }

    Vector3<T> tvec = theRay.base() - theTri.p1();

    T u = dotProduct(tvec, pvec) / det;
    if (!(isGreaterOrEqual(u, T(0.0)) && isLessOrEqual(u, T(1.0))))
    {
        intersection = invalidPoint3<T>();
        return ResultCode::eNoIntersection;
    }

    Vector3<T> qvec = crossProduct(tvec, edge1);

    T v =  dotProduct(theRay.unitDirection(), qvec) / det;
    if (!(isGreaterOrEqual(v, T(0.0)) && isLessOrEqual(u + v, T(1.0))))
    {
        intersection = invalidPoint3<T>();
        return ResultCode::eNoIntersection;
    }

    T t = dotProduct(edge2, qvec) / det;
    if (!isGreaterOrEqual(t, T(0.0)))
    {
        intersection = invalidPoint3<T>();
        return ResultCode::eNoIntersection;
    }

    intersection = theRay.base() + multiply(theRay.unitDirection(), t);

    if (!isValid(intersection))
    {
        return ResultCode::eOverflow;
    }

    return ResultCode::eOk;
}

template <typename T>
inline ResultCode intersect(const Ray3<T> & theRay, const Triangle3<T> & theTri, Point3<T> & intersection)
{
    return intersect(theTri, theRay, intersection);
}

template <typename T>
inline ResultCode intersect(const Triangle3<T> & theTri,  const Line3<T> & theLine, Point3<T> & intersection)
{
    // Check for degnerate inputs
    if (isDegenerate(theTri) || isDegenerate(theLine))
    {
        intersection = invalidPoint3<T>();
        return ResultCode::eDegenerate;
    }


    // from Moller: https://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/raytri/raytri.c

    Vector3<T> edge1 = theTri.p2() - theTri.p1();
    Vector3<T> edge2 = theTri.p3() - theTri.p1();

    Vector3<T> pvec = crossProduct(theLine.unitDirection(), edge2);

    T det = dotProduct(edge1, pvec);
    if (isEqual(det, T(0.0)))
    {
        intersection = invalidPoint3<T>();
        return ResultCode::eCoplanar;
    }

    Vector3<T> tvec = theLine.base() - theTri.p1();

    T u = dotProduct(tvec, pvec) / det;
    if (!(isGreaterOrEqual(u, T(0.0)) && isLessOrEqual(u, T(1.0))))
    {
        intersection = invalidPoint3<T>();
        return ResultCode::eNoIntersection;
    }

    Vector3<T> qvec = crossProduct(tvec, edge1);

    T v =  dotProduct(theLine.unitDirection(), qvec) / det;
    if (!(isGreaterOrEqual(v, T(0.0)) && isLessOrEqual(u + v, T(1.0))))
    {
        intersection = invalidPoint3<T>();
        return ResultCode::eNoIntersection;
    }

    T t = dotProduct(edge2, qvec) / det;
    intersection = theLine.base() + multiply(theLine.unitDirection(), t);

    if (!isValid(intersection))
    {
        return ResultCode::eOverflow;
    }

    return ResultCode::eOk;
}

template <typename T>
inline ResultCode intersect(const Line3<T> & theLine, const Triangle3<T> & theTri, Point3<T> & intersection)
{
    return intersect(theTri, theLine, intersection);
}


template <typename T>
inline ResultCode intersect(const Triangle3<T>& theTri, const Segment3<T>& theSegment, Point3<T>& intersection)
{
    // initialize the output point to an invalid point
    intersection = invalidPoint3<T>();

    // Check for degenerate inputs
    if (isDegenerate(theTri) || isDegenerate(theSegment))
    {
        return ResultCode::eDegenerate;
    }

    // from Moller: https://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/raytri/raytri.c

    Vector3<T> edge1 = theTri.p2() - theTri.p1();
    Vector3<T> edge2 = theTri.p3() - theTri.p1();

    Vector3<T> pvec = crossProduct(makeUnitVector3(theSegment.target() - theSegment.base()), edge2);

    T det = dotProduct(edge1, pvec);
    if (isEqual(det, T(0.0)))
    {
        return ResultCode::eCoplanar;
    }

    Vector3<T> tvec = theSegment.base() - theTri.p1();

    UnitVector3<T> segDir = makeUnitVector3(theSegment.target() - theSegment.base());


    T u = dotProduct(tvec, pvec) / det;
    if (!(isGreaterOrEqual(u, T(0.0)) && isLessOrEqual(u, T(1.0))))
    {
        return ResultCode::eNoIntersection;
    }

    Vector3<T> qvec = crossProduct(tvec, edge1);

    T v = dotProduct(segDir, qvec) / det;
    if (!(isGreaterOrEqual(v, T(0.0)) && isLessOrEqual(u + v, T(1.0))))
    {
        return ResultCode::eNoIntersection;
    }

    T t = dotProduct(edge2, qvec) / det;

    if (!isValid(t))
    {
        return ResultCode::eOverflow;
    }

    if (t < 0.0)
    {
        return ResultCode::eNoIntersection;
    }

    intersection = theSegment.base() + multiply(segDir, t);

    if (!isValid(intersection))
    {
        // generally, if the result is invalid, we return the constant invalidPoint3<T>()
        // but in this case we simply return the result of the calculation because it is
        // possibly useful to the caller to figure out what is overflowing.
        return ResultCode::eOverflow;
    }

    if (hubert::distance(intersection, theSegment.base()) > hubert::distance(theSegment.base(), theSegment.target()))
    {
        intersection = invalidPoint3<T>();
        return ResultCode::eNoIntersection;
    }

    // we intersected
    return ResultCode::eOk;
}

template <typename T>
inline ResultCode intersect(const Segment3<T>& theSegment, const Triangle3<T>& theTri, Point3<T>& intersection)
{
    return intersect(theTri, theSegment, intersection);
}

template <typename T>
inline ResultCode intersect(const Triangle3<T>& theTri, const Plane<T>& thePlane)
{
    // Degenerate triangles and planes are out of scope
    if (isDegenerate(theTri) || isDegenerate(thePlane))
    {
        return ResultCode::eDegenerate;
    }

    // since the triangle was not degenerate, we don't bother checking the segments
    hubert::Segment3<double> s1(theTri.p1(), theTri.p2());
    hubert::Segment3<double> s2(theTri.p2(), theTri.p3());
    hubert::Segment3<double> s3(theTri.p3(), theTri.p1());

    hubert::Point3<double> intPoint;
    auto ret1 = hubert::intersect(s1, thePlane, intPoint);
    auto ret2 = hubert::intersect(s2, thePlane, intPoint);
    auto ret3 = hubert::intersect(s3, thePlane, intPoint);

    // as long as one edge of the triangle intersects, it does not matter what the others do
    if (ret1 == ResultCode::eOk || ret2 == ResultCode::eOk || ret3 == ResultCode::eOk)
    {
        return ResultCode::eOk;
    }

    // if any of the calculations overflowed
    if (ret1 == ResultCode::eOverflow || ret2 == ResultCode::eOverflow || ret3 == ResultCode::eOverflow)
    {
        return ResultCode::eOverflow;
    }

    // if all three edges are coplanar, then that is a reasonable status
    if (ret1 == ResultCode::eCoplanar && ret2 == ResultCode::eCoplanar && ret3 == ResultCode::eCoplanar)
    {
        return ResultCode::eCoplanar;
    }

    // if all three edges are parallel, then that is a reasonable status
    if (ret1 == ResultCode::eParallel && ret2 == ResultCode::eParallel && ret3 == ResultCode::eParallel)
    {
        return ResultCode::eParallel;
    }

    // catchall - there was no intersection
    return ResultCode::eNoIntersection;
}

template <typename T>
inline ResultCode intersect(const Plane<T>& thePlane, const Triangle3<T>& theTri)
{
    return intersect(theTri, thePlane);
}


/* Triangle/triangle intersection test routine,
* by Tomas Moller, 1997.
* See article "A Fast Triangle-Triangle Intersection Test",
* Journal of Graphics Tools, 2(2), 1997
*
* Updated June 1999: removed the divisions -- a little faster now!
* Updated October 1999: added {} to CROSS and SUB macros
*
* int NoDivTriTriIsect(float V0[3],float V1[3],float V2[3],
*                      float U0[3],float U1[3],float U2[3])
*
* parameters: vertices of triangle 1: V0,V1,V2
*             vertices of triangle 2: U0,U1,U2
* result    : returns 1 if the triangles intersect, otherwise 0
*
*/

/*
Copyright 2020 Tomas Akenine-M???ller

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/



/* some macros */
#define CROSS(dest,v1,v2){                     \
              dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
              dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
              dest[2]=v1[0]*v2[1]-v1[1]*v2[0];}

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define SUB(dest,v1,v2){         \
            dest[0]=v1[0]-v2[0]; \
            dest[1]=v1[1]-v2[1]; \
            dest[2]=v1[2]-v2[2];}

/* sort so that a<=b */
#define SORT(a,b)       \
             if(a>b)    \
             {          \
               T ct; \
               ct=a;     \
               a=b;     \
               b=ct;     \
             }


/* this edge to edge test is based on Franlin Antonio's gem:
   "Faster Line Segment Intersection", in Graphics Gems III,
   pp. 199-202 */
#define EDGE_EDGE_TEST(V0,U0,U1)                      \
  Bx=U0[i0]-U1[i0];                                   \
  By=U0[i1]-U1[i1];                                   \
  Cx=V0[i0]-U0[i0];                                   \
  Cy=V0[i1]-U0[i1];                                   \
  f=Ay*Bx-Ax*By;                                      \
  d=By*Cx-Bx*Cy;                                      \
  if((f>0 && d>=0 && d<=f) || (f<0 && d<=0 && d>=f))  \
  {                                                   \
    e=Ax*Cy-Ay*Cx;                                    \
    if(f>0)                                           \
    {                                                 \
      if(e>=0 && e<=f) return ResultCode::eOk;                      \
    }                                                 \
    else                                              \
    {                                                 \
      if(e<=0 && e>=f) return ResultCode::eOk;                      \
    }                                                 \
  }

#define EDGE_AGAINST_TRI_EDGES(V0,V1,U0,U1,U2) \
{                                              \
  T Ax,Ay,Bx,By,Cx,Cy,e,d,f;               \
  Ax=V1[i0]-V0[i0];                            \
  Ay=V1[i1]-V0[i1];                            \
  /* test edge U0,U1 against V0,V1 */          \
  EDGE_EDGE_TEST(V0,U0,U1);                    \
  /* test edge U1,U2 against V0,V1 */          \
  EDGE_EDGE_TEST(V0,U1,U2);                    \
  /* test edge U2,U1 against V0,V1 */          \
  EDGE_EDGE_TEST(V0,U2,U0);                    \
}

#define POINT_IN_TRI(V0,U0,U1,U2)           \
{                                           \
  T a,b,c,d0,d1,d2;                     \
  /* is T1 completly inside T2? */          \
  /* check if V0 is inside tri(U0,U1,U2) */ \
  a=U1[i1]-U0[i1];                          \
  b=-(U1[i0]-U0[i0]);                       \
  c=-a*U0[i0]-b*U0[i1];                     \
  d0=a*V0[i0]+b*V0[i1]+c;                   \
                                            \
  a=U2[i1]-U1[i1];                          \
  b=-(U2[i0]-U1[i0]);                       \
  c=-a*U1[i0]-b*U1[i1];                     \
  d1=a*V0[i0]+b*V0[i1]+c;                   \
                                            \
  a=U0[i1]-U2[i1];                          \
  b=-(U0[i0]-U2[i0]);                       \
  c=-a*U2[i0]-b*U2[i1];                     \
  d2=a*V0[i0]+b*V0[i1]+c;                   \
  if(d0*d1>0.0)                             \
  {                                         \
    if(d0*d2>0.0) return ResultCode::eOk;                 \
  }                                         \
}

#define NEWCOMPUTE_INTERVALS(VV0,VV1,VV2,D0,D1,D2,D0D1,D0D2,A,B,C,X0,X1) \
{ \
        if(D0D1>0.0f) \
        { \
                /* here we know that D0D2<=0.0 */ \
            /* that is D0, D1 are on the same side, D2 on the other or on the plane */ \
                A=VV2; B=(VV0-VV2)*D2; C=(VV1-VV2)*D2; X0=D2-D0; X1=D2-D1; \
        } \
        else if(D0D2>0.0f)\
        { \
                /* here we know that d0d1<=0.0 */ \
            A=VV1; B=(VV0-VV1)*D1; C=(VV2-VV1)*D1; X0=D1-D0; X1=D1-D2; \
        } \
        else if(D1*D2>0.0f || D0!=0.0f) \
        { \
                /* here we know that d0d1<=0.0 or that D0!=0.0 */ \
                A=VV0; B=(VV1-VV0)*D0; C=(VV2-VV0)*D0; X0=D0-D1; X1=D0-D2; \
        } \
        else if(D1!=0.0f) \
        { \
                A=VV1; B=(VV0-VV1)*D1; C=(VV2-VV1)*D1; X0=D1-D0; X1=D1-D2; \
        } \
        else if(D2!=0.0f) \
        { \
                A=VV2; B=(VV0-VV2)*D2; C=(VV1-VV2)*D2; X0=D2-D0; X1=D2-D1; \
        } \
        else \
        { \
                /* triangles are coplanar */ \
                return coplanar_tri_tri(N1,V0,V1,V2,U0,U1,U2); \
        } \
}



template <typename T>
inline ResultCode coplanar_tri_tri(T N[3], T V0[3], T V1[3], T V2[3],
    T U0[3], T U1[3], T U2[3])
{
    T A[3];
    short i0, i1;
    /* first project onto an axis-aligned plane, that maximizes the area */
    /* of the triangles, compute indices: i0,i1. */
    A[0] = std::abs(N[0]);
    A[1] = std::abs(N[1]);
    A[2] = std::abs(N[2]);
    if (A[0] > A[1])
    {
        if (A[0] > A[2])
        {
            i0 = 1;      /* A[0] is greatest */
            i1 = 2;
        }
        else
        {
            i0 = 0;      /* A[2] is greatest */
            i1 = 1;
        }
    }
    else   /* A[0]<=A[1] */
    {
        if (A[2] > A[1])
        {
            i0 = 0;      /* A[2] is greatest */
            i1 = 1;
        }
        else
        {
            i0 = 0;      /* A[1] is greatest */
            i1 = 2;
        }
    }

    /* test all edges of triangle 1 against the edges of triangle 2 */
    EDGE_AGAINST_TRI_EDGES(V0, V1, U0, U1, U2);
    EDGE_AGAINST_TRI_EDGES(V1, V2, U0, U1, U2);
    EDGE_AGAINST_TRI_EDGES(V2, V0, U0, U1, U2);

    /* finally, test if tri1 is totally contained in tri2 or vice versa */
    POINT_IN_TRI(V0, U0, U1, U2);
    POINT_IN_TRI(U0, V0, V1, V2);

    return ResultCode::eNoIntersection;
}


template <typename T>
inline ResultCode intersect(const Triangle3<T>& tri1, const Triangle3<T>& tri2)
{
    T V0[3], V1[3], V2[3];
    T U0[3], U1[3], U2[3];

    V0[0] = tri1.p1().x();
    V0[1] = tri1.p1().y();
    V0[2] = tri1.p1().z();

    V1[0] = tri1.p2().x();
    V1[1] = tri1.p2().y();
    V1[2] = tri1.p2().z();

    V2[0] = tri1.p3().x();
    V2[1] = tri1.p3().y();
    V2[2] = tri1.p3().z();

    U0[0] = tri2.p1().x();
    U0[1] = tri2.p1().y();
    U0[2] = tri2.p1().z();

    U1[0] = tri2.p2().x();
    U1[1] = tri2.p2().y();
    U1[2] = tri2.p2().z();

    U2[0] = tri2.p3().x();
    U2[1] = tri2.p3().y();
    U2[2] = tri2.p3().z();

    T E1[3], E2[3];
    T N1[3], N2[3], d1, d2;
    T du0, du1, du2, dv0, dv1, dv2;
    T D[3];
    T isect1[2], isect2[2];
    T du0du1, du0du2, dv0dv1, dv0dv2;
    short index;
    T vp0, vp1, vp2;
    T up0, up1, up2;
    T bb, cc, max;

    /* compute plane equation of triangle(V0,V1,V2) */
    SUB(E1, V1, V0);
    SUB(E2, V2, V0);
    CROSS(N1, E1, E2);
    d1 = -DOT(N1, V0);
    /* plane equation 1: N1.X+d1=0 */

    /* put U0,U1,U2 into plane equation 1 to compute signed distances to the plane*/
    du0 = DOT(N1, U0) + d1;
    du1 = DOT(N1, U1) + d1;
    du2 = DOT(N1, U2) + d1;

    /* coplanarity robustness check */

    if (isEqual(du0, T(0.0))) du0 = 0.0;
    if (isEqual(du1, T(0.0))) du1 = 0.0;
    if (isEqual(du2, T(0.0))) du2 = 0.0;

    du0du1 = du0 * du1;
    du0du2 = du0 * du2;

    if (du0du1 > 0.0f && du0du2 > 0.0f) /* same sign on all of them + not equal 0 ? */
        return ResultCode::eNoIntersection;                    /* no intersection occurs */

     /* compute plane of triangle (U0,U1,U2) */
    SUB(E1, U1, U0);
    SUB(E2, U2, U0);
    CROSS(N2, E1, E2);
    d2 = -DOT(N2, U0);
    /* plane equation 2: N2.X+d2=0 */

    /* put V0,V1,V2 into plane equation 2 */
    dv0 = DOT(N2, V0) + d2;
    dv1 = DOT(N2, V1) + d2;
    dv2 = DOT(N2, V2) + d2;

    if (isEqual(dv0, T(0.0))) dv0 = 0.0;
    if (isEqual(dv1, T(0.0))) dv1 = 0.0;
    if (isEqual(dv2, T(0.0))) dv2 = 0.0;

    dv0dv1 = dv0 * dv1;
    dv0dv2 = dv0 * dv2;

    if (dv0dv1 > 0.0f && dv0dv2 > 0.0f) /* same sign on all of them + not equal 0 ? */
        return ResultCode::eNoIntersection;                    /* no intersection occurs */

     /* compute direction of intersection line */
    CROSS(D, N1, N2);

    /* compute and index to the largest component of D */
    max = std::abs(D[0]);
    index = 0;
    bb = std::abs(D[1]);
    cc = std::abs(D[2]);
    if (bb > max) max = bb, index = 1;
    if (cc > max) max = cc, index = 2;

    /* this is the simplified projection onto L*/
    vp0 = V0[index];
    vp1 = V1[index];
    vp2 = V2[index];

    up0 = U0[index];
    up1 = U1[index];
    up2 = U2[index];

    /* compute interval for triangle 1 */
    T a, b, c, x0, x1;
    NEWCOMPUTE_INTERVALS(vp0, vp1, vp2, dv0, dv1, dv2, dv0dv1, dv0dv2, a, b, c, x0, x1);

    /* compute interval for triangle 2 */
    T d, e, f, y0, y1;
    NEWCOMPUTE_INTERVALS(up0, up1, up2, du0, du1, du2, du0du1, du0du2, d, e, f, y0, y1);

    T xx, yy, xxyy, tmp;
    xx = x0 * x1;
    yy = y0 * y1;
    xxyy = xx * yy;

    tmp = a * xxyy;
    isect1[0] = tmp + b * x1 * yy;
    isect1[1] = tmp + c * x0 * yy;

    tmp = d * xxyy;
    isect2[0] = tmp + e * xx * y1;
    isect2[1] = tmp + f * xx * y0;

    SORT(isect1[0], isect1[1]);
    SORT(isect2[0], isect2[1]);

    if (isect1[1] < isect2[0] || isect2[1] < isect1[0]) 
        return ResultCode::eNoIntersection;

    return ResultCode::eOk;
}

} // end of hubert namespace

#endif
