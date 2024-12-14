#include "stdafx.h"
#include "Helpers.h"

TEST(WaypointTest, SerdeTest)
{
	//"DJ", "CL";
    ASSERT_EQ(WaypointToString(113), "DJ");
    ASSERT_EQ(StringToWaypoint("DJ"), 113);

    ASSERT_EQ(WaypointToString(0), "A");
    ASSERT_EQ(StringToWaypoint("A"), 0);

    for (auto idx = 0ull; idx < 702; ++idx) {
        cout << "Waypoint " << idx << ": " << WaypointToString(idx) << endl;
    }

}