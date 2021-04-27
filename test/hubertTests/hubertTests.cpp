// hubertTests.cpp : Defines the entry point for the application.
//


// system headers
#include <iomanip>
#include <iostream>
#include <vector>

// hubert header - that's what we are testing
#include "hubert.hpp"


///////////////////////////////////////////////////////////////////////////
//
// README - NOTES
//
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
        double(1.1e16),                                         // big number
        double(1.1e-16),                                        // little number
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



#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch_amalgamated.hpp"

///////////////////////////////////////////////////////////////////////////
//
// README - NOTES
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
}

TEMPLATE_TEST_CASE("Construct Point3 with constant parameters", "[Point3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));

    CHECK(p1.x() == TestType(1.1));
    CHECK(p1.y() == TestType(2.1));
    CHECK(p1.z() == TestType(3.1));
}


TEMPLATE_TEST_CASE("Construct Point3 with copy  constructor", "[Point3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Point3<TestType> p2(p1);

    CHECK(p2.x() == TestType(1.1));
    CHECK(p2.y() == TestType(2.1));
    CHECK(p2.z() == TestType(3.1));
}

TEMPLATE_TEST_CASE("Construct Point3 with assignment", "[Point3]", float, double)
{
    hubert::Point3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Point3<TestType> p2 = p1;

    CHECK(p2.x() == TestType(1.1));
    CHECK(p2.y() == TestType(2.1));
    CHECK(p2.z() == TestType(3.1));
}

TEMPLATE_TEST_CASE("Construct Point3 with initializer", "[Point3]", float, double)
{
    hubert::Point3<TestType> p2{ TestType(1.1), TestType(2.1), TestType(3.1) };

    CHECK(p2.x() == TestType(1.1));
    CHECK(p2.y() == TestType(2.1));
    CHECK(p2.z() == TestType(3.1));
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
// Vector3 construction tests 
///////////////////////////////////////////////////////////////////////////

TEMPLATE_TEST_CASE("Construct Vector3 with default parameters", "[Vector3]", float, double)
{
    hubert::Vector3<TestType> p1;

    CHECK(p1.x() == TestType(0.0));
    CHECK(p1.y() == TestType(0.0));
    CHECK(p1.z() == TestType(0.0));
}

TEMPLATE_TEST_CASE("Construct Vector3 with constant parameters", "[Vector3]", float, double)
{
    hubert::Vector3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));

    CHECK(p1.x() == TestType(1.1));
    CHECK(p1.y() == TestType(2.1));
    CHECK(p1.z() == TestType(3.1));
}


TEMPLATE_TEST_CASE("Construct Vector3 with copy  constructor", "[Vector3]", float, double)
{
    hubert::Vector3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Vector3<TestType> p2(p1);

    CHECK(p2.x() == TestType(1.1));
    CHECK(p2.y() == TestType(2.1));
    CHECK(p2.z() == TestType(3.1));
}

TEMPLATE_TEST_CASE("Construct Vector3 with assignment", "[Vector3]", float, double)
{
    hubert::Vector3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));
    hubert::Vector3<TestType> p2 = p1;

    CHECK(p2.x() == TestType(1.1));
    CHECK(p2.y() == TestType(2.1));
    CHECK(p2.z() == TestType(3.1));
}

TEMPLATE_TEST_CASE("Construct Vector3 with initializer", "[Vector3]", float, double)
{
    hubert::Vector3<TestType> p2{ TestType(1.1), TestType(2.1), TestType(3.1) };

    CHECK(p2.x() == TestType(1.1));
    CHECK(p2.y() == TestType(2.1));
    CHECK(p2.z() == TestType(3.1));
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
        CHECK(hubert::isEqual(p1.magnitude(), sqrt(TestType(2.1) * TestType(2.1) + TestType(3.1) * TestType(3.1) + TestType(4.1) * TestType(4.1))));
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
}

TEMPLATE_TEST_CASE("Construct UnitVector3 with constant parameters", "[UnitVector3]", float, double)
{
    hubert::UnitVector3<TestType> p1(TestType(1.1), TestType(2.1), TestType(3.1));

    TestType scale = sqrt(TestType(1.1) * TestType(1.1) + TestType(2.1) * TestType(2.1) + TestType(3.1) * TestType(3.1));
    hubert::UnitVector3<TestType> p2(p1.x() / scale, p1.y() / scale, p1.z() / scale);

    CHECK(hubert::isEqual(p1.x(), p2.x()));
    CHECK(hubert::isEqual(p1.y(), p2.y()));
    CHECK(hubert::isEqual(p1.z(), p2.z()));
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
}

TEMPLATE_TEST_CASE("Construct UnitVector3 with initializer", "[UnitVector3]", float, double)
{
    hubert::UnitVector3<TestType> p1{ TestType(1.1), TestType(2.1), TestType(3.1) };

    TestType scale = sqrt(TestType(1.1) * TestType(1.1) + TestType(2.1) * TestType(2.1) + TestType(3.1) * TestType(3.1));
    hubert::UnitVector3<TestType> p2(p1.x() / scale, p1.y() / scale, p1.z() / scale);

    CHECK(hubert::isEqual(p1.x(), p2.x()));
    CHECK(hubert::isEqual(p1.y(), p2.y()));
    CHECK(hubert::isEqual(p1.z(), p2.z()));
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

                    TestType len = std::sqrt(x * x + y * y + z * z);

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
                        TestType nlen = std::sqrt(p1.x() * p1.x() + p1.y() * p1.y() + p1.z() * p1.z());
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

                    CHECK(theLine.amValid() == true);
                    if (hubert::isEqual(hubert::distance(p1, p2), TestType(0.0)))
                    {
                        CHECK(theLine.amDegenerate() == true);
                    }
                    else
                    {
                        CHECK(theLine.amDegenerate() == false);
                    }

                    // we have already run the isValid test - so assuming it works, 
                    // this does too
                    bool tv = hubert::isSubnormal(p1) || hubert::isSubnormal(p2);
                    CHECK(theLine.amSubnormal() == tv);

                    // if we are returning a degenerate line, it must return invalid
                    // directions
                    if (theLine.amDegenerate())
                    {
                        CHECK_FALSE(hubert::isValid(theLine.unitDirection()));
                        CHECK_FALSE(hubert::isValid(theLine.fullDirection()));
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
