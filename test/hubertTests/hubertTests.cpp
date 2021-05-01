﻿// hubertTests.cpp : Defines the entry point for the application.
//


// system headers
#include <iomanip>
#include <iostream>
#include <vector>

// hubert header - that's what we are testing
#include "hubert.hpp"


///////////////////////////////////////////////////////////////////////////
// Set up a few sets of "special" numbers which we commonly test for and
// a template mechanism to get the correct set of numbers, since all 
// of the unity tests are templated on the supported floating point types.
///////////////////////////////////////////////////////////////////////////
struct HubertTestSetup
{
    // Canonical list of "invalid" numbers used for testing of error conditions

    std::vector<float> invalidFloat{
        std::numeric_limits<float>::quiet_NaN(),
        std::numeric_limits<float>::infinity(),
        -std::numeric_limits<float>::infinity(),
    };

    std::vector<double> invalidDouble{
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity(),
    };

    template<typename T>
    const std::vector<T>& getInvalid()
    {
        if (sizeof(T) == sizeof(double))
        {
            return (std::vector<T> &) invalidDouble;
        }
        else
        {
            return (std::vector<T> &) invalidFloat;
        }
    }

    // a list of various categories of valid (both normal and subnormal) numbers.

    std::vector<float> validFloat{
        float(0.0),                                             // zero can cause issues
        float(1.0),                                             // normal number with exact representation
        float(1.1),                                             // normal number without exact representation
        float(1.1e16),                                          // big number
        float(1.1e-16),                                         // little number
        float(std::numeric_limits<float>::min() / float(2.0)),  // subnormal number
        float(std::numeric_limits<float>::min()),               // smallest positive
        -float(std::numeric_limits<float>::min()),              // smallest negative
        float(std::numeric_limits<float>::max()),               // biggest postive
        -float(std::numeric_limits<float>::max()),              // biggest negative
    };

    std::vector<double> validDouble{
        double(0.0),                                            // zero can cause issues
        double(1.0),                                            // normal number with exact representation
        double(1.1),                                            // normal number without exact representation
        double(1.1e32),                                         // big number
        double(1.1e-32),                                        // little number
        double(std::numeric_limits<float>::min() / float(2.0)), // subnormal number
        double(std::numeric_limits<double>::min()),             // smallest positive
        -double(std::numeric_limits<double>::min()),            // smallest negative
        double(std::numeric_limits<double>::max()),             // biggest postive
        -double(std::numeric_limits<double>::max()),            // biggest negative
    };

    template<typename T, typename dummy = void>
    const std::vector<T> & getValid() 
    { 
        if (sizeof(T) == sizeof(double))
        {
            return (std::vector<T> &)validDouble;
        }
        else
        {
            return (std::vector<T> &)validFloat;
        }
        
    }

    // a list of normal (not sub-normal, and not zero) numbers.

    std::vector<float> normalFloat{
        float(1.0),                                             // normal number with exact representation
        float(1.1),                                             // normal number without exact representation
        float(1.1e16),                                          // big number
        float(1.1e-16),                                         // little number
        float(std::numeric_limits<float>::min()),               // smallest positive
        -float(std::numeric_limits<float>::min()),              // smallest negative
        float(std::numeric_limits<float>::max()),               // biggest postive
        -float(std::numeric_limits<float>::max()),              // biggest negative
    };

    std::vector<double> normalDouble{
        double(1.0),                                            // normal number with exact representation
        double(1.1),                                            // normal number without exact representation
        double(1.1e16),                                         // big number
        double(1.1e-16),                                        // little number
        double(std::numeric_limits<double>::min()),             // smallest positive
        -double(std::numeric_limits<double>::min()),            // smallest negative
        double(std::numeric_limits<double>::max()),             // biggest postive
        -double(std::numeric_limits<double>::max()),            // biggest negative
    };


    // a handful of subnormal numbers- valid but reduced precision

    std::vector <float> subnormalFloat{
        float(std::numeric_limits<float>::min() / float(2.0)),
        float(std::numeric_limits<float>::min() / float(4.0)),
        float(std::numeric_limits<float>::min() / float(8.0)),
    };

    std::vector <double> subnormalDouble{
        double(std::numeric_limits<double>::min() / double(2.0)),
        double(std::numeric_limits<double>::min() / double(4.0)),
        double(std::numeric_limits<double>::min() / double(8.0)),
    };

    // the extremes of the valid range, for checking over/underflow conditions

    std::vector<float> extremeFloat{
        float(std::numeric_limits<float>::min()),
        -float(std::numeric_limits<float>::min()),
        float(std::numeric_limits<float>::max()),
        -float(std::numeric_limits<float>::max()),
    };

    std::vector<double> extremeDouble{
        double(std::numeric_limits<double>::min()),
        -double(std::numeric_limits<double>::min()),
        double(std::numeric_limits<double>::max()),
        -double(std::numeric_limits<double>::max()),
    };
};
static HubertTestSetup gSetup;

// We use the Catch2 open source framework for unit tests. This initializes it.
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch_amalgamated.hpp"

///////////////////////////////////////////////////////////////////////////
//
// Some basic tests, more to illustrate some points than to actually 
// test anything
//
///////////////////////////////////////////////////////////////////////////


// When using constants to test, make sure to use values that don't have
// an exact floating point representation. This ensures that 
// float(x) != double(x) and helps pick up problems related to type
// coercion and precision.
//
// For example 3.0 and 3.5 both have exact representations so
// float(3.5) == d ouble(3.5), but 3.1 does not have an exact representation
// and so it fails the same test.

TEST_CASE("Illustrate non exact representation", "[intro]") {
    CHECK(float(3.5) ==  double(3.5));
    CHECK(float(3.1) != double(3.1));
}

// When using constants - use a high precision math system to compute them to 
// at least 38 significant digits. (Generally we try to standardize on 50).
// That way the tests will most definitely be ready for 128 bit floats when
// we wish to extend the library to supporting those. For example:

TEST_CASE("Illustrate float constants", "[intro]") {
    double dv = double(10.566456359631643318492081168978985573982790623604);
    float fv = float(10.566456359631643318492081168978985573982790623604);
    CHECK(dv != fv);
}

/////////////////////////////////////////////////////////////////////////////
// Epsilon comparisons
/////////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Epsilon equals", "[epsilon]", float, double)
{
    TestType  shrinkFactor = TestType(.75);

    SECTION("Values are positive")
    {
        TestType  f1 = TestType (10.1);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<TestType >::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<TestType >::epsilon() * f1 * shrinkFactor) == true);
        CHECK_FALSE(hubert::isEqual(f1, f1 + std::numeric_limits<TestType >::epsilon() * (f1 * TestType (2.0))));
        CHECK_FALSE(hubert::isEqual(f1, f1 - std::numeric_limits<TestType >::epsilon() * (f1 * TestType (2.0))));
    }

    SECTION("Values are negative")
    {
        TestType  f1 = TestType (-10.1);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<TestType >::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<TestType >::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<TestType >::epsilon() * (f1 * TestType (2.0))) == false);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<TestType >::epsilon() * (f1 * TestType (2.0))) == false);
    }

    SECTION("Values are positive and huge")
    {
        TestType  f1 = TestType (10.1e17);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<TestType >::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<TestType >::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<TestType >::epsilon() * (f1 * TestType (2.0))) == false);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<TestType >::epsilon() * (f1 * TestType (2.0))) == false);
    }

    SECTION("Values are negative and huge")
    {
        TestType  f1 = TestType (-10.1e17);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<TestType >::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<TestType >::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<TestType >::epsilon() * (f1 * TestType (2.0))) == false);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<TestType >::epsilon() * (f1 * TestType (2.0))) == false);
    }

    SECTION("Values are positive and tiny")
    {
        TestType  f1 = TestType (10.1e-17);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<TestType >::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<TestType >::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<TestType >::epsilon() * (f1 * TestType (2.0))) == false);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<TestType >::epsilon() * (f1 * TestType (2.0))) == false);
    }

    SECTION("Values are negative and tiny")
    {
        TestType  f1 = TestType (-10.1e-17);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<TestType >::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<TestType >::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<TestType >::epsilon() * (f1 * TestType (2.0))) == false);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<TestType >::epsilon() * (f1 * TestType (2.0))) == false);
    }
}


TEMPLATE_TEST_CASE("Epsilon greater or equal", "[epsilon]", float, double)
{
    TestType shrinkFactor = .75;

    SECTION ("Values are positive and equal or close to equal")
    {
        TestType f1 = TestType(10.1);
        CHECK(hubert::isGreaterOrEqual(f1, f1) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 - std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are negative and equal or close to equal")
    {
        TestType f1 = TestType(-10.1);
        CHECK(hubert::isGreaterOrEqual(f1, f1) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 - std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are huge positive and equal or close to equal")
    {
        TestType f1 = TestType(10.1e17);
        CHECK(hubert::isGreaterOrEqual(f1, f1) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 - std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are huge negative and equal or close to equal")
    {
        TestType f1 = TestType(-10.1e17);
        CHECK(hubert::isGreaterOrEqual(f1, f1) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 - std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are tiny positive and equal or close to equal")
    {
        TestType f1 = TestType(10.1e-17);
        CHECK(hubert::isGreaterOrEqual(f1, f1) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 - std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are tiny negative and equal or close to equal")
    {
        TestType f1 = TestType(-10.1e-17);
        CHECK(hubert::isGreaterOrEqual(f1, f1) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 - std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
    }

    TestType growFactor = 2.0;

    SECTION("Values are positive and greater")
    {
        TestType f1 = TestType(10.1);
        CHECK(hubert::isGreaterOrEqual(f1 + std::numeric_limits<TestType>::epsilon() * f1 * growFactor, f1) == true);
    }

    SECTION("Values are negative and greater")
    {
        TestType f1 = TestType(-10.1);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<TestType>::epsilon() * f1 * growFactor) == true);
    }

    SECTION("Values are huge positive and greater")
    {
        TestType f1 = TestType(10.1e17);
        CHECK(hubert::isGreaterOrEqual(f1 + std::numeric_limits<TestType>::epsilon() * f1 * growFactor, f1) == true);
    }

    SECTION("Values are huge negative and greater")
    {
        TestType f1 = TestType(-10.1e-17);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<TestType>::epsilon() * f1 * growFactor) == true);
    }

    SECTION("Values are tiny positive and greater")
    {
        TestType f1 = TestType(10.1e-17);
        CHECK(hubert::isGreaterOrEqual(f1 + std::numeric_limits<TestType>::epsilon() * f1 * growFactor, f1) == true);
    }

    SECTION("Values are tiny negative and greater")
    {
        TestType f1 = TestType(-10.1e-17);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<TestType>::epsilon() * f1 * growFactor) == true);
    }
}


TEMPLATE_TEST_CASE("Epsilon less or equal", "[epsilon]", float, double)
{
    TestType shrinkFactor = .75;

    SECTION("Values are positive and equal or close to equal")
    {
        TestType f1 = TestType(10.1);
        CHECK(hubert::isLessOrEqual(f1, f1) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 - std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are negative and equal or close to equal")
    {
        TestType f1 = TestType(-10.1);
        CHECK(hubert::isLessOrEqual(f1, f1) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 - std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are huge positive and equal or close to equal")
    {
        TestType f1 = TestType(10.1e17);
        CHECK(hubert::isLessOrEqual(f1, f1) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 - std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are huge negative and equal or close to equal")
    {
        TestType f1 = TestType(-10.1e17);
        CHECK(hubert::isLessOrEqual(f1, f1) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 - std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are tiny positive and equal or close to equal")
    {
        TestType f1 = TestType(10.1e-17);
        CHECK(hubert::isLessOrEqual(f1, f1) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 - std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are tiny negative and equal or close to equal")
    {
        TestType f1 = TestType(-10.1e-17);
        CHECK(hubert::isLessOrEqual(f1, f1) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 - std::numeric_limits<TestType>::epsilon() * f1 * shrinkFactor) == true);
    }

    TestType growFactor = 2.0;

    SECTION("Values are positive and less")
    {
        TestType f1 = TestType(10.1);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<TestType>::epsilon() * f1 * growFactor) == true);
    }

    SECTION("Values are negative and less")
    {
        TestType f1 = TestType(-10.1);
        CHECK(hubert::isLessOrEqual(f1 + std::numeric_limits<TestType>::epsilon() * f1 * growFactor, f1) == true);
    }

    SECTION("Values are huge positive and less")
    {
        TestType f1 = TestType(10.1e17);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<TestType>::epsilon() * f1 * growFactor) == true);
    }

    SECTION("Values are huge negative and less")
    {
        TestType f1 = TestType(-10.1e-17);
        CHECK(hubert::isLessOrEqual( f1 + std::numeric_limits<TestType>::epsilon() * f1 * growFactor, f1) == true);
    }

    SECTION("Values are tiny positive and less")
    {
        TestType f1 = TestType(10.1e-17);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<TestType>::epsilon() * f1 * growFactor) == true);
    }

    SECTION("Values are tiny negative and less")
    {
        TestType f1 = TestType(-10.1e-17);
        CHECK(hubert::isLessOrEqual(f1 + std::numeric_limits<TestType>::epsilon() * f1 * growFactor, f1) == true);
    }
}

///////////////////////////////////////////////////////////////////////////
// Validation checks on primitive types 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Invalidity check on primitive types", "[validity]", float, double)
{
    SECTION("Results should be true")
    {
        CHECK(hubert::isValid(TestType(1.2)) == true);
        CHECK(hubert::isValid(TestType(0.0)) == true);
        CHECK(hubert::isValid(TestType(-0.0)) == true);
        CHECK(hubert::isValid(std::numeric_limits<TestType>::min() / TestType(2.0)) == true);
    }

    SECTION("Results should be false")
    {
        CHECK(hubert::isValid(std::numeric_limits<TestType>::infinity()) == false);
        CHECK(hubert::isValid(-std::numeric_limits<TestType>::infinity()) == false);
        CHECK(hubert::isValid(std::numeric_limits<TestType>::quiet_NaN()) == false);
        CHECK(hubert::isValid(-std::numeric_limits<TestType>::quiet_NaN()) == false);
    }
}

TEMPLATE_TEST_CASE("Subnormal  check on primitive types", "[validity]", float, double)
{
    SECTION("Results should be true")
    {
        CHECK(hubert::isSubnormal(std::numeric_limits<TestType>::min() / TestType(2.0)) == true);
    }

    SECTION("Results should be false")
    {
        CHECK(hubert::isSubnormal(TestType(1.2)) == false);
        CHECK(hubert::isSubnormal(TestType(0.0)) == false);
        CHECK(hubert::isSubnormal(TestType(-0.0)) == false);
        CHECK(hubert::isSubnormal(std::numeric_limits<TestType>::infinity()) == false);
        CHECK(hubert::isSubnormal(-std::numeric_limits<TestType>::infinity()) == false);
        CHECK(hubert::isSubnormal(std::numeric_limits<TestType>::quiet_NaN()) == false);
        CHECK(hubert::isSubnormal(-std::numeric_limits<TestType>::quiet_NaN()) == false);
    }
}

///////////////////////////////////////////////////////////////////////////
// Point3 construction tests 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Construct Point3 with default parameters", "[Point3]", float, double)
{
    hubert::Point3<TestType> p1;

    CHECK(p1.x() == TestType(0.0));
    CHECK(p1.y() == TestType(0.0));
    CHECK(p1.z() == TestType(0.0));

    CHECK(isValid(p1));
    CHECK_FALSE(isDegenerate(p1));
}

TEMPLATE_TEST_CASE("Construct Point3 with constant parameters", "[Point3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));

    CHECK(p1.x() == TestType(1.1));
    CHECK(p1.y() == TestType(2.1));
    CHECK(p1.z() == TestType(3.1));

    CHECK(isValid(p1));
    CHECK_FALSE(isDegenerate(p1));
}


TEMPLATE_TEST_CASE("Construct Point3 with copy  constructor", "[Point3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Point3<TestType> p2(p1);

    CHECK(p2.x() == TestType(1.1));
    CHECK(p2.y() == TestType(2.1));
    CHECK(p2.z() == TestType(3.1));

    CHECK(isValid(p2));
    CHECK_FALSE(isDegenerate(p2));
}

TEMPLATE_TEST_CASE("Construct Point3 with assignment", "[Point3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Point3<TestType> p2 = p1;

    CHECK(p2.x() == TestType(1.1));
    CHECK(p2.y() == TestType(2.1));
    CHECK(p2.z() == TestType(3.1));

    CHECK(isValid(p2));
    CHECK_FALSE(isDegenerate(p2));
}

TEMPLATE_TEST_CASE("Construct Point3 with initializer", "[Point3]", float, double)
{
    hubert::Point3<TestType> p2{ TestType(1.1), TestType(2.1), TestType(3.1) };

    CHECK(p2.x() == TestType(1.1));
    CHECK(p2.y() == TestType(2.1));
    CHECK(p2.z() == TestType(3.1));

    CHECK(isValid(p2));
    CHECK_FALSE(isDegenerate(p2));
}

///////////////////////////////////////////////////////////////////////////
// Point3 validity 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Check Point3 validity routines", "[Point3]", float, double)
{
    SECTION("Valid")
    {
        for (auto x : gSetup.getValid<TestType>())
        {
            for (auto y : gSetup.getValid<TestType>())
            {
                for (auto z : gSetup.getValid<TestType>())
                {
                    hubert::Point3<TestType> p1{ x, y, z };

                    CHECK(p1.amValid() == true);
                    CHECK(p1.amDegenerate() == false);

                    // we have already run the isValid test - so assuming it works, 
                    // this does too
                    bool tv = hubert::isSubnormal(x) || hubert::isSubnormal(y) || hubert::isSubnormal(z);
                    CHECK(p1.amSubnormal() == tv);
                }
            }
        }
    }

    SECTION("Invalid")
    {
        for (auto x : gSetup.getInvalid<TestType>())
        {
            for (auto y : gSetup.getInvalid<TestType>())
            {
                for (auto z : gSetup.getInvalid<TestType>())
                {
                    hubert::Point3<TestType> p1{ x, y, z };

                    CHECK(p1.amValid() == false);
                    CHECK(p1.amDegenerate() == true);
                    CHECK(p1.amSubnormal() == false);
                }
            }
        }
    }
} 

///////////////////////////////////////////////////////////////////////////
// Point3 specific degeneracy tests 
///////////////////////////////////////////////////////////////////////////

// there are currently no non-validity related degeneracy cases for Point3

///////////////////////////////////////////////////////////////////////////
// Vector3 construction tests 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Construct Vector3 with default parameters", "[Vector3]", float, double)
{
    hubert::Vector3<TestType> p1;

    CHECK(p1.x() == TestType(0.0));
    CHECK(p1.y() == TestType(0.0));
    CHECK(p1.z() == TestType(0.0));

    CHECK(isValid(p1));
    CHECK_FALSE(isDegenerate(p1));
}

TEMPLATE_TEST_CASE("Construct Vector3 with constant parameters", "[Vector3]", float, double)
{
    hubert::Vector3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));

    CHECK(p1.x() == TestType(1.1));
    CHECK(p1.y() == TestType(2.1));
    CHECK(p1.z() == TestType(3.1));

    CHECK(isValid(p1));
    CHECK_FALSE(isDegenerate(p1));
}


TEMPLATE_TEST_CASE("Construct Vector3 with copy  constructor", "[Vector3]", float, double)
{
    hubert::Vector3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Vector3<TestType> p2(p1);

    CHECK(p2.x() == TestType(1.1));
    CHECK(p2.y() == TestType(2.1));
    CHECK(p2.z() == TestType(3.1));

    CHECK(isValid(p2));
    CHECK_FALSE(isDegenerate(p2));
}

TEMPLATE_TEST_CASE("Construct Vector3 with assignment", "[Vector3]", float, double)
{
    hubert::Vector3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Vector3<TestType> p2 = p1;

    CHECK(p2.x() == TestType(1.1));
    CHECK(p2.y() == TestType(2.1));
    CHECK(p2.z() == TestType(3.1));

    CHECK(isValid(p2));
    CHECK_FALSE(isDegenerate(p2));
}

TEMPLATE_TEST_CASE("Construct Vector3 with initializer", "[Vector3]", float, double)
{
    hubert::Vector3<TestType> p2{ TestType(1.1), TestType(2.1), TestType(3.1) };

    CHECK(p2.x() == TestType(1.1));
    CHECK(p2.y() == TestType(2.1));
    CHECK(p2.z() == TestType(3.1));

    CHECK(isValid(p2));
    CHECK_FALSE(isDegenerate(p2));
}

///////////////////////////////////////////////////////////////////////////
// Vector3 validity 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Check Vector3 validity routines", "[Vector3]", float, double)
{
    SECTION("Valid")
    {
        for (auto x : gSetup.getValid<TestType>())
        {
            for (auto y : gSetup.getValid<TestType>())
            {
                for (auto z : gSetup.getValid<TestType>())
                {
                    hubert::Vector3<TestType> p1{ x, y, z };

                    CHECK(p1.amValid() == true);
                    CHECK(p1.amDegenerate() == false);

                    // we have already run the isValid test - so assuming it works, 
                    // this does too
                    bool tv = hubert::isSubnormal(x) || hubert::isSubnormal(y) || hubert::isSubnormal(z);
                    CHECK(p1.amSubnormal() == tv);
                }
            }
        }
    } 

    SECTION("Invalid")
    {
        for (auto x : gSetup.getInvalid<TestType>())
        {
            for (auto y : gSetup.getInvalid<TestType>())
            {
                for (auto z : gSetup.getInvalid<TestType>())
                {
                    hubert::Vector3<TestType> p1{ x, y, z };

                    CHECK(p1.amValid() == false);
                    CHECK(p1.amDegenerate() == true);
                    CHECK(p1.amSubnormal() == false);
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// Vector3 specific degeneracy tests 
///////////////////////////////////////////////////////////////////////////

// there are currently no non-validity related degeneracy cases for Vector3

///////////////////////////////////////////////////////////////////////////
// Vector3 magnitude function 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Check Vector3 magnitude function", "[Vector3]", float, double)
{
    SECTION("Valid exact positive")
    {
        hubert::Vector3<TestType> p1{ TestType(2.0), TestType(10.0), TestType(11.0) };
        CHECK(hubert::isEqual(p1.magnitude(), TestType(15.0)));
    } 
    SECTION("Valid exact negative")
    {
        hubert::Vector3<TestType> p1{ TestType(2.0), TestType(-10.0), TestType(11.0) };
        CHECK(hubert::isEqual(p1.magnitude(), TestType(15.0)));
    }
    SECTION("Valid inexact")
    {
        hubert::Vector3<TestType> p1{ TestType(2.1), TestType(3.1), TestType(4.1) };
        CHECK(hubert::isEqual(p1.magnitude(), std::hypot(TestType(2.1), TestType(3.1), TestType(4.1))));
    }

    SECTION("Invalid")
    {
        hubert::Vector3<TestType> p1{ hubert::infinity<TestType>(), TestType(3.1), TestType(4.1) };
        CHECK(p1.magnitude() == hubert::infinity<TestType>());
    }
} 

///////////////////////////////////////////////////////////////////////////
// UnitVector3 construction tests 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Construct UnitVector3 with default parameters", "[UnitVector3]", float, double)
{
    hubert::UnitVector3<TestType> p1;

    CHECK(p1.x() == TestType(0.0));
    CHECK(p1.y() == TestType(1.0));
    CHECK(p1.z() == TestType(0.0));

    CHECK(isValid(p1));
    CHECK_FALSE(isDegenerate(p1));
}

TEMPLATE_TEST_CASE("Construct UnitVector3 with constant parameters", "[UnitVector3]", float, double)
{
    hubert::UnitVector3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));

    TestType scale = sqrt(TestType(1.1) * TestType(1.1) + TestType(2.1) * TestType(2.1) + TestType(3.1) * TestType(3.1));
    hubert::UnitVector3<TestType> p2(p1.x() / scale, p1.y() / scale, p1.z() / scale);

    CHECK(hubert::isEqual(p1.x(), p2.x()));
    CHECK(hubert::isEqual(p1.y(), p2.y()));
    CHECK(hubert::isEqual(p1.z(), p2.z()));

    CHECK(isValid(p1));
    CHECK_FALSE(isDegenerate(p1));
}


TEMPLATE_TEST_CASE("Construct UnitVector3 with copy  constructor", "[UnitVector3]", float, double)
{
    hubert::UnitVector3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::UnitVector3<TestType> p2(p1);

    TestType scale = sqrt(TestType(1.1) * TestType(1.1) + TestType(2.1) * TestType(2.1) + TestType(3.1) * TestType(3.1));
    hubert::UnitVector3<TestType> p3(p1.x() / scale, p1.y() / scale, p1.z() / scale);

    CHECK(hubert::isEqual(p3.x(), p2.x()));
    CHECK(hubert::isEqual(p3.y(), p2.y()));
    CHECK(hubert::isEqual(p3.z(), p2.z()));

    CHECK(isValid(p3));
    CHECK_FALSE(isDegenerate(p3));
}

TEMPLATE_TEST_CASE("Construct UnitVector3 with assignment", "[UnitVector3]", float, double)
{
    hubert::UnitVector3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::UnitVector3<TestType> p2 = p1;

    TestType scale = sqrt(TestType(1.1) * TestType(1.1) + TestType(2.1) * TestType(2.1) + TestType(3.1) * TestType(3.1));
    hubert::UnitVector3<TestType> p3(p1.x() / scale, p1.y() / scale, p1.z() / scale);

    CHECK(hubert::isEqual(p3.x(), p2.x()));
    CHECK(hubert::isEqual(p3.y(), p2.y()));
    CHECK(hubert::isEqual(p3.z(), p2.z()));

    CHECK(isValid(p3));
    CHECK_FALSE(isDegenerate(p3));
}

TEMPLATE_TEST_CASE("Construct UnitVector3 with initializer", "[UnitVector3]", float, double)
{
    hubert::UnitVector3<TestType> p1{ TestType(1.1), TestType(2.1), TestType(3.1) };

    TestType scale = sqrt(TestType(1.1) * TestType(1.1) + TestType(2.1) * TestType(2.1) + TestType(3.1) * TestType(3.1));
    hubert::UnitVector3<TestType> p2(p1.x() / scale, p1.y() / scale, p1.z() / scale);

    CHECK(hubert::isEqual(p1.x(), p2.x()));
    CHECK(hubert::isEqual(p1.y(), p2.y()));
    CHECK(hubert::isEqual(p1.z(), p2.z()));

    CHECK(isValid(p1));
    CHECK_FALSE(isDegenerate(p1));
}

///////////////////////////////////////////////////////////////////////////
// UnitVector3 validity 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Check UnitVector3 validity routines", "[UnitVector3]", float, double)
{
    SECTION("Valid")
    {
        for (auto x : gSetup.getValid<TestType>())
        {
            for (auto y : gSetup.getValid<TestType>())
            {
                for (auto z : gSetup.getValid<TestType>())
                {
                    hubert::UnitVector3<TestType> p1{ x, y, z };

                    TestType len = std::hypot(x, y, z);

                    if (hubert::isEqual(len, TestType(0.0)))
                    {
                        CHECK(p1.amValid() == true);
                        CHECK(p1.amDegenerate() == true);

                    }
                    else if (len == hubert::infinity<TestType>())
                    {
                        CHECK(p1.amValid() == true);
                        CHECK(p1.amDegenerate() == true);
                    }
                    else
                    {
                        TestType nlen = std::hypot(p1.x(), p1.y(), + p1.z());
                        CHECK(hubert::isEqual(nlen, TestType(1.0)));
                        CHECK(p1.amValid() == true);
                        CHECK(p1.amDegenerate() == false);
                    }

                    // we have already run the isValid test - so assuming it works, 
                    // this does too
                    bool tv = hubert::isSubnormal(x) || hubert::isSubnormal(y) || hubert::isSubnormal(z);
                    bool tv2 = hubert::isSubnormal(x/len) || hubert::isSubnormal(y/len) || hubert::isSubnormal(z/len);
                    CHECK(p1.amSubnormal() == (tv || tv2));
                }
            }
        }
    }

    SECTION("Invalid") 
    {
        for (auto x : gSetup.getInvalid<TestType>())
        {
            for (auto y : gSetup.getInvalid<TestType>())
            {
                for (auto z : gSetup.getInvalid<TestType>())
                {
                    hubert::UnitVector3<TestType> p1{ x, y, z };

                    CHECK(p1.amValid() == false);
                    CHECK(p1.amDegenerate() == true);
                    CHECK(p1.amSubnormal() == false);
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// UnitVector3 specific degeneracy tests 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("UnitVector3 specific degeneracy", "[Line3]", float, double)
{
    SECTION("Zero length")
    {
        hubert::UnitVector3<TestType> uv(0.0, 0.0, hubert::epsilon<TestType>());
        CHECK(uv.amDegenerate());
        CHECK(hubert::isDegenerate(uv));
    }

    SECTION("Just over zero length")
    {
        hubert::UnitVector3<TestType> uv(0.0, 0.0, hubert::epsilon<TestType>() * TestType(2.0));
        CHECK_FALSE(uv.amDegenerate());
        CHECK_FALSE(hubert::isDegenerate(uv));
    }


    SECTION("Very large")
    {
        hubert::UnitVector3<TestType> uv(std::numeric_limits<TestType>::max(), std::numeric_limits<TestType>::max(), std::numeric_limits<TestType>::max());
        CHECK(uv.amDegenerate());
        CHECK(hubert::isDegenerate(uv));
    }

    SECTION("Just under very large") 
    {
        hubert::UnitVector3<TestType> uv(std::numeric_limits<TestType>::max() / TestType(2.0), std::numeric_limits<TestType>::max() / TestType(2.0), std::numeric_limits<TestType>::max() / TestType(2.0));
        CHECK_FALSE(uv.amDegenerate());
        CHECK_FALSE(hubert::isDegenerate(uv));
    }
}

///////////////////////////////////////////////////////////////////////////
// Line3 construction tests 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Construct Line3 with default parameters", "[Line3]", float, double)
{
    hubert::Line3<TestType> theLine;

    CHECK(theLine.base().x() == TestType(0.0));
    CHECK(theLine.base().y() == TestType(0.0));
    CHECK(theLine.base().z() == TestType(0.0));
    CHECK(theLine.target().x() == TestType(1.0));
    CHECK(theLine.target().y() == TestType(1.0));
    CHECK(theLine.target().z() == TestType(1.0));
    CHECK(theLine.fullDirection().x() == TestType(1.0));
    CHECK(theLine.fullDirection().y() == TestType(1.0));
    CHECK(theLine.fullDirection().z() == TestType(1.0));
    CHECK(hubert::isEqual(theLine.unitDirection().x(), TestType(1.0)/TestType(std::sqrt(TestType(3.0)))));
    CHECK(hubert::isEqual(theLine.unitDirection().y(), TestType(1.0) / TestType(std::sqrt(TestType(3.0)))));
    CHECK(hubert::isEqual(theLine.unitDirection().z(), TestType(1.0) / TestType(std::sqrt(TestType(3.0)))));

    CHECK(isValid(theLine));
    CHECK_FALSE(isDegenerate(theLine));
}

TEMPLATE_TEST_CASE("Construct Line3 with constant parameters", "[Line3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Point3<TestType> p2(TestType(-7.3), TestType(3.2), TestType(-3.2));

    hubert::Line3<TestType> theLine(p1, p2);

    CHECK(theLine.base().x() == p1.x());
    CHECK(theLine.base().y() == p1.y());
    CHECK(theLine.base().z() == p1.z());
    CHECK(theLine.target().x() == p2.x());
    CHECK(theLine.target().y() == p2.y());
    CHECK(theLine.target().z() == p2.z());
    CHECK(hubert::isEqual(theLine.fullDirection().x(), p2.x() - p1.x()));
    CHECK(hubert::isEqual(theLine.fullDirection().y(), p2.y() - p1.y()));
    CHECK(hubert::isEqual(theLine.fullDirection().z(), p2.z() - p1.z()));

    TestType dist = hubert::distance(p1, p2);

    CHECK(hubert::isEqual(theLine.unitDirection().x(), theLine.fullDirection().x() / dist));
    CHECK(hubert::isEqual(theLine.unitDirection().y(), theLine.fullDirection().y() / dist));
    CHECK(hubert::isEqual(theLine.unitDirection().z(), theLine.fullDirection().z() / dist));

    CHECK(isValid(theLine));
    CHECK_FALSE(isDegenerate(theLine));
}

TEMPLATE_TEST_CASE("Construct Line3 with copy  constructor", "[Line3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Point3<TestType> p2(TestType(-7.3), TestType(3.2), TestType(-3.2));

    hubert::Line3<TestType> sourceLine(p1, p2);
    hubert::Line3<TestType> theLine(sourceLine);

    CHECK(theLine.base().x() == p1.x());
    CHECK(theLine.base().y() == p1.y());
    CHECK(theLine.base().z() == p1.z());
    CHECK(theLine.target().x() == p2.x());
    CHECK(theLine.target().y() == p2.y());
    CHECK(theLine.target().z() == p2.z());
    CHECK(hubert::isEqual(theLine.fullDirection().x(), p2.x() - p1.x()));
    CHECK(hubert::isEqual(theLine.fullDirection().y(), p2.y() - p1.y()));
    CHECK(hubert::isEqual(theLine.fullDirection().z(), p2.z() - p1.z()));

    TestType dist = hubert::distance(p1, p2);

    CHECK(hubert::isEqual(theLine.unitDirection().x(), theLine.fullDirection().x() / dist));
    CHECK(hubert::isEqual(theLine.unitDirection().y(), theLine.fullDirection().y() / dist));
    CHECK(hubert::isEqual(theLine.unitDirection().z(), theLine.fullDirection().z() / dist));

    CHECK(isValid(theLine));
    CHECK_FALSE(isDegenerate(theLine));
}

TEMPLATE_TEST_CASE("Construct Line3 with assignment", "[Line3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Point3<TestType> p2(TestType(-7.3), TestType(3.2), TestType(-3.2));

    hubert::Line3<TestType> sourceLine(p1, p2);
    hubert::Line3<TestType> theLine = sourceLine;

    CHECK(theLine.base().x() == p1.x());
    CHECK(theLine.base().y() == p1.y());
    CHECK(theLine.base().z() == p1.z());
    CHECK(theLine.target().x() == p2.x());
    CHECK(theLine.target().y() == p2.y());
    CHECK(theLine.target().z() == p2.z());
    CHECK(hubert::isEqual(theLine.fullDirection().x(), p2.x() - p1.x()));
    CHECK(hubert::isEqual(theLine.fullDirection().y(), p2.y() - p1.y()));
    CHECK(hubert::isEqual(theLine.fullDirection().z(), p2.z() - p1.z()));

    TestType dist = hubert::distance(p1, p2);

    CHECK(hubert::isEqual(theLine.unitDirection().x(), theLine.fullDirection().x() / dist));
    CHECK(hubert::isEqual(theLine.unitDirection().y(), theLine.fullDirection().y() / dist));
    CHECK(hubert::isEqual(theLine.unitDirection().z(), theLine.fullDirection().z() / dist));

    CHECK(isValid(theLine));
    CHECK_FALSE(isDegenerate(theLine));
}

TEMPLATE_TEST_CASE("Construct Line3 with initializer", "[Line3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Point3<TestType> p2(TestType(-7.3), TestType(3.2), TestType(-3.2));

    hubert::Line3<TestType> theLine{ p1, p2 };

    CHECK(theLine.base().x() == p1.x());
    CHECK(theLine.base().y() == p1.y());
    CHECK(theLine.base().z() == p1.z());
    CHECK(theLine.target().x() == p2.x());
    CHECK(theLine.target().y() == p2.y());
    CHECK(theLine.target().z() == p2.z());
    CHECK(hubert::isEqual(theLine.fullDirection().x(), p2.x() - p1.x()));
    CHECK(hubert::isEqual(theLine.fullDirection().y(), p2.y() - p1.y()));
    CHECK(hubert::isEqual(theLine.fullDirection().z(), p2.z() - p1.z()));

    TestType dist = hubert::distance(p1, p2);

    CHECK(hubert::isEqual(theLine.unitDirection().x(), theLine.fullDirection().x() / dist));
    CHECK(hubert::isEqual(theLine.unitDirection().y(), theLine.fullDirection().y() / dist));
    CHECK(hubert::isEqual(theLine.unitDirection().z(), theLine.fullDirection().z() / dist));

    CHECK(isValid(theLine));
    CHECK_FALSE(isDegenerate(theLine));
}

///////////////////////////////////////////////////////////////////////////
// Line3 validity 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Check Line3 validity routines", "[Line3]", float, double)
{
    SECTION("Valid")
    {
        for (auto x1 : gSetup.getValid<TestType>())
        {
            for (auto y1 : gSetup.getValid<TestType>())
            {
                for (auto z1 : gSetup.getValid<TestType>())
                {
                    auto x2 = y1;
                    auto y2 = z1;
                    auto z2 = x1;

                    hubert::Point3<TestType> p1{ x1, y1, z1 };
                    hubert::Point3<TestType> p2{ x2, y2, z2 };

                    hubert::Line3<TestType> theLine(p1, p2);

                    // regardless of the validity, the input data should be preserved
                    CHECK(theLine.base().x() == p1.x());
                    CHECK(theLine.base().y() == p1.y());
                    CHECK(theLine.base().z() == p1.z());
                    CHECK(theLine.target().x() == p2.x());
                    CHECK(theLine.target().y() == p2.y());
                    CHECK(theLine.target().z() == p2.z());

                    CHECK(theLine.amValid() == true);
                    if (hubert::isEqual(hubert::distance(p1, p2), TestType(0.0)))
                    {
                        CHECK(theLine.amDegenerate() == true);
                    }
                    else
                    {
                        if (hubert::isValid(hubert::distance(p1, p2)))
                        {
                            CHECK_FALSE(theLine.amDegenerate());
                        }
                        else
                        {
                            CHECK(theLine.amDegenerate());
                        }
                    }

                    // we have already run the isValid test - so assuming it works, 
                    // this does too
                    bool tv = hubert::isSubnormal(p1) || hubert::isSubnormal(p2);
                    CHECK(theLine.amSubnormal() == tv);

                    // if we are returning a degenerate line, it must return invalid
                    // directions
                    if (theLine.amDegenerate())
                    {
                        CHECK(hubert::isDegenerate(theLine.unitDirection()));
                    }
                }
            }
        }
    }

    SECTION("Invalid")
    {
        for (auto x1 : gSetup.getInvalid<TestType>())
        {
            for (auto y1 : gSetup.getInvalid<TestType>())
            {
                for (auto z1 : gSetup.getInvalid<TestType>())
                {
                    for (auto x2 : gSetup.getInvalid<TestType>())
                    {
                        for (auto y2 : gSetup.getInvalid<TestType>())
                        {
                            for (auto z2 : gSetup.getInvalid<TestType>())
                            {
                                hubert::Point3<TestType> p1{ x1, y1, z1 };
                                hubert::Point3<TestType> p2{ x2, y2, z2 };

                                hubert::Line3<TestType> theLine(p1, p2);

                                // regardless of the validity, the input data should be preserved

                                CHECK(((std::isnan(theLine.base().x()) && std::isnan(p1.x())) || (theLine.base().x() == p1.x())));
                                CHECK(((std::isnan(theLine.base().y()) && std::isnan(p1.y())) || (theLine.base().y() == p1.y())));
                                CHECK(((std::isnan(theLine.base().z()) && std::isnan(p1.z())) || (theLine.base().z() == p1.z())));
                                CHECK(((std::isnan(theLine.target().x()) && std::isnan(p2.x())) || (theLine.target().x() == p2.x())));
                                CHECK(((std::isnan(theLine.target().y()) && std::isnan(p2.y())) || (theLine.target().y() == p2.y())));
                                CHECK(((std::isnan(theLine.target().z()) && std::isnan(p2.z())) || (theLine.target().z() == p2.z())));

                                CHECK(theLine.amValid() == false);
                                CHECK(theLine.amDegenerate() == true);
                                CHECK(theLine.amSubnormal() == false);

                                CHECK_FALSE(hubert::isValid(theLine.unitDirection()));
                                CHECK_FALSE(hubert::isValid(theLine.fullDirection()));
                            }
                        }
                    }
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// Line3 specific degeneracy tests 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Line3 specific degeneracy", "[Line3]", float, double)
{
    SECTION("Zero length")
    {
        hubert::Point3<TestType> p1(TestType(1.0), TestType(1.0), TestType(1.0));
        hubert::Point3<TestType> p2(TestType(1.0), TestType(1.0), TestType(1.0));

        hubert::Line3<TestType> theLine(p1, p2);

        CHECK(theLine.amDegenerate());
        CHECK(hubert::isDegenerate(theLine));
    }

    SECTION("Within epsilon of Zero length")
    {
        hubert::Point3<TestType> p1(TestType(1.0), TestType(1.0) + hubert::epsilon<TestType>(), TestType(1.0));
        hubert::Point3<TestType> p2(TestType(1.0), TestType(1.0), TestType(1.0));

        hubert::Line3<TestType> theLine(p1, p2);

        CHECK(theLine.amDegenerate());
        CHECK(hubert::isDegenerate(theLine));
    }

    SECTION("Just over zero length")
    {
        hubert::Point3<TestType> p1(TestType(1.0), TestType(1.0) + hubert::epsilon<TestType>() * TestType(2.0), TestType(1.0));
        hubert::Point3<TestType> p2(TestType(1.0) + hubert::epsilon<TestType>() * TestType(2.0), TestType(1.0), TestType(1.0));

        hubert::Line3<TestType> theLine(p1, p2);

        CHECK_FALSE(theLine.amDegenerate());
        CHECK_FALSE(hubert::isDegenerate(theLine));
    }


    SECTION("Very large - sure to overflow")
    {
        hubert::Point3<TestType> p1(std::numeric_limits<TestType>::max(), std::numeric_limits<TestType>::max(), std::numeric_limits<TestType>::max());
        hubert::Point3<TestType> p2(-std::numeric_limits<TestType>::max(), -std::numeric_limits<TestType>::max(), -std::numeric_limits<TestType>::max());

        hubert::Line3<TestType> theLine(p1, p2);

        CHECK(theLine.amDegenerate());
        CHECK(hubert::isDegenerate(theLine));
    }

    SECTION("Just under very large - should not overflow")
    {
        hubert::Point3<TestType> p1(std::numeric_limits<TestType>::max() / TestType(4.0), std::numeric_limits<TestType>::max() / TestType(4.0), std::numeric_limits<TestType>::max() / TestType(4.0));
        hubert::Point3<TestType> p2(-std::numeric_limits<TestType>::max() / TestType(4.0), -std::numeric_limits<TestType>::max() / TestType(4.0), -std::numeric_limits<TestType>::max() / TestType(4.0));

        hubert::Line3<TestType> theLine(p1, p2);

        CHECK_FALSE(theLine.amDegenerate());
        CHECK_FALSE(hubert::isDegenerate(theLine));
    }

}


///////////////////////////////////////////////////////////////////////////
// Plane construction tests 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Construct Plane with default parameters", "[Plane]", float, double)
{
    hubert::Plane<TestType> thePlane;

    CHECK(thePlane.base().x() == TestType(0.0));
    CHECK(thePlane.base().y() == TestType(0.0));
    CHECK(thePlane.base().z() == TestType(0.0));
    CHECK(thePlane.up().x() == TestType(0.0));
    CHECK(thePlane.up().y() == TestType(0.0));
    CHECK(thePlane.up().z() == TestType(1.0));

    CHECK(isValid(thePlane));
    CHECK_FALSE(isDegenerate(thePlane));
}


TEMPLATE_TEST_CASE("Construct Plane with constant parameters", "[Plane]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Vector3<TestType> v1(TestType(-7.3), TestType(3.2), TestType(-3.2));
    hubert::UnitVector3<TestType> uv1(v1.x(), v1.y(), v1.z());

    hubert::Plane<TestType> thePlane(p1, uv1);

    CHECK(thePlane.base().x() == p1.x());
    CHECK(thePlane.base().y() == p1.y());
    CHECK(thePlane.base().z() == p1.z());

    CHECK(hubert::isEqual(thePlane.up().x(), v1.x() / v1.magnitude()));
    CHECK(hubert::isEqual(thePlane.up().y(), v1.y() / v1.magnitude()));
    CHECK(hubert::isEqual(thePlane.up().z(), v1.z() / v1.magnitude()));

    CHECK(isValid(thePlane));
    CHECK_FALSE(isDegenerate(thePlane));
}


TEMPLATE_TEST_CASE("Construct Plane with copy  constructor", "[Plane]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Vector3<TestType> v1(TestType(-7.3), TestType(3.2), TestType(-3.2));
    hubert::UnitVector3<TestType> uv1(v1.x(), v1.y(), v1.z());

    hubert::Plane<TestType> sourceLine(p1, uv1);
    hubert::Plane<TestType> thePlane(sourceLine);

    CHECK(thePlane.base().x() == p1.x());
    CHECK(thePlane.base().y() == p1.y());
    CHECK(thePlane.base().z() == p1.z());

    CHECK(hubert::isEqual(thePlane.up().x(), v1.x() / v1.magnitude()));
    CHECK(hubert::isEqual(thePlane.up().y(), v1.y() / v1.magnitude()));
    CHECK(hubert::isEqual(thePlane.up().z(), v1.z() / v1.magnitude()));

    CHECK(isValid(thePlane));
    CHECK_FALSE(isDegenerate(thePlane));
}


TEMPLATE_TEST_CASE("Construct Plane with assignment", "[Plane]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Vector3<TestType> v1(TestType(-7.3), TestType(3.2), TestType(-3.2));
    hubert::UnitVector3<TestType> uv1(v1.x(), v1.y(), v1.z());

    hubert::Plane<TestType> sourceLine(p1, uv1);
    hubert::Plane<TestType> thePlane = sourceLine;

    CHECK(thePlane.base().x() == p1.x());
    CHECK(thePlane.base().y() == p1.y());
    CHECK(thePlane.base().z() == p1.z());

    CHECK(hubert::isEqual(thePlane.up().x(), v1.x() / v1.magnitude()));
    CHECK(hubert::isEqual(thePlane.up().y(), v1.y() / v1.magnitude()));
    CHECK(hubert::isEqual(thePlane.up().z(), v1.z() / v1.magnitude()));

    CHECK(isValid(thePlane));
    CHECK_FALSE(isDegenerate(thePlane));
}


TEMPLATE_TEST_CASE("Construct Plane with initializer", "[Plane]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Vector3<TestType> v1(TestType(-7.3), TestType(3.2), TestType(-3.2));
    hubert::UnitVector3<TestType> uv1(v1.x(), v1.y(), v1.z());

    hubert::Plane<TestType> thePlane{ p1, uv1 };

    CHECK(thePlane.base().x() == p1.x());
    CHECK(thePlane.base().y() == p1.y());
    CHECK(thePlane.base().z() == p1.z());

    CHECK(hubert::isEqual(thePlane.up().x(), v1.x() / v1.magnitude()));
    CHECK(hubert::isEqual(thePlane.up().y(), v1.y() / v1.magnitude()));
    CHECK(hubert::isEqual(thePlane.up().z(), v1.z() / v1.magnitude()));

    CHECK(isValid(thePlane));
    CHECK_FALSE(isDegenerate(thePlane));
}

///////////////////////////////////////////////////////////////////////////
// Plane validity 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Check Plane validity routines", "[Plane]", float, double)
{
    SECTION("Valid")
    {
        for (auto x1 : gSetup.getValid<TestType>())
        {
            for (auto y1 : gSetup.getValid<TestType>())
            {
                for (auto z1 : gSetup.getValid<TestType>())
                {
                    auto x2 = y1;
                    auto y2 = z1;
                    auto z2 = x1;

                    hubert::Point3<TestType> p1{ x1, y1, z1 };
                    hubert::Vector3<TestType> sv1{ x2, y2, z2 };
                    hubert::UnitVector3<TestType> v1 = hubert::makeUnitVector3(sv1);

                    hubert::Plane<TestType> thePlane(p1, v1);

                    // regardless of validity, input numbers should not be modified
                    CHECK(thePlane.base().x() == p1.x());
                    CHECK(thePlane.base().y() == p1.y());
                    CHECK(thePlane.base().z() == p1.z());

                    CHECK(hubert::isEqual(thePlane.up().x(), v1.x()));
                    CHECK(hubert::isEqual(thePlane.up().y(), v1.y()));
                    CHECK(hubert::isEqual(thePlane.up().z(), v1.z()));

                    // we know the plane is valid (as opposed to degenerate) because it was
                    // directly constructed out of numbers known to be valid
                    CHECK(thePlane.amValid());

                    // if the numbers are valid, then the plane can only be degenerate if 
                    // the normal is degenerate
                    if (v1.amDegenerate())
                    {
                        CHECK(thePlane.amDegenerate());
                    }
                    else
                    {
                        CHECK_FALSE(thePlane.amDegenerate());
                    }

                    // we have already run the isValid test - so assuming it works, 
                    // this does too
                    bool tv = hubert::isSubnormal(p1) || hubert::isSubnormal(v1);
                    CHECK(thePlane.amSubnormal() == tv);
                }
            }
        }
    }

    SECTION("Invalid")
    {
        for (auto x1 : gSetup.getInvalid<TestType>())
        {
            for (auto y1 : gSetup.getInvalid<TestType>())
            {
                for (auto z1 : gSetup.getInvalid<TestType>())
                {
                    for (auto x2 : gSetup.getInvalid<TestType>())
                    {
                        for (auto y2 : gSetup.getInvalid<TestType>())
                        {
                            for (auto z2 : gSetup.getInvalid<TestType>())
                            {
                                hubert::Point3<TestType> p1{ x1, y1, z1 };
                                hubert::UnitVector3<TestType> v1{ x2, y2, z2 };

                                hubert::Plane<TestType> thePlane(p1, v1);

                                CHECK_FALSE(p1.amValid());
                                CHECK(p1.amDegenerate());

                                CHECK_FALSE(v1.amValid());
                                CHECK(v1.amDegenerate());

                                CHECK_FALSE(thePlane.amValid());
                                CHECK(thePlane.amDegenerate());
                                CHECK_FALSE(thePlane.amSubnormal());

                            }
                        }
                    }
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// Ray3 construction tests 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Construct Ray3 with default parameters", "[Ray3]", float, double)
{
    hubert::Ray3<TestType> theRay;

    CHECK(theRay.base().x() == TestType(0.0));
    CHECK(theRay.base().y() == TestType(0.0));
    CHECK(theRay.base().z() == TestType(0.0));
    CHECK(theRay.unitDirection().x() == TestType(0.0));
    CHECK(theRay.unitDirection().y() == TestType(0.0));
    CHECK(theRay.unitDirection().z() == TestType(1.0));

    CHECK(isValid(theRay));
    CHECK_FALSE(isDegenerate(theRay));
}

TEMPLATE_TEST_CASE("Construct Ray3 with constant parameters", "[Ray3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Point3<TestType> p2(TestType(-7.3), TestType(3.2), TestType(-3.2));

    hubert::Vector3<TestType> v1(p2 - p1);
    hubert::UnitVector3<TestType> uv1 = hubert::makeUnitVector3<TestType>(v1);

    hubert::Ray3<TestType> theRay(p1, uv1);

    CHECK(theRay.base().x() == p1.x());
    CHECK(theRay.base().y() == p1.y());
    CHECK(theRay.base().z() == p1.z());

    TestType dist = hubert::distance(p1, p2);

    CHECK(hubert::isEqual(theRay.unitDirection().x(), v1.x() / dist));
    CHECK(hubert::isEqual(theRay.unitDirection().y(), v1.y() / dist));
    CHECK(hubert::isEqual(theRay.unitDirection().z(), v1.z() / dist));

    CHECK(isValid(theRay));
    CHECK_FALSE(isDegenerate(theRay));
}

TEMPLATE_TEST_CASE("Construct Ray3 with copy  constructor", "[Ray3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Point3<TestType> p2(TestType(-7.3), TestType(3.2), TestType(-3.2));

    hubert::Vector3<TestType> v1(p2 - p1);
    hubert::UnitVector3<TestType> uv1 = hubert::makeUnitVector3<TestType>(v1);

    hubert::Ray3<TestType> sourceRay(p1, uv1);
    hubert::Ray3<TestType> theRay(sourceRay);

    CHECK(theRay.base().x() == p1.x());
    CHECK(theRay.base().y() == p1.y());
    CHECK(theRay.base().z() == p1.z());

    TestType dist = hubert::distance(p1, p2);

    CHECK(hubert::isEqual(theRay.unitDirection().x(), v1.x() / dist));
    CHECK(hubert::isEqual(theRay.unitDirection().y(), v1.y() / dist));
    CHECK(hubert::isEqual(theRay.unitDirection().z(), v1.z() / dist));

    CHECK(isValid(theRay));
    CHECK_FALSE(isDegenerate(theRay));
}

TEMPLATE_TEST_CASE("Construct Ray3 with assignment", "[Ray3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Point3<TestType> p2(TestType(-7.3), TestType(3.2), TestType(-3.2));

    hubert::Vector3<TestType> v1(p2 - p1);
    hubert::UnitVector3<TestType> uv1 = hubert::makeUnitVector3<TestType>(v1);

    hubert::Ray3<TestType> sourceRay(p1, uv1);
    hubert::Ray3<TestType> theRay = sourceRay;

    CHECK(theRay.base().x() == p1.x());
    CHECK(theRay.base().y() == p1.y());
    CHECK(theRay.base().z() == p1.z());

    TestType dist = hubert::distance(p1, p2);

    CHECK(hubert::isEqual(theRay.unitDirection().x(), v1.x() / dist));
    CHECK(hubert::isEqual(theRay.unitDirection().y(), v1.y() / dist));
    CHECK(hubert::isEqual(theRay.unitDirection().z(), v1.z() / dist));

    CHECK(isValid(theRay));
    CHECK_FALSE(isDegenerate(theRay));
}

TEMPLATE_TEST_CASE("Construct Ray3 with initializer", "[Ray3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Point3<TestType> p2(TestType(-7.3), TestType(3.2), TestType(-3.2));

    hubert::Vector3<TestType> v1(p2 - p1);
    hubert::UnitVector3<TestType> uv1 = hubert::makeUnitVector3<TestType>(v1);

    hubert::Ray3<TestType> theRay { p1, uv1 };

    CHECK(theRay.base().x() == p1.x());
    CHECK(theRay.base().y() == p1.y());
    CHECK(theRay.base().z() == p1.z());

    TestType dist = hubert::distance(p1, p2);

    CHECK(hubert::isEqual(theRay.unitDirection().x(), v1.x() / dist));
    CHECK(hubert::isEqual(theRay.unitDirection().y(), v1.y() / dist));
    CHECK(hubert::isEqual(theRay.unitDirection().z(), v1.z() / dist));

    CHECK(isValid(theRay));
    CHECK_FALSE(isDegenerate(theRay));
}

///////////////////////////////////////////////////////////////////////////
// Ray3 validity 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Check Ray3 validity routines", "[Ray3]", float, double)
{
    SECTION("Valid")
    {
        for (auto x1 : gSetup.getValid<TestType>())
        {
            for (auto y1 : gSetup.getValid<TestType>())
            {
                for (auto z1 : gSetup.getValid<TestType>())
                {
                    auto x2 = y1;
                    auto y2 = z1;
                    auto z2 = x1;

                    hubert::Point3<TestType> p1{ x1, y1, z1 };
                    hubert::Point3<TestType> p2{ x2, y2, z2 };

                    hubert::Vector3<TestType> v1(x2, y2, z2);
                    hubert::UnitVector3<TestType> uv1 = makeUnitVector3(v1);

                    hubert::Ray3<TestType> theRay(p1, uv1);

                    // regardless of the validity, the input data should be preserved
                    CHECK(theRay.base().x() == p1.x());
                    CHECK(theRay.base().y() == p1.y());
                    CHECK(theRay.base().z() == p1.z());

                    // we know the ray is valid (as opposed to degenerate) because it was
                    // directly constructed out of numbers known to be valid
                    CHECK(theRay.amValid());
                    if (uv1.amDegenerate())
                    {
                        CHECK(theRay.amDegenerate());
                    }
                    else
                    {
                        CHECK_FALSE(theRay.amDegenerate());
                    }

                    // we have already run the isValid test - so assuming it works, 
                    // this does too
                    bool tv = hubert::isSubnormal(p1) || hubert::isSubnormal(v1) || hubert::isSubnormal(uv1);
                    CHECK(theRay.amSubnormal() == tv);
                }
            }
        }
    }

    SECTION("Invalid")
    {
        for (auto x1 : gSetup.getInvalid<TestType>())
        {
            for (auto y1 : gSetup.getInvalid<TestType>())
            {
                for (auto z1 : gSetup.getInvalid<TestType>())
                {
                    for (auto x2 : gSetup.getInvalid<TestType>())
                    {
                        for (auto y2 : gSetup.getInvalid<TestType>())
                        {
                            for (auto z2 : gSetup.getInvalid<TestType>())
                            {
                                hubert::Point3<TestType> p1{ x1, y1, z1 };
                                hubert::UnitVector3<TestType> uv1{ x2, y2, z2 };

                                hubert::Ray3<TestType> theRay(p1, uv1);

                                // regardless of the validity, the input data should be preserved

                                CHECK(((std::isnan(theRay.base().x()) && std::isnan(p1.x())) || (theRay.base().x() == p1.x())));
                                CHECK(((std::isnan(theRay.base().y()) && std::isnan(p1.y())) || (theRay.base().y() == p1.y())));
                                CHECK(((std::isnan(theRay.base().z()) && std::isnan(p1.z())) || (theRay.base().z() == p1.z())));

                                CHECK(theRay.amValid() == false);
                                CHECK(theRay.amDegenerate() == true);
                                CHECK(theRay.amSubnormal() == false);

                                CHECK_FALSE(hubert::isValid(theRay.unitDirection()));
                            }
                        }
                    }
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// Segment3 construction tests 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Construct Segment3 with default parameters", "[Segment3]", float, double)
{
    hubert::Segment3<TestType> theSegment;

    CHECK(theSegment.base().x() == TestType(0.0));
    CHECK(theSegment.base().y() == TestType(0.0));
    CHECK(theSegment.base().z() == TestType(0.0));
    CHECK(theSegment.target().x() == TestType(1.0));
    CHECK(theSegment.target().y() == TestType(1.0));
    CHECK(theSegment.target().z() == TestType(1.0));

    CHECK(isValid(theSegment));
    CHECK_FALSE(isDegenerate(theSegment));
}


TEMPLATE_TEST_CASE("Construct Segment3 with constant parameters", "[Segment3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Point3<TestType> p2(TestType(-7.3), TestType(3.2), TestType(-3.2));

    hubert::Segment3<TestType> theSegment(p1, p2);

    CHECK(theSegment.base().x() == p1.x());
    CHECK(theSegment.base().y() == p1.y());
    CHECK(theSegment.base().z() == p1.z());
    CHECK(theSegment.target().x() == p2.x());
    CHECK(theSegment.target().y() == p2.y());
    CHECK(theSegment.target().z() == p2.z());

    CHECK(isValid(theSegment));
    CHECK_FALSE(isDegenerate(theSegment));
}

TEMPLATE_TEST_CASE("Construct Segment3 with copy  constructor", "[Segment3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Point3<TestType> p2(TestType(-7.3), TestType(3.2), TestType(-3.2));

    hubert::Segment3<TestType> sourceLine(p1, p2);
    hubert::Segment3<TestType> theSegment(sourceLine);

    CHECK(theSegment.base().x() == p1.x());
    CHECK(theSegment.base().y() == p1.y());
    CHECK(theSegment.base().z() == p1.z());
    CHECK(theSegment.target().x() == p2.x());
    CHECK(theSegment.target().y() == p2.y());
    CHECK(theSegment.target().z() == p2.z());

    CHECK(isValid(theSegment));
    CHECK_FALSE(isDegenerate(theSegment));
}

TEMPLATE_TEST_CASE("Construct Segment3 with assignment", "[Segment3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Point3<TestType> p2(TestType(-7.3), TestType(3.2), TestType(-3.2));

    hubert::Segment3<TestType> sourceLine(p1, p2);
    hubert::Segment3<TestType> theSegment = sourceLine;

    CHECK(theSegment.base().x() == p1.x());
    CHECK(theSegment.base().y() == p1.y());
    CHECK(theSegment.base().z() == p1.z());
    CHECK(theSegment.target().x() == p2.x());
    CHECK(theSegment.target().y() == p2.y());
    CHECK(theSegment.target().z() == p2.z());

    CHECK(isValid(theSegment));
    CHECK_FALSE(isDegenerate(theSegment));
}

TEMPLATE_TEST_CASE("Construct Segment3 with initializer", "[Segment3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Point3<TestType> p2(TestType(-7.3), TestType(3.2), TestType(-3.2));

    hubert::Segment3<TestType> theSegment{ p1, p2 };

    CHECK(theSegment.base().x() == p1.x());
    CHECK(theSegment.base().y() == p1.y());
    CHECK(theSegment.base().z() == p1.z());
    CHECK(theSegment.target().x() == p2.x());
    CHECK(theSegment.target().y() == p2.y());
    CHECK(theSegment.target().z() == p2.z());

    CHECK(isValid(theSegment));
    CHECK_FALSE(isDegenerate(theSegment));
}

///////////////////////////////////////////////////////////////////////////
// Segment3 validity 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Check Segment3 validity routines", "[Segment3]", float, double)
{
    SECTION("Valid")
    {
        for (auto x1 : gSetup.getValid<TestType>())
        {
            for (auto y1 : gSetup.getValid<TestType>())
            {
                for (auto z1 : gSetup.getValid<TestType>())
                {
                    auto x2 = y1;
                    auto y2 = z1;
                    auto z2 = x1;

                    hubert::Point3<TestType> p1{ x1, y1, z1 };
                    hubert::Point3<TestType> p2{ x2, y2, z2 };

                    hubert::Segment3<TestType> theSegment(p1, p2);

                    // regardless of the validity, the input data should be preserved
                    CHECK(theSegment.base().x() == p1.x());
                    CHECK(theSegment.base().y() == p1.y());
                    CHECK(theSegment.base().z() == p1.z());
                    CHECK(theSegment.target().x() == p2.x());
                    CHECK(theSegment.target().y() == p2.y());
                    CHECK(theSegment.target().z() == p2.z());

                    CHECK(theSegment.amValid() == true);

                    TestType dist = hubert::distance(p1, p2);
                    if (hubert::isEqual(dist, TestType(0.0)) || !hubert::isValid(dist))
                    {
                        CHECK(theSegment.amDegenerate());
                    }
                    else
                    {
                        CHECK_FALSE(theSegment.amDegenerate());
                    }

                    // we have already run the isValid test - so assuming it works, 
                    // this does too
                    bool tv = hubert::isSubnormal(p1) || hubert::isSubnormal(p2);
                    CHECK(theSegment.amSubnormal() == tv);
                }
            }
        }
    }

    SECTION("Invalid")
    {
        for (auto x1 : gSetup.getInvalid<TestType>())
        {
            for (auto y1 : gSetup.getInvalid<TestType>())
            {
                for (auto z1 : gSetup.getInvalid<TestType>())
                {
                    for (auto x2 : gSetup.getInvalid<TestType>())
                    {
                        for (auto y2 : gSetup.getInvalid<TestType>())
                        {
                            for (auto z2 : gSetup.getInvalid<TestType>())
                            {
                                hubert::Point3<TestType> p1{ x1, y1, z1 };
                                hubert::Point3<TestType> p2{ x2, y2, z2 };

                                hubert::Segment3<TestType> theSegment(p1, p2);

                                // regardless of the validity, the input data should be preserved

                                CHECK(((std::isnan(theSegment.base().x()) && std::isnan(p1.x())) || (theSegment.base().x() == p1.x())));
                                CHECK(((std::isnan(theSegment.base().y()) && std::isnan(p1.y())) || (theSegment.base().y() == p1.y())));
                                CHECK(((std::isnan(theSegment.base().z()) && std::isnan(p1.z())) || (theSegment.base().z() == p1.z())));
                                CHECK(((std::isnan(theSegment.target().x()) && std::isnan(p2.x())) || (theSegment.target().x() == p2.x())));
                                CHECK(((std::isnan(theSegment.target().y()) && std::isnan(p2.y())) || (theSegment.target().y() == p2.y())));
                                CHECK(((std::isnan(theSegment.target().z()) && std::isnan(p2.z())) || (theSegment.target().z() == p2.z())));

                                CHECK(theSegment.amValid() == false);
                                CHECK(theSegment.amDegenerate() == true);
                                CHECK(theSegment.amSubnormal() == false);
                            }
                        }
                    }
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// Triangle3 construction tests 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Construct Triangle3 with default parameters", "[Triangle3]", float, double)
{
    hubert::Triangle3<TestType> theTriangle;

    CHECK(theTriangle.p1().x() == TestType(0.0));
    CHECK(theTriangle.p1().y() == TestType(0.0));
    CHECK(theTriangle.p1().z() == TestType(0.0));
    CHECK(theTriangle.p2().x() == TestType(1.0));
    CHECK(theTriangle.p2().y() == TestType(0.0));
    CHECK(theTriangle.p2().z() == TestType(0.0));
    CHECK(theTriangle.p3().x() == TestType(0.0));
    CHECK(theTriangle.p3().y() == TestType(1.0));
    CHECK(theTriangle.p3().z() == TestType(0.0));

    CHECK(isValid(theTriangle));
    CHECK_FALSE(isDegenerate(theTriangle));
}

TEMPLATE_TEST_CASE("Construct Triangle3 with constant parameters", "[Triangle3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Point3<TestType> p2(TestType(-7.3), TestType(3.2), TestType(-3.2));
    hubert::Point3<TestType> p3(TestType(-8.3), TestType(-13.2), TestType(17.8));

    hubert::Triangle3<TestType> theTriangle(p1, p2, p3);

    CHECK(theTriangle.p1().x() == p1.x());
    CHECK(theTriangle.p1().y() == p1.y());
    CHECK(theTriangle.p1().z() == p1.z());
    CHECK(theTriangle.p2().x() == p2.x());
    CHECK(theTriangle.p2().y() == p2.y());
    CHECK(theTriangle.p2().z() == p2.z());
    CHECK(theTriangle.p3().x() == p3.x());
    CHECK(theTriangle.p3().y() == p3.y());
    CHECK(theTriangle.p3().z() == p3.z());

    CHECK(isValid(theTriangle));
    CHECK_FALSE(isDegenerate(theTriangle));
}


TEMPLATE_TEST_CASE("Construct Triangle3 with copy  constructor", "[Triangle3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Point3<TestType> p2(TestType(-7.3), TestType(3.2), TestType(-3.2));
    hubert::Point3<TestType> p3(TestType(-8.3), TestType(-13.2), TestType(17.8));

    hubert::Triangle3<TestType> sourceTriangle(p1, p2, p3);
    hubert::Triangle3<TestType> theTriangle(sourceTriangle);

    CHECK(theTriangle.p1().x() == p1.x());
    CHECK(theTriangle.p1().y() == p1.y());
    CHECK(theTriangle.p1().z() == p1.z());
    CHECK(theTriangle.p2().x() == p2.x());
    CHECK(theTriangle.p2().y() == p2.y());
    CHECK(theTriangle.p2().z() == p2.z());
    CHECK(theTriangle.p3().x() == p3.x());
    CHECK(theTriangle.p3().y() == p3.y());
    CHECK(theTriangle.p3().z() == p3.z());

    CHECK(isValid(theTriangle));
    CHECK_FALSE(isDegenerate(theTriangle));
}

TEMPLATE_TEST_CASE("Construct Triangle3 with assignment", "[Triangle3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Point3<TestType> p2(TestType(-7.3), TestType(3.2), TestType(-3.2));
    hubert::Point3<TestType> p3(TestType(-8.3), TestType(-13.2), TestType(17.8));

    hubert::Triangle3<TestType> sourceTriangle(p1, p2, p3);
    hubert::Triangle3<TestType> theTriangle = sourceTriangle;

    CHECK(theTriangle.p1().x() == p1.x());
    CHECK(theTriangle.p1().y() == p1.y());
    CHECK(theTriangle.p1().z() == p1.z());
    CHECK(theTriangle.p2().x() == p2.x());
    CHECK(theTriangle.p2().y() == p2.y());
    CHECK(theTriangle.p2().z() == p2.z());
    CHECK(theTriangle.p3().x() == p3.x());
    CHECK(theTriangle.p3().y() == p3.y());
    CHECK(theTriangle.p3().z() == p3.z());

    CHECK(isValid(theTriangle));
    CHECK_FALSE(isDegenerate(theTriangle));
}

TEMPLATE_TEST_CASE("Construct Triangle3 with initializer", "[Triangle3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Point3<TestType> p2(TestType(-7.3), TestType(3.2), TestType(-3.2));
    hubert::Point3<TestType> p3(TestType(-8.3), TestType(-13.2), TestType(17.8));

    hubert::Triangle3<TestType> theTriangle{ p1, p2, p3 };

    CHECK(theTriangle.p1().x() == p1.x());
    CHECK(theTriangle.p1().y() == p1.y());
    CHECK(theTriangle.p1().z() == p1.z());
    CHECK(theTriangle.p2().x() == p2.x());
    CHECK(theTriangle.p2().y() == p2.y());
    CHECK(theTriangle.p2().z() == p2.z());
    CHECK(theTriangle.p3().x() == p3.x());
    CHECK(theTriangle.p3().y() == p3.y());
    CHECK(theTriangle.p3().z() == p3.z());

    CHECK(isValid(theTriangle));
    CHECK_FALSE(isDegenerate(theTriangle));
}



///////////////////////////////////////////////////////////////////////////
// Triangle3 validity 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Check Triangle3 validity routines", "[Triangle3]", float, double)
{
    SECTION("Valid")
    {
        for (auto x1 : gSetup.getValid<TestType>())
        {
            for (auto y1 : gSetup.getValid<TestType>())
            {
                for (auto z1 : gSetup.getValid<TestType>())
                {
                    auto x2 = y1;
                    auto y2 = z1;
                    auto z2 = x1;

                    auto x3 = z1;
                    auto y3 = x1;
                    auto z3 = y1;


                    hubert::Point3<TestType> p1{ x1, y1, z1 };
                    hubert::Point3<TestType> p2{ x2, y2, z2 };
                    hubert::Point3<TestType> p3{ x3, y3, z3 };

                    hubert::Triangle3<TestType> theTriangle(p1, p2, p3);

                    // regardless of the validity, the input data should be preserved
                    CHECK(theTriangle.p1().x() == p1.x());
                    CHECK(theTriangle.p1().y() == p1.y());
                    CHECK(theTriangle.p1().z() == p1.z());
                    CHECK(theTriangle.p2().x() == p2.x());
                    CHECK(theTriangle.p2().y() == p2.y());
                    CHECK(theTriangle.p2().z() == p2.z());
                    CHECK(theTriangle.p3().x() == p3.x());
                    CHECK(theTriangle.p3().y() == p3.y());
                    CHECK(theTriangle.p3().z() == p3.z());

                    CHECK(theTriangle.amValid() == true);

                    TestType dist1 = hubert::distance(p1, p2);
                    TestType dist2 = hubert::distance(p2, p3);
                    TestType dist3 = hubert::distance(p3, p1);
                    if (hubert::isValid(dist1) && hubert::isValid(dist2) && hubert::isValid(dist3))
                    {
                        if (hubert::isEqual(dist1, TestType(0.0)) || hubert::isEqual(dist2, TestType(0.0)) || hubert::isEqual(dist3, TestType(0.0)))
                        {
                            CHECK(theTriangle.amDegenerate());
                        }
                        else
                        {
                            CHECK_FALSE(theTriangle.amDegenerate());
                        }
                    }
                    else
                    {
                        CHECK(theTriangle.amDegenerate());
                    }

                    // we have already run the isValid test - so assuming it works, 
                    // this does too
                    bool tv = hubert::isSubnormal(p1) || hubert::isSubnormal(p2);
                    CHECK(theTriangle.amSubnormal() == tv);
                }
            }
        }
    }

    SECTION("Invalid")
    {
        for (auto x1 : gSetup.getInvalid<TestType>())
        {
            for (auto y1 : gSetup.getInvalid<TestType>())
            {
                for (auto z1 : gSetup.getInvalid<TestType>())
                {
                    for (auto x2 : gSetup.getInvalid<TestType>())
                    {
                        for (auto y2 : gSetup.getInvalid<TestType>())
                        {
                            for (auto z2 : gSetup.getInvalid<TestType>())
                            {
                                for (auto x3 : gSetup.getInvalid<TestType>())
                                {
                                    for (auto y3 : gSetup.getInvalid<TestType>())
                                    {
                                        for (auto z3 : gSetup.getInvalid<TestType>())
                                        {
                                            hubert::Point3<TestType> p1{ x1, y1, z1 };
                                            hubert::Point3<TestType> p2{ x2, y2, z2 };
                                            hubert::Point3<TestType> p3{ x3, y3, z3 };

                                            hubert::Triangle3<TestType> theTriangle(p1, p2, p3);

                                            // regardless of the validity, the input data should be preserved

                                            CHECK(((std::isnan(theTriangle.p1().x()) && std::isnan(p1.x())) || (theTriangle.p1().x() == p1.x())));
                                            CHECK(((std::isnan(theTriangle.p1().y()) && std::isnan(p1.y())) || (theTriangle.p1().y() == p1.y())));
                                            CHECK(((std::isnan(theTriangle.p1().z()) && std::isnan(p1.z())) || (theTriangle.p1().z() == p1.z())));
                                            CHECK(((std::isnan(theTriangle.p2().x()) && std::isnan(p2.x())) || (theTriangle.p2().x() == p2.x())));
                                            CHECK(((std::isnan(theTriangle.p2().y()) && std::isnan(p2.y())) || (theTriangle.p2().y() == p2.y())));
                                            CHECK(((std::isnan(theTriangle.p2().z()) && std::isnan(p2.z())) || (theTriangle.p2().z() == p2.z())));
                                            CHECK(((std::isnan(theTriangle.p3().x()) && std::isnan(p3.x())) || (theTriangle.p3().x() == p3.x())));
                                            CHECK(((std::isnan(theTriangle.p3().y()) && std::isnan(p3.y())) || (theTriangle.p3().y() == p3.y())));
                                            CHECK(((std::isnan(theTriangle.p3().z()) && std::isnan(p3.z())) || (theTriangle.p3().z() == p3.z())));

                                            CHECK(theTriangle.amValid() == false);
                                            CHECK(theTriangle.amDegenerate() == true);
                                            CHECK(theTriangle.amSubnormal() == false);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

