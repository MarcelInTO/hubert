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
    ~HubertTestSetup() { printf("Destructor\n"); }

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

TEST_CASE("Epsilon equals float", "[epsilon]") 
{
    float shrinkFactor = .75;

    SECTION("Values are positive")
    {
        float f1 = float(10.1);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<float>::epsilon() * (f1 * float(2.0))) == false);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<float>::epsilon() * (f1 * float(2.0))) == false);
    }

    SECTION("Values are negative")
    {
        float f1 = float(-10.1);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<float>::epsilon() * (f1 * float(2.0))) == false);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<float>::epsilon() * (f1 * float(2.0))) == false);
    }

    SECTION("Values are positive and huge")
    {
        float f1 = float(10.1e17);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<float>::epsilon() * (f1 * float(2.0))) == false);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<float>::epsilon() * (f1 * float(2.0))) == false);
    }

    SECTION("Values are negative and huge")
    {
        float f1 = float(-10.1e17);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<float>::epsilon() * (f1 * float(2.0))) == false);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<float>::epsilon() * (f1 * float(2.0))) == false);
    }

    SECTION("Values are positive and tiny")
    {
        float f1 = float(10.1e-17);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<float>::epsilon() * (f1 * float(2.0))) == false);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<float>::epsilon() * (f1 * float(2.0))) == false);
    }

    SECTION("Values are negative and tiny")
    {
        float f1 = float(-10.1e-17);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<float>::epsilon() * (f1 * float(2.0))) == false);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<float>::epsilon() * (f1 * float(2.0))) == false);
    }
}

TEST_CASE("Epsilon equals double", "[epsilon]")
{
    double shrinkFactor = .75;

    SECTION("Values are positive")
    {
        double f1 = double(10.1);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<double>::epsilon() * (f1 * double(2.0))) == false);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<double>::epsilon() * (f1 * double(2.0))) == false);
    }

    SECTION("Values are negative")
    {
        double f1 = double(-10.1);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<double>::epsilon() * (f1 * double(2.0))) == false);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<double>::epsilon() * (f1 * double(2.0))) == false);
    }

    SECTION("Values are positive and huge")
    {
        double f1 = double(10.1e17);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<double>::epsilon() * (f1 * double(2.0))) == false);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<double>::epsilon() * (f1 * double(2.0))) == false);
    }

    SECTION("Values are negative and huge")
    {
        double f1 = double(-10.1e17);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<double>::epsilon() * (f1 * double(2.0))) == false);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<double>::epsilon() * (f1 * double(2.0))) == false);
    }

    SECTION("Values are positive and tiny")
    {
        double f1 = double(10.1e-17);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<double>::epsilon() * (f1 * double(2.0))) == false);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<double>::epsilon() * (f1 * double(2.0))) == false);
    }

    SECTION("Values are negative and tiny")
    {
        double f1 = double(-10.1e-17);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isEqual(f1, f1 + std::numeric_limits<double>::epsilon() * (f1 * double(2.0))) == false);
        CHECK(hubert::isEqual(f1, f1 - std::numeric_limits<double>::epsilon() * (f1 * double(2.0))) == false);
    }
}

TEST_CASE("Epsilon greater or equal float", "[epsilon]")
{
    float shrinkFactor = .75;

    SECTION ("Values are positive and equal or close to equal")
    {
        float f1 = float(10.1);
        CHECK(hubert::isGreaterOrEqual(f1, f1) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 - std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are negative and equal or close to equal")
    {
        float f1 = float(-10.1);
        CHECK(hubert::isGreaterOrEqual(f1, f1) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 - std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are huge positive and equal or close to equal")
    {
        float f1 = float(10.1e17);
        CHECK(hubert::isGreaterOrEqual(f1, f1) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 - std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are huge negative and equal or close to equal")
    {
        float f1 = float(-10.1e17);
        CHECK(hubert::isGreaterOrEqual(f1, f1) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 - std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are tiny positive and equal or close to equal")
    {
        float f1 = float(10.1e-17);
        CHECK(hubert::isGreaterOrEqual(f1, f1) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 - std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are tiny negative and equal or close to equal")
    {
        float f1 = float(-10.1e-17);
        CHECK(hubert::isGreaterOrEqual(f1, f1) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 - std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
    }

    float growFactor = 2.0;

    SECTION("Values are positive and greater")
    {
        float f1 = float(10.1);
        CHECK(hubert::isGreaterOrEqual(f1 + std::numeric_limits<float>::epsilon() * f1 * growFactor, f1) == true);
    }

    SECTION("Values are negative and greater")
    {
        float f1 = float(-10.1);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * growFactor) == true);
    }

    SECTION("Values are huge positive and greater")
    {
        float f1 = float(10.1e17);
        CHECK(hubert::isGreaterOrEqual(f1 + std::numeric_limits<float>::epsilon() * f1 * growFactor, f1) == true);
    }

    SECTION("Values are huge negative and greater")
    {
        float f1 = float(-10.1e-17);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * growFactor) == true);
    }

    SECTION("Values are tiny positive and greater")
    {
        float f1 = float(10.1e-17);
        CHECK(hubert::isGreaterOrEqual(f1 + std::numeric_limits<float>::epsilon() * f1 * growFactor, f1) == true);
    }

    SECTION("Values are tiny negative and greater")
    {
        float f1 = float(-10.1e-17);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * growFactor) == true);
    }
}


TEST_CASE("Epsilon greater or equal double", "[epsilon]")
{
    double shrinkFactor = .75;

    SECTION("Values are positive and equal or close to equal")
    {
        double f1 = double(10.1);
        CHECK(hubert::isGreaterOrEqual(f1, f1) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 - std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are negative and equal or close to equal")
    {
        double f1 = double(-10.1);
        CHECK(hubert::isGreaterOrEqual(f1, f1) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 - std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are huge positive and equal or close to equal")
    {
        double f1 = double(10.1e17);
        CHECK(hubert::isGreaterOrEqual(f1, f1) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 - std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are huge negative and equal or close to equal")
    {
        double f1 = double(-10.1e17);
        CHECK(hubert::isGreaterOrEqual(f1, f1) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 - std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are tiny positive and equal or close to equal")
    {
        double f1 = double(10.1e-17);
        CHECK(hubert::isGreaterOrEqual(f1, f1) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 - std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are tiny negative and equal or close to equal")
    {
        double f1 = double(-10.1e-17);
        CHECK(hubert::isGreaterOrEqual(f1, f1) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isGreaterOrEqual(f1, f1 - std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
    }

    double growFactor = 2.0;

    SECTION("Values are positive and greater")
    {
        double f1 = double(10.1);
        CHECK(hubert::isGreaterOrEqual(f1 + std::numeric_limits<double>::epsilon() * f1 * growFactor, f1) == true);
    }

    SECTION("Values are negative and greater")
    {
        double f1 = double(-10.1);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * growFactor) == true);
    }

    SECTION("Values are huge positive and greater")
    {
        double f1 = double(10.1e17);
        CHECK(hubert::isGreaterOrEqual(f1 + std::numeric_limits<double>::epsilon() * f1 * growFactor, f1) == true);
    }

    SECTION("Values are huge negative and greater")
    {
        double f1 = double(-10.1e-17);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * growFactor) == true);
    }

    SECTION("Values are tiny positive and greater")
    {
        double f1 = double(10.1e-17);
        CHECK(hubert::isGreaterOrEqual(f1 + std::numeric_limits<double>::epsilon() * f1 * growFactor, f1) == true);
    }

    SECTION("Values are tiny negative and greater")
    {
        double f1 = double(-10.1e-17);
        CHECK(hubert::isGreaterOrEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * growFactor) == true);
    }
}


TEST_CASE("Epsilon less or equal float", "[epsilon]")
{
    float shrinkFactor = .75;

    SECTION("Values are positive and equal or close to equal")
    {
        float f1 = float(10.1);
        CHECK(hubert::isLessOrEqual(f1, f1) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 - std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are negative and equal or close to equal")
    {
        float f1 = float(-10.1);
        CHECK(hubert::isLessOrEqual(f1, f1) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 - std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are huge positive and equal or close to equal")
    {
        float f1 = float(10.1e17);
        CHECK(hubert::isLessOrEqual(f1, f1) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 - std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are huge negative and equal or close to equal")
    {
        float f1 = float(-10.1e17);
        CHECK(hubert::isLessOrEqual(f1, f1) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 - std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are tiny positive and equal or close to equal")
    {
        float f1 = float(10.1e-17);
        CHECK(hubert::isLessOrEqual(f1, f1) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 - std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are tiny negative and equal or close to equal")
    {
        float f1 = float(-10.1e-17);
        CHECK(hubert::isLessOrEqual(f1, f1) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 - std::numeric_limits<float>::epsilon() * f1 * shrinkFactor) == true);
    }

    float growFactor = 2.0;

    SECTION("Values are positive and less")
    {
        float f1 = float(10.1);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * growFactor) == true);
    }

    SECTION("Values are negative and less")
    {
        float f1 = float(-10.1);
        CHECK(hubert::isLessOrEqual(f1 + std::numeric_limits<float>::epsilon() * f1 * growFactor, f1) == true);
    }

    SECTION("Values are huge positive and less")
    {
        float f1 = float(10.1e17);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * growFactor) == true);
    }

    SECTION("Values are huge negative and less")
    {
        float f1 = float(-10.1e-17);
        CHECK(hubert::isLessOrEqual( f1 + std::numeric_limits<float>::epsilon() * f1 * growFactor, f1) == true);
    }

    SECTION("Values are tiny positive and less")
    {
        float f1 = float(10.1e-17);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<float>::epsilon() * f1 * growFactor) == true);
    }

    SECTION("Values are tiny negative and less")
    {
        float f1 = float(-10.1e-17);
        CHECK(hubert::isLessOrEqual(f1 + std::numeric_limits<float>::epsilon() * f1 * growFactor, f1) == true);
    }
}

TEST_CASE("Epsilon less or equal double", "[epsilon]")
{
    double shrinkFactor = .75;

    SECTION("Values are positive and equal or close to equal")
    {
        double f1 = double(10.1);
        CHECK(hubert::isLessOrEqual(f1, f1) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 - std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are negative and equal or close to equal")
    {
        double f1 = double(-10.1);
        CHECK(hubert::isLessOrEqual(f1, f1) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 - std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are huge positive and equal or close to equal")
    {
        double f1 = double(10.1e17);
        CHECK(hubert::isLessOrEqual(f1, f1) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 - std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are huge negative and equal or close to equal")
    {
        double f1 = double(-10.1e17);
        CHECK(hubert::isLessOrEqual(f1, f1) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 - std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are tiny positive and equal or close to equal")
    {
        double f1 = double(10.1e-17);
        CHECK(hubert::isLessOrEqual(f1, f1) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 - std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
    }

    SECTION("Values are tiny negative and equal or close to equal")
    {
        double f1 = double(-10.1e-17);
        CHECK(hubert::isLessOrEqual(f1, f1) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
        CHECK(hubert::isLessOrEqual(f1, f1 - std::numeric_limits<double>::epsilon() * f1 * shrinkFactor) == true);
    }

    double growFactor = 2.0;

    SECTION("Values are positive and less")
    {
        double f1 = double(10.1);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * growFactor) == true);
    }

    SECTION("Values are negative and less")
    {
        double f1 = double(-10.1);
        CHECK(hubert::isLessOrEqual(f1 + std::numeric_limits<double>::epsilon() * f1 * growFactor, f1) == true);
    }

    SECTION("Values are huge positive and less")
    {
        double f1 = double(10.1e17);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * growFactor) == true);
    }

    SECTION("Values are huge negative and less")
    {
        double f1 = double(-10.1e-17);
        CHECK(hubert::isLessOrEqual(f1 + std::numeric_limits<double>::epsilon() * f1 * growFactor, f1) == true);
    }

    SECTION("Values are tiny positive and less")
    {
        double f1 = double(10.1e-17);
        CHECK(hubert::isLessOrEqual(f1, f1 + std::numeric_limits<double>::epsilon() * f1 * growFactor) == true);
    }

    SECTION("Values are tiny negative and less")
    {
        double f1 = double(-10.1e-17);
        CHECK(hubert::isLessOrEqual(f1 + std::numeric_limits<double>::epsilon() * f1 * growFactor, f1) == true);
    }
}

///////////////////////////////////////////////////////////////////////////
// Validation checks on primitive types 
///////////////////////////////////////////////////////////////////////////

TEST_CASE("Invalidity check on primitive types", "[validity]")
{
    SECTION("Values are float and results should be true")
    {
        CHECK(hubert::isValid(float(1.2)) == true);
        CHECK(hubert::isValid(float(0.0)) == true);
        CHECK(hubert::isValid(float(-0.0)) == true);
        CHECK(hubert::isValid(std::numeric_limits<float>::min() / float(2.0)) == true);
    }

    SECTION("Values are float and results should be false")
    {
        CHECK(hubert::isValid(std::numeric_limits<float>::infinity()) == false);
        CHECK(hubert::isValid(-std::numeric_limits<float>::infinity()) == false);
        CHECK(hubert::isValid(std::numeric_limits<float>::quiet_NaN()) == false);
        CHECK(hubert::isValid(-std::numeric_limits<float>::quiet_NaN()) == false);
    }

    SECTION("Values are double and results should be true")
    {
        CHECK(hubert::isValid(double(1.2)) == true);
        CHECK(hubert::isValid(double(0.0)) == true);
        CHECK(hubert::isValid(double(-0.0)) == true);
        CHECK(hubert::isValid(std::numeric_limits<double>::min() / double(2.0)) == true);
    }

    SECTION("Values are double and results should be false")
    {
        CHECK(hubert::isValid(std::numeric_limits<double>::infinity()) == false);
        CHECK(hubert::isValid(-std::numeric_limits<double>::infinity()) == false);
        CHECK(hubert::isValid(std::numeric_limits<double>::quiet_NaN()) == false);
        CHECK(hubert::isValid(-std::numeric_limits<double>::quiet_NaN()) == false);
    }
}

TEST_CASE("Subnormal  check on primitive types", "[validity]")
{
    SECTION("Values are float and results should be true")
    {
        CHECK(hubert::isSubnormal(std::numeric_limits<float>::min() / float(2.0)) == true);
    }

    SECTION("Values are float and results should be false")
    {
        CHECK(hubert::isSubnormal(float(1.2)) == false);
        CHECK(hubert::isSubnormal(float(0.0)) == false);
        CHECK(hubert::isSubnormal(float(-0.0)) == false);
        CHECK(hubert::isSubnormal(std::numeric_limits<float>::infinity()) == false);
        CHECK(hubert::isSubnormal(-std::numeric_limits<float>::infinity()) == false);
        CHECK(hubert::isSubnormal(std::numeric_limits<float>::quiet_NaN()) == false);
        CHECK(hubert::isSubnormal(-std::numeric_limits<float>::quiet_NaN()) == false);
    }

    SECTION("Values are double and results should be true")
    {
        CHECK(hubert::isSubnormal(std::numeric_limits<double>::min() / double(2.0)) == true);
    }

    SECTION("Values are double and results should be false")
    {
        CHECK(hubert::isSubnormal(double(1.2)) == false);
        CHECK(hubert::isSubnormal(double(0.0)) == false);
        CHECK(hubert::isSubnormal(double(-0.0)) == false);
        CHECK(hubert::isSubnormal(std::numeric_limits<double>::infinity()) == false);
        CHECK(hubert::isSubnormal(-std::numeric_limits<double>::infinity()) == false);
        CHECK(hubert::isSubnormal(std::numeric_limits<double>::quiet_NaN()) == false);
        CHECK(hubert::isSubnormal(-std::numeric_limits<double>::quiet_NaN()) == false);
    }
}

///////////////////////////////////////////////////////////////////////////
// Point3 construction tests 
///////////////////////////////////////////////////////////////////////////

TEST_CASE("Construct Point3<float> with default parameters", "[Point3]")
{
    hubert::Point3<float> p1;

    CHECK(p1.x() == float(0.0));
    CHECK(p1.y() == float(0.0));
    CHECK(p1.z() == float(0.0));
}

TEST_CASE("Construct Point3<double> with default parameters", "[Point3]")
{
    hubert::Point3<double> p1;

    CHECK(p1.x() == double(0.0));
    CHECK(p1.y() == double(0.0));
    CHECK(p1.z() == double(0.0));
}

TEST_CASE("Construct Point3<float> with constant parameters", "[Point3]")
{
    hubert::Point3<float> p1(float(1.1), float(2.1), float(3.1));

    CHECK(p1.x() == float(1.1));
    CHECK(p1.y() == float(2.1));
    CHECK(p1.z() == float(3.1));
}


TEST_CASE("Construct Point3<double> with constant parameters", "[Point3]")
{
    hubert::Point3<double> p1(1.1, 2.1, 3.1);

    CHECK(p1.x() == double(1.1));
    CHECK(p1.y() == double(2.1));
    CHECK(p1.z() == double(3.1));
}


TEST_CASE("Construct Point3<float> with copy  constructor", "[Point3]")
{
    hubert::Point3<float> p1(float(1.1), float(2.1), float(3.1));
    hubert::Point3<float> p2(p1);

    CHECK(p2.x() == float(1.1));
    CHECK(p2.y() == float(2.1));
    CHECK(p2.z() == float(3.1));
}

TEST_CASE("Construct Point3<double> with copy  constructor", "[Point3]")
{
    hubert::Point3<double> p1(1.1, 2.1, 3.1);
    hubert::Point3<double> p2(p1);

    CHECK(p2.x() == double(1.1));
    CHECK(p2.y() == double(2.1));
    CHECK(p2.z() == double(3.1));
}

TEST_CASE("Construct Point3<float> with assignment", "[Point3]")
{
    hubert::Point3<float> p1(float(1.1), float(2.1), float(3.1));
    hubert::Point3<float> p2 = p1;

    CHECK(p2.x() == float(1.1));
    CHECK(p2.y() == float(2.1));
    CHECK(p2.z() == float(3.1));
}

TEST_CASE("Construct Point3<double> with assignment", "[Point3]")
{
    hubert::Point3<double> p1(1.1, 2.1, 3.1);
    hubert::Point3<double> p2 = p1;

    CHECK(p2.x() == double(1.1));
    CHECK(p2.y() == double(2.1));
    CHECK(p2.z() == double(3.1));
}

TEST_CASE("Construct Point3<float> with initializer", "[Point3]")
{
    hubert::Point3<float> p2{ float(1.1), float(2.1), float(3.1) };

    CHECK(p2.x() == float(1.1));
    CHECK(p2.y() == float(2.1));
    CHECK(p2.z() == float(3.1));
}

TEST_CASE("Construct Point3<double> with initializer", "[Point3]")
{
    hubert::Point3<double> p2{ 1.1, 2.1, 3.1 };

    CHECK(p2.x() == double(1.1));
    CHECK(p2.y() == double(2.1));
    CHECK(p2.z() == double(3.1));
}