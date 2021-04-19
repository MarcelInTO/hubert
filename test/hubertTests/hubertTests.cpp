﻿// hubertTests.cpp : Defines the entry point for the application.
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
        return std::vector<T>();
    }

    template<>
    const std::vector<float>& getInvalid()
    {
        return invalidFloat;
    }

    template<>
    const std::vector<double>& getInvalid()
    {
        return invalidDouble;
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

    template<typename T>
    const std::vector<T> & getValid() 
    { 
        return std::vector<T>(); 
    }

    template<>
    const std::vector<float>& getValid()
    {
        return validFloat;
    }

    template<>
    const std::vector<double>& getValid()
    {
        return validDouble;
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
    TestType  shrinkFactor = .75;

    SECTION("Values are positive")
    {
        TestType  f1 = TestType (10.1);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<TestType >::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<TestType >::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<TestType >::epsilon() * (f1 * TestType (2.0))) == false);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<TestType >::epsilon() * (f1 * TestType (2.0))) == false);
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

TEMPLATE_TEST_CASE("Check validity routines", "[Point3]", float, double)
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
