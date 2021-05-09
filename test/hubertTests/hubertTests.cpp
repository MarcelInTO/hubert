// hubertTests.cpp : Defines the entry point for the application.
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

                    CHECK(p1.amValid());
                    CHECK(hubert::isValid(p1));

                    CHECK_FALSE(p1.amDegenerate());
                    CHECK_FALSE(hubert::isDegenerate(p1));

                    // we have already run the isValid test - so assuming it works, 
                    // this does too
                    bool tv = hubert::isSubnormal(x) || hubert::isSubnormal(y) || hubert::isSubnormal(z);
                    CHECK(p1.amSubnormal() == tv);
                    CHECK(hubert::isSubnormal(p1) == tv);
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

                    CHECK_FALSE(p1.amValid());
                    CHECK_FALSE(hubert::isValid(p1));

                    CHECK(p1.amDegenerate());
                    CHECK(hubert::isDegenerate(p1));

                    CHECK_FALSE(p1.amSubnormal());
                    CHECK_FALSE(hubert::isSubnormal(p1));
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

    CHECK(p1.amValid());
    CHECK(isValid(p1));

    CHECK_FALSE(p1.amDegenerate());
    CHECK_FALSE(isDegenerate(p1));
}

TEMPLATE_TEST_CASE("Construct Vector3 with constant parameters", "[Vector3]", float, double)
{
    hubert::Vector3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));

    CHECK(p1.x() == TestType(1.1));
    CHECK(p1.y() == TestType(2.1));
    CHECK(p1.z() == TestType(3.1));

    CHECK(p1.amValid());
    CHECK(isValid(p1));

    CHECK_FALSE(p1.amDegenerate());
    CHECK_FALSE(isDegenerate(p1));
}


TEMPLATE_TEST_CASE("Construct Vector3 with copy  constructor", "[Vector3]", float, double)
{
    hubert::Vector3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Vector3<TestType> p2(p1);

    CHECK(p2.x() == TestType(1.1));
    CHECK(p2.y() == TestType(2.1));
    CHECK(p2.z() == TestType(3.1));

    CHECK(p2.amValid());
    CHECK(isValid(p2));

    CHECK_FALSE(p2.amDegenerate());
    CHECK_FALSE(isDegenerate(p2));
}

TEMPLATE_TEST_CASE("Construct Vector3 with assignment", "[Vector3]", float, double)
{
    hubert::Vector3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Vector3<TestType> p2 = p1;

    CHECK(p2.x() == TestType(1.1));
    CHECK(p2.y() == TestType(2.1));
    CHECK(p2.z() == TestType(3.1));

    CHECK(p2.amValid());
    CHECK(isValid(p2));

    CHECK_FALSE(p2.amDegenerate());
    CHECK_FALSE(isDegenerate(p2));
}

TEMPLATE_TEST_CASE("Construct Vector3 with initializer", "[Vector3]", float, double)
{
    hubert::Vector3<TestType> p2{ TestType(1.1), TestType(2.1), TestType(3.1) };

    CHECK(p2.x() == TestType(1.1));
    CHECK(p2.y() == TestType(2.1));
    CHECK(p2.z() == TestType(3.1));

    CHECK(p2.amValid());
    CHECK(isValid(p2));

    CHECK_FALSE(p2.amDegenerate());
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

                    CHECK(p1.amValid());
                    CHECK(isValid(p1));

                    CHECK_FALSE(p1.amDegenerate());
                    CHECK_FALSE(isDegenerate(p1));

                    // we have already run the isValid test - so assuming it works, 
                    // this does too
                    bool tv = hubert::isSubnormal(x) || hubert::isSubnormal(y) || hubert::isSubnormal(z);
                    CHECK(p1.amSubnormal() == tv);
                    CHECK(isSubnormal(p1) == tv);
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

                    CHECK_FALSE(p1.amValid());
                    CHECK_FALSE(isValid(p1));

                    CHECK(p1.amDegenerate());
                    CHECK(isDegenerate(p1));

                    CHECK_FALSE(p1.amSubnormal());
                    CHECK_FALSE(isSubnormal(p1));
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
                        CHECK(p1.amValid());
                        CHECK(isValid(p1));

                        CHECK(p1.amDegenerate());
                        CHECK(isDegenerate(p1));

                    }
                    else if (len == hubert::infinity<TestType>())
                    {
                        CHECK(p1.amValid());
                        CHECK(isValid(p1));

                        CHECK(p1.amDegenerate());
                        CHECK(isDegenerate(p1));
                    }
                    else
                    {
                        TestType nlen = std::hypot(p1.x(), p1.y(), + p1.z());

                        CHECK(hubert::isEqual(nlen, TestType(1.0)));

                        CHECK(p1.amValid());
                        CHECK(isValid(p1));

                        CHECK_FALSE(p1.amDegenerate());
                        CHECK_FALSE(isDegenerate(p1));
                    }

                    // we have already run the isValid test - so assuming it works, 
                    // this does too
                    bool tv = hubert::isSubnormal(x) || hubert::isSubnormal(y) || hubert::isSubnormal(z);
                    bool tv2 = hubert::isSubnormal(x/len) || hubert::isSubnormal(y/len) || hubert::isSubnormal(z/len);
                    CHECK(p1.amSubnormal() == (tv || tv2));
                    CHECK(isSubnormal(p1) == (tv || tv2));
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

                    CHECK_FALSE(p1.amValid());
                    CHECK_FALSE(isValid(p1));

                    CHECK(p1.amDegenerate());
                    CHECK(isDegenerate(p1));

                    CHECK(p1.amSubnormal() == false);
                    CHECK_FALSE(isSubnormal(p1));
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
// Matrix3 construction tests 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Construct Matrix3 with default parameters", "[Matrix3]", float, double)
{
    hubert::Matrix3<TestType> theMatrix;

    CHECK(theMatrix.get(0, 0) == TestType(0.0));
    CHECK(theMatrix.get(0, 1) == TestType(0.0));
    CHECK(theMatrix.get(0, 2) == TestType(0.0));
    CHECK(theMatrix.get(1, 0) == TestType(0.0));
    CHECK(theMatrix.get(1, 1) == TestType(0.0));
    CHECK(theMatrix.get(1, 2) == TestType(0.0));
    CHECK(theMatrix.get(2, 0) == TestType(0.0));
    CHECK(theMatrix.get(2, 1) == TestType(0.0));
    CHECK(theMatrix.get(2, 2) == TestType(0.0));

    CHECK(hubert::isValid(theMatrix));
    CHECK_FALSE(hubert::isDegenerate(theMatrix));
}

TEMPLATE_TEST_CASE("Construct Matrix3 with constant parameters", "[Matrix3]", float, double)
{
    hubert::Matrix3<TestType> theMatrix(TestType(1.1), TestType(2.2), TestType(3.3), TestType(4.4), TestType(5.5), TestType(6.6), TestType(7.7), TestType(8.8), TestType(9.9));

    CHECK(theMatrix.get(0, 0) == TestType(1.1));
    CHECK(theMatrix.get(0, 1) == TestType(2.2));
    CHECK(theMatrix.get(0, 2) == TestType(3.3));
    CHECK(theMatrix.get(1, 0) == TestType(4.4));
    CHECK(theMatrix.get(1, 1) == TestType(5.5));
    CHECK(theMatrix.get(1, 2) == TestType(6.6));
    CHECK(theMatrix.get(2, 0) == TestType(7.7));
    CHECK(theMatrix.get(2, 1) == TestType(8.8));
    CHECK(theMatrix.get(2, 2) == TestType(9.9));

    CHECK(hubert::isValid(theMatrix));
    CHECK_FALSE(hubert::isDegenerate(theMatrix));
}


TEMPLATE_TEST_CASE("Construct Matrix3 with copy  constructor", "[Matrix3]", float, double)
{
    hubert::Matrix3<TestType> sourceMatrix(TestType(1.1), TestType(2.2), TestType(3.3), TestType(4.4), TestType(5.5), TestType(6.6), TestType(7.7), TestType(8.8), TestType(9.9));
    hubert::Matrix3<TestType> theMatrix(sourceMatrix);

    CHECK(theMatrix.get(0, 0) == TestType(1.1));
    CHECK(theMatrix.get(0, 1) == TestType(2.2));
    CHECK(theMatrix.get(0, 2) == TestType(3.3));
    CHECK(theMatrix.get(1, 0) == TestType(4.4));
    CHECK(theMatrix.get(1, 1) == TestType(5.5));
    CHECK(theMatrix.get(1, 2) == TestType(6.6));
    CHECK(theMatrix.get(2, 0) == TestType(7.7));
    CHECK(theMatrix.get(2, 1) == TestType(8.8));
    CHECK(theMatrix.get(2, 2) == TestType(9.9));

    CHECK(hubert::isValid(theMatrix));
    CHECK_FALSE(hubert::isDegenerate(theMatrix));
}



TEMPLATE_TEST_CASE("Construct Matrix3 with assignment", "[Matrix3]", float, double)
{
    hubert::Matrix3<TestType> sourceMatrix(TestType(1.1), TestType(2.2), TestType(3.3), TestType(4.4), TestType(5.5), TestType(6.6), TestType(7.7), TestType(8.8), TestType(9.9));
    hubert::Matrix3<TestType> theMatrix = sourceMatrix;

    CHECK(theMatrix.get(0, 0) == TestType(1.1));
    CHECK(theMatrix.get(0, 1) == TestType(2.2));
    CHECK(theMatrix.get(0, 2) == TestType(3.3));
    CHECK(theMatrix.get(1, 0) == TestType(4.4));
    CHECK(theMatrix.get(1, 1) == TestType(5.5));
    CHECK(theMatrix.get(1, 2) == TestType(6.6));
    CHECK(theMatrix.get(2, 0) == TestType(7.7));
    CHECK(theMatrix.get(2, 1) == TestType(8.8));
    CHECK(theMatrix.get(2, 2) == TestType(9.9));

    CHECK(hubert::isValid(theMatrix));
    CHECK_FALSE(hubert::isDegenerate(theMatrix));
}


TEMPLATE_TEST_CASE("Construct Matrix3 with initializer", "[Matrix3]", float, double)
{
    hubert::Matrix3<TestType> sourceMatrix(TestType(1.1), TestType(2.2), TestType(3.3), TestType(4.4), TestType(5.5), TestType(6.6), TestType(7.7), TestType(8.8), TestType(9.9));
    hubert::Matrix3<TestType> theMatrix { sourceMatrix };

    CHECK(theMatrix.get(0, 0) == TestType(1.1));
    CHECK(theMatrix.get(0, 1) == TestType(2.2));
    CHECK(theMatrix.get(0, 2) == TestType(3.3));
    CHECK(theMatrix.get(1, 0) == TestType(4.4));
    CHECK(theMatrix.get(1, 1) == TestType(5.5));
    CHECK(theMatrix.get(1, 2) == TestType(6.6));
    CHECK(theMatrix.get(2, 0) == TestType(7.7));
    CHECK(theMatrix.get(2, 1) == TestType(8.8));
    CHECK(theMatrix.get(2, 2) == TestType(9.9));

    CHECK(hubert::isValid(theMatrix));
    CHECK_FALSE(hubert::isDegenerate(theMatrix));
}

///////////////////////////////////////////////////////////////////////////
// Matrix3 validity 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Check Matrix3 validity routines", "[Matrix3]", float, double)
{
    SECTION("Valid")
    {
        for (auto x : gSetup.getValid<TestType>())
        {
            for (auto y : gSetup.getValid<TestType>())
            {
                for (auto z : gSetup.getValid<TestType>())
                {
                    hubert::Matrix3<TestType> theMatrix{ x, y, z, y, z, x, z, x, y };

                    CHECK(theMatrix.amValid());
                    CHECK(isValid(theMatrix));

                    CHECK_FALSE(theMatrix.amDegenerate());
                    CHECK_FALSE(isDegenerate(theMatrix));

                    // we have already run the isValid test - so assuming it works, 
                    // this does too
                    bool tv = hubert::isSubnormal(x) || hubert::isSubnormal(y) || hubert::isSubnormal(z);
                    CHECK(theMatrix.amSubnormal() == tv);
                    CHECK(isSubnormal(theMatrix) == tv);
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
                    hubert::Matrix3<TestType> theMatrix{ x, y, z, y, z, x, z, x, y };

                    CHECK_FALSE(theMatrix.amValid());
                    CHECK_FALSE(isValid(theMatrix));

                    CHECK(theMatrix.amDegenerate());
                    CHECK(isDegenerate(theMatrix));

                    CHECK_FALSE(theMatrix.amSubnormal());
                    CHECK_FALSE(isSubnormal(theMatrix));
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// Matrix3 specific degeneracy tests 
///////////////////////////////////////////////////////////////////////////

// there are currently no non-validity related degeneracy cases for Matrix3

///////////////////////////////////////////////////////////////////////////
// Matrix3 public  function 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Test transpose() function", "[Matrix3]", float, double)
{
    hubert::Matrix3<TestType> sourceMatrix(TestType(1.1), TestType(2.2), TestType(3.3), TestType(4.4), TestType(5.5), TestType(6.6), TestType(7.7), TestType(8.8), TestType(9.9));

    hubert::Matrix3<TestType> theMatrix(sourceMatrix.transpose());

    CHECK(theMatrix.get(0, 0) == sourceMatrix.get(0, 0));
    CHECK(theMatrix.get(0, 1) == sourceMatrix.get(1, 0));
    CHECK(theMatrix.get(0, 2) == sourceMatrix.get(2, 0));
    CHECK(theMatrix.get(1, 0) == sourceMatrix.get(0, 1));
    CHECK(theMatrix.get(1, 1) == sourceMatrix.get(1, 1));
    CHECK(theMatrix.get(1, 2) == sourceMatrix.get(2, 1));
    CHECK(theMatrix.get(2, 0) == sourceMatrix.get(0, 2));
    CHECK(theMatrix.get(2, 1) == sourceMatrix.get(1, 2));
    CHECK(theMatrix.get(2, 2) == sourceMatrix.get(2, 2));

    CHECK(hubert::isValid(sourceMatrix) == hubert::isValid(theMatrix));
    CHECK(hubert::isDegenerate(sourceMatrix) == hubert::isDegenerate(theMatrix));
}

TEMPLATE_TEST_CASE("Test isIdentity() function", "[Matrix3]", float, double)
{
    hubert::Matrix3<TestType> sourceMatrix1(TestType(1.1), TestType(2.2), TestType(3.3), TestType(4.4), TestType(5.5), TestType(6.6), TestType(7.7), TestType(8.8), TestType(9.9));
    hubert::Matrix3<TestType> sourceMatrix2(TestType(1.0), TestType(0.0), TestType(0.0), TestType(0.0), TestType(1.0), TestType(0.0), TestType(0.0), TestType(0.0), TestType(1.0));

    CHECK_FALSE(sourceMatrix1.isIdentity());
    CHECK(sourceMatrix2.isIdentity());
}

TEMPLATE_TEST_CASE("Test determinant() function", "[Matrix3]", float, double)
{
    SECTION("Det is 0")
    {
        hubert::Matrix3<TestType> sourceMatrix(TestType(1.1), TestType(2.2), TestType(3.3), TestType(4.4), TestType(5.5), TestType(6.6), TestType(7.7), TestType(8.8), TestType(9.9));
        CHECK(hubert::isEqualScaled(sourceMatrix.determinant(), TestType(0.0), sourceMatrix.getDeterminantEpsilonScale()));
    }
    SECTION("Det is non-0")
    {
        hubert::Matrix3<TestType> sourceMatrix(TestType(1.1), TestType(2.2), TestType(3.3), TestType(4.4), TestType(0.0), TestType(6.6), TestType(7.7), TestType(8.8), TestType(9.9));
        CHECK(hubert::isEqualScaled(sourceMatrix.determinant(), TestType(79.86), sourceMatrix.getDeterminantEpsilonScale()));
    }
}

TEMPLATE_TEST_CASE("Test multiply() function", "[Matrix3]", float, double)
{
    SECTION("Case 1")
    {
        hubert::Matrix3<TestType> matrix1(TestType(1.1), TestType(2.2), TestType(3.3), TestType(4.4), TestType(5.5), TestType(6.6), TestType(7.7), TestType(8.8), TestType(9.9));
        hubert::Matrix3<TestType> matrix2(matrix1.transpose());

        hubert::Matrix3<TestType> ret(matrix1.multiply(matrix2));

        CHECK(hubert::isEqual(ret.get(0, 0), TestType(16.94)));
        CHECK(hubert::isEqual(ret.get(0, 1), TestType(38.72)));
        CHECK(hubert::isEqual(ret.get(0, 2), TestType(60.5)));
        CHECK(hubert::isEqual(ret.get(1, 0), TestType(38.72)));
        CHECK(hubert::isEqual(ret.get(1, 1), TestType(93.17)));
        CHECK(hubert::isEqual(ret.get(1, 2), TestType(147.62)));
        CHECK(hubert::isEqual(ret.get(2, 0), TestType(60.5)));
        CHECK(hubert::isEqual(ret.get(2, 1), TestType(147.62)));
        CHECK(hubert::isEqual(ret.get(2, 2), TestType(234.74)));
    }
    SECTION("Case 2")
    {
        hubert::Matrix3<TestType> matrix1(TestType(1.1), TestType(2.2), TestType(3.3), TestType(4.4), TestType(5.5), TestType(6.6), TestType(7.7), TestType(8.8), TestType(9.9));
        hubert::Matrix3<TestType> matrix2(TestType(1.1), TestType(-2.2), TestType(3.3), TestType(-4.4), TestType(5.5), TestType(-6.6), TestType(7.7), TestType(-8.8), TestType(9.9));

        hubert::Matrix3<TestType> ret(matrix1.multiply(matrix2));

        CHECK(hubert::isEqual(ret.get(0, 0), TestType(16.94)));
        CHECK(hubert::isEqual(ret.get(0, 1), TestType(-19.36)));
        CHECK(hubert::isEqual(ret.get(0, 2), TestType(21.78)));
        CHECK(hubert::isEqual(ret.get(1, 0), TestType(31.46)));
        CHECK(hubert::isEqual(ret.get(1, 1), TestType(-37.51)));
        CHECK(hubert::isEqual(ret.get(1, 2), TestType(43.56)));
        CHECK(hubert::isEqual(ret.get(2, 0), TestType(45.98)));
        CHECK(hubert::isEqual(ret.get(2, 1), TestType(-55.66)));
        CHECK(hubert::isEqual(ret.get(2, 2), TestType(65.34)));
    }
}

///////////////////////////////////////////////////////////////////////////
// MatrixRotation3 construction tests 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Construct MatrixRotation3 with default parameters", "[MatrixRotation3]", float, double)
{
    hubert::MatrixRotation3<TestType> theMatrix;

    CHECK(theMatrix.get(0, 0) == TestType(1.0));
    CHECK(theMatrix.get(0, 1) == TestType(0.0));
    CHECK(theMatrix.get(0, 2) == TestType(0.0));
    CHECK(theMatrix.get(1, 0) == TestType(0.0));
    CHECK(theMatrix.get(1, 1) == TestType(1.0));
    CHECK(theMatrix.get(1, 2) == TestType(0.0));
    CHECK(theMatrix.get(2, 0) == TestType(0.0));
    CHECK(theMatrix.get(2, 1) == TestType(0.0));
    CHECK(theMatrix.get(2, 2) == TestType(1.0));

    CHECK(hubert::isValid(theMatrix));
    CHECK_FALSE(hubert::isDegenerate(theMatrix));
}

TEMPLATE_TEST_CASE("Construct MatrixRotation3 with constant parameters", "[MatrixRotation3]", float, double)
{
    hubert::UnitVector3<TestType> uv1(TestType(0.8911844994581091), TestType(-0.2924131506006626), TestType(-0.34682090087160805));
    hubert::UnitVector3<TestType> uv2(TestType(0.34682090087160805), TestType(0.9319903121613182), TestType(0.1054007625971222));
    hubert::UnitVector3<TestType> uv3(TestType(0.2924131506006626), TestType(-0.21421626313901312), TestType(0.9319903121613182));

    hubert::MatrixRotation3<TestType> theMatrix(uv1, uv2, uv3);

    CHECK(theMatrix.get(0, 0) == TestType(uv1.x()));
    CHECK(theMatrix.get(0, 1) == TestType(uv2.x()));
    CHECK(theMatrix.get(0, 2) == TestType(uv3.x()));
    CHECK(theMatrix.get(1, 0) == TestType(uv1.y()));
    CHECK(theMatrix.get(1, 1) == TestType(uv2.y()));
    CHECK(theMatrix.get(1, 2) == TestType(uv3.y()));
    CHECK(theMatrix.get(2, 0) == TestType(uv1.z()));
    CHECK(theMatrix.get(2, 1) == TestType(uv2.z()));
    CHECK(theMatrix.get(2, 2) == TestType(uv3.z()));

    CHECK(hubert::isValid(theMatrix));
    CHECK_FALSE(hubert::isDegenerate(theMatrix));
}

TEMPLATE_TEST_CASE("Construct MatrixRotation3 with copy  constructor", "[MatrixRotation3]", float, double)
{
    hubert::UnitVector3<TestType> uv1(TestType(0.8911844994581091), TestType(-0.2924131506006626), TestType(-0.34682090087160805));
    hubert::UnitVector3<TestType> uv2(TestType(0.34682090087160805), TestType(0.9319903121613182), TestType(0.1054007625971222));
    hubert::UnitVector3<TestType> uv3(TestType(0.2924131506006626), TestType(-0.21421626313901312), TestType(0.9319903121613182));

    hubert::MatrixRotation3<TestType> sourceMatrix(uv1, uv2, uv3);
    hubert::MatrixRotation3<TestType> theMatrix(sourceMatrix);

    CHECK(theMatrix.get(0, 0) == TestType(uv1.x()));
    CHECK(theMatrix.get(0, 1) == TestType(uv2.x()));
    CHECK(theMatrix.get(0, 2) == TestType(uv3.x()));
    CHECK(theMatrix.get(1, 0) == TestType(uv1.y()));
    CHECK(theMatrix.get(1, 1) == TestType(uv2.y()));
    CHECK(theMatrix.get(1, 2) == TestType(uv3.y()));
    CHECK(theMatrix.get(2, 0) == TestType(uv1.z()));
    CHECK(theMatrix.get(2, 1) == TestType(uv2.z()));
    CHECK(theMatrix.get(2, 2) == TestType(uv3.z()));

    CHECK(hubert::isValid(theMatrix));
    CHECK_FALSE(hubert::isDegenerate(theMatrix));
}

TEMPLATE_TEST_CASE("Construct MatrixRotation3 with assignment", "[MatrixRotation3]", float, double)
{
    hubert::UnitVector3<TestType> uv1(TestType(0.8911844994581091), TestType(-0.2924131506006626), TestType(-0.34682090087160805));
    hubert::UnitVector3<TestType> uv2(TestType(0.34682090087160805), TestType(0.9319903121613182), TestType(0.1054007625971222));
    hubert::UnitVector3<TestType> uv3(TestType(0.2924131506006626), TestType(-0.21421626313901312), TestType(0.9319903121613182));

    hubert::MatrixRotation3<TestType> sourceMatrix(uv1, uv2, uv3);
    hubert::MatrixRotation3<TestType> theMatrix = sourceMatrix;

    CHECK(theMatrix.get(0, 0) == TestType(uv1.x()));
    CHECK(theMatrix.get(0, 1) == TestType(uv2.x()));
    CHECK(theMatrix.get(0, 2) == TestType(uv3.x()));
    CHECK(theMatrix.get(1, 0) == TestType(uv1.y()));
    CHECK(theMatrix.get(1, 1) == TestType(uv2.y()));
    CHECK(theMatrix.get(1, 2) == TestType(uv3.y()));
    CHECK(theMatrix.get(2, 0) == TestType(uv1.z()));
    CHECK(theMatrix.get(2, 1) == TestType(uv2.z()));
    CHECK(theMatrix.get(2, 2) == TestType(uv3.z()));

    CHECK(hubert::isValid(theMatrix));
    CHECK_FALSE(hubert::isDegenerate(theMatrix));
}


TEMPLATE_TEST_CASE("Construct MatrixRotation3 with initializer", "[MatrixRotation3]", float, double)
{
    hubert::UnitVector3<TestType> uv1(TestType(0.8911844994581091), TestType(-0.2924131506006626), TestType(-0.34682090087160805));
    hubert::UnitVector3<TestType> uv2(TestType(0.34682090087160805), TestType(0.9319903121613182), TestType(0.1054007625971222));
    hubert::UnitVector3<TestType> uv3(TestType(0.2924131506006626), TestType(-0.21421626313901312), TestType(0.9319903121613182));

    hubert::MatrixRotation3<TestType> sourceMatrix(uv1, uv2, uv3);
    hubert::MatrixRotation3<TestType> theMatrix { sourceMatrix };

    CHECK(theMatrix.get(0, 0) == TestType(uv1.x()));
    CHECK(theMatrix.get(0, 1) == TestType(uv2.x()));
    CHECK(theMatrix.get(0, 2) == TestType(uv3.x()));
    CHECK(theMatrix.get(1, 0) == TestType(uv1.y()));
    CHECK(theMatrix.get(1, 1) == TestType(uv2.y()));
    CHECK(theMatrix.get(1, 2) == TestType(uv3.y()));
    CHECK(theMatrix.get(2, 0) == TestType(uv1.z()));
    CHECK(theMatrix.get(2, 1) == TestType(uv2.z()));
    CHECK(theMatrix.get(2, 2) == TestType(uv3.z()));

    CHECK(hubert::isValid(theMatrix));
    CHECK_FALSE(hubert::isDegenerate(theMatrix));
}


///////////////////////////////////////////////////////////////////////////
// MatrixRotation3 validity 
///////////////////////////////////////////////////////////////////////////

// matrix validity is determined by the parent Matrix3 class

///////////////////////////////////////////////////////////////////////////
// MatrixRotation3 specific degeneracy tests 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE(" MatrixRotation3 specific degeneracy test", "[MatrixRotation3]", float, double)
{
    SECTION("Valid Rotation matrix")
    {
        hubert::UnitVector3<TestType> uv1(TestType(0.8911844994581091), TestType(-0.2924131506006626), TestType(-0.34682090087160805));
        hubert::UnitVector3<TestType> uv2(TestType(0.34682090087160805), TestType(0.9319903121613182), TestType(0.1054007625971222));
        hubert::UnitVector3<TestType> uv3(TestType(0.2924131506006626), TestType(-0.21421626313901312), TestType(0.9319903121613182));

        hubert::MatrixRotation3<TestType> sourceMatrix(uv1, uv2, uv3);
        hubert::MatrixRotation3<TestType> theMatrix{ sourceMatrix };

        CHECK(theMatrix.get(0, 0) == TestType(uv1.x()));
        CHECK(theMatrix.get(0, 1) == TestType(uv2.x()));
        CHECK(theMatrix.get(0, 2) == TestType(uv3.x()));
        CHECK(theMatrix.get(1, 0) == TestType(uv1.y()));
        CHECK(theMatrix.get(1, 1) == TestType(uv2.y()));
        CHECK(theMatrix.get(1, 2) == TestType(uv3.y()));
        CHECK(theMatrix.get(2, 0) == TestType(uv1.z()));
        CHECK(theMatrix.get(2, 1) == TestType(uv2.z()));
        CHECK(theMatrix.get(2, 2) == TestType(uv3.z()));

        CHECK(hubert::isValid(theMatrix));
        CHECK_FALSE(hubert::isDegenerate(theMatrix));
    }

    SECTION("Invalid Rotation matrix - two columns same")
    {
        hubert::UnitVector3<TestType> uv1(TestType(0.8911844994581091), TestType(-0.2924131506006626), TestType(-0.34682090087160805));
        hubert::UnitVector3<TestType> uv2(TestType(0.34682090087160805), TestType(0.9319903121613182), TestType(0.1054007625971222));
        hubert::UnitVector3<TestType> uv3(TestType(0.34682090087160805), TestType(0.9319903121613182), TestType(0.1054007625971222));

        hubert::MatrixRotation3<TestType> sourceMatrix(uv1, uv2, uv3);
        hubert::MatrixRotation3<TestType> theMatrix{ sourceMatrix };

        CHECK(theMatrix.get(0, 0) == TestType(uv1.x()));
        CHECK(theMatrix.get(0, 1) == TestType(uv2.x()));
        CHECK(theMatrix.get(0, 2) == TestType(uv3.x()));
        CHECK(theMatrix.get(1, 0) == TestType(uv1.y()));
        CHECK(theMatrix.get(1, 1) == TestType(uv2.y()));
        CHECK(theMatrix.get(1, 2) == TestType(uv3.y()));
        CHECK(theMatrix.get(2, 0) == TestType(uv1.z()));
        CHECK(theMatrix.get(2, 1) == TestType(uv2.z()));
        CHECK(theMatrix.get(2, 2) == TestType(uv3.z()));

        CHECK(hubert::isValid(theMatrix));
        CHECK(hubert::isDegenerate(theMatrix));
    }
}

///////////////////////////////////////////////////////////////////////////
// MatrixRotation3 construction tests 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Test MatrixRotation3 transpose() function", "[MatrixRotation3]", float, double)
{
    hubert::UnitVector3<TestType> uv1(TestType(0.8911844994581091), TestType(-0.2924131506006626), TestType(-0.34682090087160805));
    hubert::UnitVector3<TestType> uv2(TestType(0.34682090087160805), TestType(0.9319903121613182), TestType(0.1054007625971222));
    hubert::UnitVector3<TestType> uv3(TestType(0.2924131506006626), TestType(-0.21421626313901312), TestType(0.9319903121613182));
    hubert::MatrixRotation3<TestType> sourceMatrix(uv1, uv2, uv3);

    hubert::MatrixRotation3<TestType> theMatrix(sourceMatrix.transpose());

    CHECK(hubert::isEqual(theMatrix.get(0, 0), sourceMatrix.get(0, 0)));
    CHECK(hubert::isEqual(theMatrix.get(0, 1), sourceMatrix.get(1, 0)));
    CHECK(hubert::isEqual(theMatrix.get(0, 2), sourceMatrix.get(2, 0)));
    CHECK(hubert::isEqual(theMatrix.get(1, 0), sourceMatrix.get(0, 1)));
    CHECK(hubert::isEqual(theMatrix.get(1, 1), sourceMatrix.get(1, 1)));
    CHECK(hubert::isEqual(theMatrix.get(1, 2), sourceMatrix.get(2, 1)));
    CHECK(hubert::isEqual(theMatrix.get(2, 0), sourceMatrix.get(0, 2)));
    CHECK(hubert::isEqual(theMatrix.get(2, 1), sourceMatrix.get(1, 2)));
    CHECK(hubert::isEqual(theMatrix.get(2, 2), sourceMatrix.get(2, 2)));

    CHECK(hubert::isValid(sourceMatrix) == hubert::isValid(theMatrix));
    CHECK(hubert::isDegenerate(sourceMatrix) == hubert::isDegenerate(theMatrix));
}

TEMPLATE_TEST_CASE("Test MatrixRotation3 multiply() function", "[MatrixRotation3]", float, double)
{
    SECTION("Case 1")
    {
        hubert::UnitVector3<TestType> uv1(TestType(0.8911844994581091), TestType(-0.2924131506006626), TestType(-0.34682090087160805));
        hubert::UnitVector3<TestType> uv2(TestType(0.34682090087160805), TestType(0.9319903121613182), TestType(0.1054007625971222));
        hubert::UnitVector3<TestType> uv3(TestType(0.2924131506006626), TestType(-0.21421626313901312), TestType(0.9319903121613182));
        hubert::MatrixRotation3<TestType> matrix1(uv1, uv2, uv3);
        hubert::MatrixRotation3<TestType> matrix2(matrix1.transpose());

        hubert::MatrixRotation3<TestType> ret(matrix1.multiply(matrix2));

        CHECK(hubert::isEqual(ret.get(0, 0), TestType(1.0)));
        CHECK(hubert::isEqual(ret.get(0, 1), TestType(0.0)));
        CHECK(hubert::isEqual(ret.get(0, 2), TestType(0.0)));
        CHECK(hubert::isEqual(ret.get(1, 0), TestType(0.0)));
        CHECK(hubert::isEqual(ret.get(1, 1), TestType(1.0)));
        CHECK(hubert::isEqual(ret.get(1, 2), TestType(0.0)));
        CHECK(hubert::isEqual(ret.get(2, 0), TestType(0.0)));
        CHECK(hubert::isEqual(ret.get(2, 1), TestType(0.0)));
        CHECK(hubert::isEqual(ret.get(2, 2), TestType(1.0)));
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

    CHECK(theLine.amValid());
    CHECK(isValid(theLine));

    CHECK_FALSE(theLine.amDegenerate());
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

    CHECK(theLine.amValid());
    CHECK(isValid(theLine));

    CHECK_FALSE(theLine.amDegenerate());
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

    CHECK(theLine.amValid());
    CHECK(isValid(theLine));

    CHECK_FALSE(theLine.amDegenerate());
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

    CHECK(theLine.amValid());
    CHECK(isValid(theLine));

    CHECK_FALSE(theLine.amDegenerate());
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

    CHECK(theLine.amValid());
    CHECK(isValid(theLine));

    CHECK_FALSE(theLine.amDegenerate());
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
                    CHECK(isValid(theLine));


                    if (hubert::isEqual(hubert::distance(p1, p2), TestType(0.0)))
                    {
                        CHECK(theLine.amDegenerate() == true);
                        CHECK(isDegenerate(theLine));
                    }
                    else
                    {
                        if (hubert::isValid(hubert::distance(p1, p2)))
                        {
                            CHECK_FALSE(theLine.amDegenerate());
                            CHECK_FALSE(isDegenerate(theLine));
                        }
                        else
                        {
                            CHECK(theLine.amDegenerate());
                            CHECK(isDegenerate(theLine));
                        }
                    }

                    // we have already run the isValid test - so assuming it works, 
                    // this does too
                    bool tv = hubert::isSubnormal(p1) || hubert::isSubnormal(p2);
                    CHECK(theLine.amSubnormal() == tv);
                    CHECK(isSubnormal(theLine) == tv);

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

                                CHECK_FALSE(theLine.amValid());
                                CHECK_FALSE(isValid(theLine));

                                CHECK(theLine.amDegenerate());
                                CHECK(isDegenerate(theLine));

                                CHECK_FALSE(theLine.amSubnormal());
                                CHECK_FALSE(isSubnormal(theLine));

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

    CHECK(thePlane.amValid());
    CHECK(isValid(thePlane));

    CHECK_FALSE(thePlane.amDegenerate());
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

    CHECK(thePlane.amValid());
    CHECK(isValid(thePlane));

    CHECK_FALSE(thePlane.amDegenerate());
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

    CHECK(thePlane.amValid());
    CHECK(isValid(thePlane));

    CHECK_FALSE(thePlane.amDegenerate());
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

    CHECK(thePlane.amValid());
    CHECK(isValid(thePlane));

    CHECK_FALSE(thePlane.amDegenerate());
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

    CHECK(thePlane.amValid());
    CHECK(isValid(thePlane));

    CHECK_FALSE(thePlane.amDegenerate());
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
                    CHECK(isValid(thePlane));

                    // if the numbers are valid, then the plane can only be degenerate if 
                    // the normal is degenerate
                    if (v1.amDegenerate())
                    {
                        CHECK(thePlane.amDegenerate());
                        CHECK(isDegenerate(thePlane));
                    }
                    else
                    {
                        CHECK_FALSE(thePlane.amDegenerate());
                        CHECK_FALSE(isDegenerate(thePlane));
                    }

                    // we have already run the isValid test - so assuming it works, 
                    // this does too
                    bool tv = hubert::isSubnormal(p1) || hubert::isSubnormal(v1);
                    CHECK(thePlane.amSubnormal() == tv);
                    CHECK(isSubnormal(thePlane) == tv);
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
                                CHECK_FALSE(isValid(p1));

                                CHECK(p1.amDegenerate());
                                CHECK(isDegenerate(p1));

                                CHECK_FALSE(v1.amValid());
                                CHECK_FALSE(isValid(v1));

                                CHECK(v1.amDegenerate());
                                CHECK(isDegenerate(v1));

                                CHECK_FALSE(thePlane.amValid());
                                CHECK_FALSE(isValid(thePlane));

                                CHECK(thePlane.amDegenerate());
                                CHECK(isDegenerate(thePlane));

                                CHECK_FALSE(thePlane.amSubnormal());
                                CHECK_FALSE(isSubnormal(thePlane));

                            }
                        }
                    }
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// Plane specific degeneracy tests 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Plane specific degeneracy", "[Plane]", float, double)
{
    SECTION("Zero length up vector")
    {
        hubert::Point3<TestType> base(TestType(1.0), TestType(2.1), TestType(3.2));
        hubert::UnitVector3<TestType> uv(0.0, 0.0, hubert::epsilon<TestType>());

        hubert::Plane thePlane(base, uv);

        CHECK(thePlane.amDegenerate());
        CHECK(hubert::isDegenerate(thePlane));
    }

    SECTION("Just over zero length up vector")
    {
        hubert::Point3<TestType> base(TestType(1.0), TestType(2.1), TestType(3.2));
        hubert::UnitVector3<TestType> uv(0.0, 0.0, hubert::epsilon<TestType>() * TestType(2.0));

        hubert::Plane thePlane(base, uv);

        CHECK_FALSE(thePlane.amDegenerate());
        CHECK_FALSE(hubert::isDegenerate(thePlane));
    }


    SECTION("Very large up vector")
    {
        hubert::Point3<TestType> base(TestType(1.0), TestType(2.1), TestType(3.2));
        hubert::UnitVector3<TestType> uv(std::numeric_limits<TestType>::max(), std::numeric_limits<TestType>::max(), std::numeric_limits<TestType>::max());

        hubert::Plane thePlane(base, uv);

        CHECK(thePlane.amDegenerate());
        CHECK(hubert::isDegenerate(thePlane));
    }

    SECTION("Just under very large up vector")
    {
        hubert::Point3<TestType> base(TestType(1.0), TestType(2.1), TestType(3.2));
        hubert::UnitVector3<TestType> uv(std::numeric_limits<TestType>::max() / TestType(2.0), std::numeric_limits<TestType>::max() / TestType(2.0), std::numeric_limits<TestType>::max() / TestType(2.0));

        hubert::Plane thePlane(base, uv);

        CHECK_FALSE(thePlane.amDegenerate());
        CHECK_FALSE(hubert::isDegenerate(thePlane));
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

    CHECK(theRay.amValid());
    CHECK(isValid(theRay));

    CHECK_FALSE(theRay.amDegenerate());
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

    CHECK(theRay.amValid());
    CHECK(isValid(theRay));

    CHECK_FALSE(theRay.amDegenerate());
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

    CHECK(theRay.amValid());
    CHECK(isValid(theRay));

    CHECK_FALSE(theRay.amDegenerate());
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

    CHECK(theRay.amValid());
    CHECK(isValid(theRay));

    CHECK_FALSE(theRay.amDegenerate());
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

    CHECK(theRay.amValid());
    CHECK(isValid(theRay));

    CHECK_FALSE(theRay.amDegenerate());
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
                    CHECK(isValid(theRay));

                    if (uv1.amDegenerate())
                    {
                        CHECK(theRay.amDegenerate());
                        CHECK(isDegenerate(theRay));
                    }
                    else
                    {
                        CHECK_FALSE(theRay.amDegenerate());
                        CHECK_FALSE(isDegenerate(theRay));
                    }

                    // we have already run the isValid test - so assuming it works, 
                    // this does too
                    bool tv = hubert::isSubnormal(p1) || hubert::isSubnormal(v1) || hubert::isSubnormal(uv1);
                    CHECK(theRay.amSubnormal() == tv);
                    CHECK(isSubnormal(theRay) == tv);
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

                                CHECK_FALSE(theRay.amValid());
                                CHECK_FALSE(isValid(theRay));

                                CHECK(theRay.amDegenerate());
                                CHECK(isDegenerate(theRay));

                                CHECK_FALSE(theRay.amSubnormal());
                                CHECK_FALSE(isSubnormal(theRay));

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
// Ray3 specific degeneracy tests 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Ray3 specific degeneracy", "[Ray3]", float, double)
{
    SECTION("Zero length up vector")
    {
        hubert::Point3<TestType> base(TestType(1.0), TestType(2.1), TestType(3.2));
        hubert::UnitVector3<TestType> uv(0.0, 0.0, hubert::epsilon<TestType>());

        hubert::Ray3 theRay3(base, uv);

        CHECK(theRay3.amDegenerate());
        CHECK(hubert::isDegenerate(theRay3));
    }

    SECTION("Just over zero length up vector")
    {
        hubert::Point3<TestType> base(TestType(1.0), TestType(2.1), TestType(3.2));
        hubert::UnitVector3<TestType> uv(0.0, 0.0, hubert::epsilon<TestType>() * TestType(2.0));

        hubert::Ray3 theRay3(base, uv);

        CHECK_FALSE(theRay3.amDegenerate());
        CHECK_FALSE(hubert::isDegenerate(theRay3));
    }


    SECTION("Very large up vector")
    {
        hubert::Point3<TestType> base(TestType(1.0), TestType(2.1), TestType(3.2));
        hubert::UnitVector3<TestType> uv(std::numeric_limits<TestType>::max(), std::numeric_limits<TestType>::max(), std::numeric_limits<TestType>::max());

        hubert::Ray3 theRay3(base, uv);

        CHECK(theRay3.amDegenerate());
        CHECK(hubert::isDegenerate(theRay3));
    }

    SECTION("Just under very large up vector")
    {
        hubert::Point3<TestType> base(TestType(1.0), TestType(2.1), TestType(3.2));
        hubert::UnitVector3<TestType> uv(std::numeric_limits<TestType>::max() / TestType(2.0), std::numeric_limits<TestType>::max() / TestType(2.0), std::numeric_limits<TestType>::max() / TestType(2.0));

        hubert::Ray3 theRay3(base, uv);

        CHECK_FALSE(theRay3.amDegenerate());
        CHECK_FALSE(hubert::isDegenerate(theRay3));
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

    CHECK(theSegment.amValid());
    CHECK(isValid(theSegment));

    CHECK_FALSE(theSegment.amDegenerate());
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

    CHECK(theSegment.amValid());
    CHECK(isValid(theSegment));

    CHECK_FALSE(theSegment.amDegenerate());
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

    CHECK(theSegment.amValid());
    CHECK(isValid(theSegment));

    CHECK_FALSE(theSegment.amDegenerate());
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

    CHECK(theSegment.amValid());
    CHECK(isValid(theSegment));

    CHECK_FALSE(theSegment.amDegenerate());
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

    CHECK(theSegment.amValid());
    CHECK(isValid(theSegment));

    CHECK_FALSE(theSegment.amDegenerate());
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
// Segment3 specific degeneracy tests 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Segment3 specific degeneracy", "[Segment3]", float, double)
{
    SECTION("Zero length")
    {
        hubert::Point3<TestType> p1(TestType(1.0), TestType(1.0), TestType(1.0));
        hubert::Point3<TestType> p2(TestType(1.0), TestType(1.0), TestType(1.0));

        hubert::Segment3<TestType> theSegment(p1, p2);

        CHECK(theSegment.amDegenerate());
        CHECK(hubert::isDegenerate(theSegment));
    }

    SECTION("Within epsilon of Zero length")
    {
        hubert::Point3<TestType> p1(TestType(1.0), TestType(1.0) + hubert::epsilon<TestType>(), TestType(1.0));
        hubert::Point3<TestType> p2(TestType(1.0), TestType(1.0), TestType(1.0));

        hubert::Segment3<TestType> theSegment(p1, p2);

        CHECK(theSegment.amDegenerate());
        CHECK(hubert::isDegenerate(theSegment));
    }

    SECTION("Just over zero length")
    {
        hubert::Point3<TestType> p1(TestType(1.0), TestType(1.0) + hubert::epsilon<TestType>() * TestType(2.0), TestType(1.0));
        hubert::Point3<TestType> p2(TestType(1.0) + hubert::epsilon<TestType>() * TestType(2.0), TestType(1.0), TestType(1.0));

        hubert::Segment3<TestType> theSegment(p1, p2);

        CHECK_FALSE(theSegment.amDegenerate());
        CHECK_FALSE(hubert::isDegenerate(theSegment));
    }


    SECTION("Very large - sure to overflow")
    {
        hubert::Point3<TestType> p1(std::numeric_limits<TestType>::max(), std::numeric_limits<TestType>::max(), std::numeric_limits<TestType>::max());
        hubert::Point3<TestType> p2(-std::numeric_limits<TestType>::max(), -std::numeric_limits<TestType>::max(), -std::numeric_limits<TestType>::max());

        hubert::Segment3<TestType> theSegment(p1, p2);

        CHECK(theSegment.amDegenerate());
        CHECK(hubert::isDegenerate(theSegment));
    }

    SECTION("Just under very large - should not overflow")
    {
        hubert::Point3<TestType> p1(std::numeric_limits<TestType>::max() / TestType(4.0), std::numeric_limits<TestType>::max() / TestType(4.0), std::numeric_limits<TestType>::max() / TestType(4.0));
        hubert::Point3<TestType> p2(-std::numeric_limits<TestType>::max() / TestType(4.0), -std::numeric_limits<TestType>::max() / TestType(4.0), -std::numeric_limits<TestType>::max() / TestType(4.0));

        hubert::Segment3<TestType> theSegment(p1, p2);

        CHECK_FALSE(theSegment.amDegenerate());
        CHECK_FALSE(hubert::isDegenerate(theSegment));
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

///////////////////////////////////////////////////////////////////////////
// Creation functions - (alternates to provided constructor) 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("makeVector3(Point3, Point3)", "[make]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.0), TestType(2.0), TestType(3.0));
    hubert::Point3<TestType> p2(TestType(3.5), TestType(5.5), TestType(7.5));

    hubert::Vector3<TestType> madeVector = hubert::makeVector3(p1, p2);

    CHECK(madeVector.x() == TestType(2.5));
    CHECK(madeVector.y() == TestType(3.5));
    CHECK(madeVector.z() == TestType(4.5));
}

TEMPLATE_TEST_CASE("makeVector3(UnitVector3)", "[make]", float, double)
{
    hubert::Vector3<TestType> v1(TestType(1.0), TestType(2.0), TestType(3.0));
    hubert::UnitVector3<TestType> uv1(v1.x(), v1.y(), v1.z());

    hubert::Vector3<TestType> madeVector = hubert::makeVector3(uv1);

    CHECK(hubert::isEqual(madeVector.x(), v1.x() / hubert::magnitude(v1)));
    CHECK(hubert::isEqual(madeVector.y(), v1.y() / hubert::magnitude(v1)));
    CHECK(hubert::isEqual(madeVector.z(), v1.z() / hubert::magnitude(v1)));
}

TEMPLATE_TEST_CASE("makeUnitVector3(Vector3)", "[make]", float, double)
{
    hubert::Vector3<TestType> v1(TestType(1.0), TestType(2.0), TestType(3.0));

    hubert::UnitVector3<TestType> madeVector = hubert::makeUnitVector3(v1);

    CHECK(hubert::isEqual(madeVector.x(), v1.x() / hubert::magnitude(v1)));
    CHECK(hubert::isEqual(madeVector.y(), v1.y() / hubert::magnitude(v1)));
    CHECK(hubert::isEqual(madeVector.z(), v1.z() / hubert::magnitude(v1)));
}

TEMPLATE_TEST_CASE("makeUnitVector3(Point3, Point3)", "[make]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.0), TestType(2.0), TestType(3.0));
    hubert::Point3<TestType> p2(TestType(3.5), TestType(5.5), TestType(7.5));

    hubert::Vector3<TestType> v1 = p2 - p1;

    hubert::UnitVector3<TestType> madeVector = hubert::makeUnitVector3(p1, p2);

    CHECK(hubert::isEqual(madeVector.x(), v1.x() / hubert::magnitude(v1)));
    CHECK(hubert::isEqual(madeVector.y(), v1.y() / hubert::magnitude(v1)));
    CHECK(hubert::isEqual(madeVector.z(), v1.z() / hubert::magnitude(v1)));
}

TEMPLATE_TEST_CASE("makeLine3(Point3, Vector3)", "[make]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.0), TestType(2.0), TestType(3.0));
    hubert::Vector3<TestType> v1(TestType(1.0), TestType(1.0), TestType(1.0));

    hubert::Line3<TestType> theLine = hubert::makeLine3(p1, v1);

    CHECK(hubert::isEqual(theLine.base().x(), p1.x()));
    CHECK(hubert::isEqual(theLine.base().y(), p1.y()));
    CHECK(hubert::isEqual(theLine.base().z(), p1.z()));
    CHECK(hubert::isEqual(theLine.target().x(), p1.x() + v1.x()));
    CHECK(hubert::isEqual(theLine.target().y(), p1.y() + v1.y()));
    CHECK(hubert::isEqual(theLine.target().z(), p1.z() + v1.z()));
}

TEMPLATE_TEST_CASE("makeLine3(Point3, UnitVector3)", "[make]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.0), TestType(2.0), TestType(3.0));
    hubert::Vector3<TestType> v1(TestType(1.0), TestType(1.0), TestType(1.0));
    hubert::UnitVector3<TestType> uv1 = hubert::makeUnitVector3(v1);

    hubert::Line3<TestType> theLine = hubert::makeLine3(p1, uv1);

    CHECK(hubert::isEqual(theLine.base().x(), p1.x()));
    CHECK(hubert::isEqual(theLine.base().y(), p1.y()));
    CHECK(hubert::isEqual(theLine.base().z(), p1.z()));
    CHECK(hubert::isEqual(theLine.target().x(), p1.x() + uv1.x()));
    CHECK(hubert::isEqual(theLine.target().y(), p1.y() + uv1.y()));
    CHECK(hubert::isEqual(theLine.target().z(), p1.z() + uv1.z()));
}

TEMPLATE_TEST_CASE("makePlane(Point3, Point3, Point3)", "[make]", float, double)
{
    SECTION("xy plane up")
    {
        hubert::Point3<TestType> p1(TestType(-1.0), TestType(-1.0), TestType(3.0));
        hubert::Point3<TestType> p2(TestType(1.0), TestType(-1.0), TestType(3.0));
        hubert::Point3<TestType> p3(TestType(0.0), TestType(1.0), TestType(3.0));

        hubert::Plane<TestType> thePlane = hubert::makePlane(p1, p2, p3);

        CHECK(hubert::isEqual(thePlane.base().x(), p1.x()));
        CHECK(hubert::isEqual(thePlane.base().y(), p1.y()));
        CHECK(hubert::isEqual(thePlane.base().z(), p1.z()));
        CHECK(hubert::isEqual(thePlane.up().x(), TestType(0.0)));
        CHECK(hubert::isEqual(thePlane.up().y(), TestType(0.0)));
        CHECK(hubert::isEqual(thePlane.up().z(), TestType(1.0)));
    }
    SECTION("xy plane down")
    {
        hubert::Point3<TestType> p1(TestType(-1.0), TestType(-1.0), TestType(3.0));
        hubert::Point3<TestType> p2(TestType(0.0), TestType(1.0), TestType(3.0));
        hubert::Point3<TestType> p3(TestType(1.0), TestType(-1.0), TestType(3.0));

        hubert::Plane<TestType> thePlane = hubert::makePlane(p1, p2, p3);

        CHECK(hubert::isEqual(thePlane.base().x(), p1.x()));
        CHECK(hubert::isEqual(thePlane.base().y(), p1.y()));
        CHECK(hubert::isEqual(thePlane.base().z(), p1.z()));
        CHECK(hubert::isEqual(thePlane.up().x(), TestType(0.0)));
        CHECK(hubert::isEqual(thePlane.up().y(), TestType(0.0)));
        CHECK(hubert::isEqual(thePlane.up().z(), TestType(-1.0)));
    }
    SECTION("xz plane front")
    {
        hubert::Point3<TestType> p1(TestType(-1.0), TestType(3.0), TestType(-1.0));
        hubert::Point3<TestType> p2(TestType(1.0), TestType(3.0), TestType(-1.0));
        hubert::Point3<TestType> p3(TestType(0.0), TestType(3.0), TestType(1.0));

        hubert::Plane<TestType> thePlane = hubert::makePlane(p1, p2, p3);

        CHECK(hubert::isEqual(thePlane.base().x(), p1.x()));
        CHECK(hubert::isEqual(thePlane.base().y(), p1.y()));
        CHECK(hubert::isEqual(thePlane.base().z(), p1.z()));
        CHECK(hubert::isEqual(thePlane.up().x(), TestType(0.0)));
        CHECK(hubert::isEqual(thePlane.up().y(), TestType(-1.0)));
        CHECK(hubert::isEqual(thePlane.up().z(), TestType(0.0)));
    }
    SECTION("xz plane back")
    {
        hubert::Point3<TestType> p1(TestType(-1.0), TestType(3.0), TestType(-1.0));
        hubert::Point3<TestType> p2(TestType(0.0), TestType(3.0), TestType(1.0));
        hubert::Point3<TestType> p3(TestType(1.0), TestType(3.0), TestType(-1.0));

        hubert::Plane<TestType> thePlane = hubert::makePlane(p1, p2, p3);

        CHECK(hubert::isEqual(thePlane.base().x(), p1.x()));
        CHECK(hubert::isEqual(thePlane.base().y(), p1.y()));
        CHECK(hubert::isEqual(thePlane.base().z(), p1.z()));
        CHECK(hubert::isEqual(thePlane.up().x(), TestType(0.0)));
        CHECK(hubert::isEqual(thePlane.up().y(), TestType(1.0)));
        CHECK(hubert::isEqual(thePlane.up().z(), TestType(0.0)));
    }
}

TEMPLATE_TEST_CASE("makeRay3(Point3, Point3)", "[make]", float, double)
{
    hubert::Point3<TestType> p1(TestType(-1.0), TestType(3.0), TestType(-1.0));
    hubert::Point3<TestType> p2(TestType(0.0), TestType(3.0), TestType(1.0));
    hubert::Vector3<TestType> v1 = p2 - p1;

    hubert::Ray3<TestType> madeRay = hubert::makeRay3(p1, p2);

    CHECK(hubert::isEqual(madeRay.base().x(), p1.x()));
    CHECK(hubert::isEqual(madeRay.base().y(), p1.y()));
    CHECK(hubert::isEqual(madeRay.base().z(), p1.z()));
    CHECK(hubert::isEqual(madeRay.unitDirection().x(), v1.x() / hubert::magnitude(v1)));
    CHECK(hubert::isEqual(madeRay.unitDirection().y(), v1.y() / hubert::magnitude(v1)));
    CHECK(hubert::isEqual(madeRay.unitDirection().z(), v1.z() / hubert::magnitude(v1)));
}

