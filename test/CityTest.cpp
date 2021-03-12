/* Copyright 2021 J. Mark Deric */

#include <algorithm>
#include <vector>
#include "City.h"
#include "gtest/gtest.h"

/*
TEST(CityTestA, BasicFunction) {
    City c;
    c.addBldg(5, 15, 7);
    c.addBldg(2, 8, 4);
    c.addBldg(7, 10, 12);
    c.addBldg(8, 12, 9);
    City::Skyline s, expected{{2,4},{5,7},{7,12},{10,9},{12,7},{15,0}};
    c.getSkyline(s);
    ASSERT_EQ(s.size(),expected.size());
    bool same = std::equal(
        s.cbegin(), s.cend(), expected.cbegin(),
        [](const City::Point& a, const City::Point& b) {
            return (a.x_ == b.x_ && a.y_ == b.y_);
        });
    ASSERT_TRUE(same);
}
*/

class CityTest : public ::testing::Test {
 protected:
    struct BldgInit {
        int x1;
        int x2;
        int y;
    };
    using Bldgs = std::vector<BldgInit>;
    void SetupBldgs(const Bldgs& bldgs) {
        for (auto& b : bldgs) {
            c.addBldg(b.x1, b.x2, b.y);
        }
    }

    void SetExpectedSkyline(const City::Skyline& s) {
        expected = s;
    }

    void CheckSkyline() const {
        City::Skyline s = c.getSkyline();
        ASSERT_EQ(s.size(), expected.size());
        int i = 0;
        for (const City::Point& pe : expected) {
            const City::Point& p = s.front();
            ASSERT_TRUE(p.x_ == pe.x_ && p.y_ == pe.y_)
                << "at Skyline index " << i << ", expected [" << pe.x_ << ", "
                    << pe.y_ << "] was not as computed [" << p.x_ << ", "
                    << p.y_ << "]";
            s.pop_front();
            ++i;
        }
    }
    void RunCase(const Bldgs& bldgs, const City::Skyline& s) {
        SetupBldgs(bldgs);
        SetExpectedSkyline(s);
        CheckSkyline();
    }

    City c;
    City::Skyline expected;
};

TEST_F(CityTest, BasicFunction) {
    Bldgs bldgs{{5,15,7},{2,8,4},{7,10,12},{8,12,9}}; // NOLINT
    City::Skyline expected{{2,4},{5,7},{7,12},{10,9},{12,7},{15,0}}; // NOLINT
    RunCase(bldgs, expected);
}

TEST_F(CityTest, SameBldgTwice) {
    Bldgs bldgs{{2,5,7},{2,5,7}}; // NOLINT
    City::Skyline expected{{2,7},{5,0}}; // NOLINT
    RunCase(bldgs, expected);
}
