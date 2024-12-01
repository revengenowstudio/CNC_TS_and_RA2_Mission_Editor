#include "stdafx.h"
#include "../MissionEditor/IniFile.h"

class IniTestHelper
{
	std::string m_fileName;

	void writeDownContent(const char* pContent) {
		std::ofstream iniFile(m_fileName.c_str());
		ASSERT(iniFile.is_open() == true);
		iniFile << pContent;
		iniFile.flush();
		iniFile.close();
	}

public:
	IniTestHelper(std::string&& name, const char* pContent) :
		m_fileName(std::move(name))
	{
		ASSERT(!m_fileName.empty());
		ASSERT(pContent != nullptr);
		writeDownContent(pContent);
	}
	~IniTestHelper() {
		remove(m_fileName.c_str());
	}

};

TEST(CIniFileClass, LoadFileTest) {
	auto const fileName = "test.ini";
	IniTestHelper helper(fileName, R"(
[Debug]
;DisplayAllOverlay=Yes ; Doesn´t cripple the overlay list in any way
;EnableTrackLogic=Yes ; Enables Track Logic
;IgnoreSHPImageHeadUnused=Yes ; Use this *carefully* to make SHP graphics of some mods work that incorrectly have the shadow flag set
AllowTunnels=yes
AllowUnidirectionalTunnels=yes

[BuildingVoxelTurretsRA2OLD]
GTGCANX=00
GTGCANY=44;-6
)");


	CIniFile file;
	ASSERT_EQ(file.LoadFile(std::string(fileName)), 0);

	EXPECT_NE(file.Size(), 0);
	EXPECT_EQ(true, file.GetBool("Debug", "AllowTunnels"));
	EXPECT_EQ(false, file.GetBool("Debug", "DisplayAllOverlay"));
	EXPECT_EQ("00", file.GetString("BuildingVoxelTurretsRA2OLD", "GTGCANX"));
}


TEST(CIniFileClass, IniSequenceTest) {
	auto const fileName = "test.ini";
	IniTestHelper helper(fileName, R"(
[SlopeSetPiecesDirections]
Count=10
0=Right_1
1=Left_1
2=Top_1
3=Bottom_1
4=Right_2
5=Left_2
6=Left_2
7=Bottom_2
8=Top_2
9=Top_2
)");

	CIniFile file;
	ASSERT_EQ(file.LoadFile(std::string(fileName)), 0);
	EXPECT_EQ(11, file["SlopeSetPiecesDirections"].Size());
	EXPECT_EQ("10", file["SlopeSetPiecesDirections"].Nth(0).second);

}

TEST(CIniFileClass, IniSetValueTest) {
	auto const fileName = "test.ini";
	IniTestHelper helper(fileName, R"(
[Debug]
;DisplayAllOverlay=Yes ; Doesn´t cripple the overlay list in any way
;EnableTrackLogic=Yes ; Enables Track Logic
;IgnoreSHPImageHeadUnused=Yes ; Use this *carefully* to make SHP graphics of some mods work that incorrectly have the shadow flag set
AllowTunnels=yes
AllowUnidirectionalTunnels=yes
)");

	CIniFile file;
	ASSERT_EQ(file.LoadFile(std::string(fileName)), 0);
	// Test value not exists
	EXPECT_EQ(false, file.GetBool("Debug", "DisplayAllOverlay"));
	file.SetBool("Debug", "DisplayAllOverlay", true);
	EXPECT_EQ(true, file.GetBool("Debug", "DisplayAllOverlay"));
	// Test existed value and override
	EXPECT_EQ(true, file.GetBool("Debug", "AllowTunnels"));
	file.SetBool("Debug", "AllowTunnels", false);
	EXPECT_EQ(false, file.GetBool("Debug", "AllowTunnels"));
	// Test section not exists
	EXPECT_EQ("", file.GetString("SlopeSetPiecesDirections", "0"));
	file.SetString("SlopeSetPiecesDirections", "0", "Right_1");
	EXPECT_EQ("Right_1", file.GetString("SlopeSetPiecesDirections", "0"));
}

TEST(CIniFileClass, IniSetSectionTest) {
	auto const fileName = "test.ini";
	IniTestHelper helper(fileName, R"(
[SlopeSetPiecesDirections]
Count=10
0=Right_1
1=Left_1
2=Top_1
3=Bottom_1
4=Right_2
5=Left_2
6=Left_2
7=Bottom_2
8=Top_2
9=Top_2
)");

	CIniFile file;
	ASSERT_EQ(file.LoadFile(std::string(fileName)), 0);

	CIniFile anotherIni;
	anotherIni.AddSection("Debug");
	auto pDebugSecAnother = anotherIni.TryGetSection("Debug");
	ASSERT_NE(pDebugSecAnother, nullptr);
	pDebugSecAnother->SetBool("DisplayAllOverlay", false);
	pDebugSecAnother->SetBool("AllowTunnels", true);


	file.SetSection("Debug", anotherIni["Debug"]);
	EXPECT_EQ(true, file.GetBool("Debug", "AllowTunnels"));
	EXPECT_EQ(false, file.GetBool("Debug", "DisplayAllOverlay"));
}

TEST(CIniFileClass, IniAddSectionTest) {
	auto const fileName = "test.ini";
	IniTestHelper helper(fileName, R"(
[SlopeSetPiecesDirections]
Count=10
0=Right_1
1=Left_1
2=Top_1
3=Bottom_1
4=Right_2
5=Left_2
6=Left_2
7=Bottom_2
8=Top_2
9=Top_2
)");

	CIniFile file;
	ASSERT_EQ(file.LoadFile(std::string(fileName)), 0);
	file.AddSection("Debug");
	auto pDebugSec = file.TryGetSection("Debug");
	ASSERT_NE(pDebugSec, nullptr);
	pDebugSec->SetBool("DisplayAllOverlay", false);
	pDebugSec->SetBool("AllowTunnels", true);
	EXPECT_EQ(true, file.GetBool("Debug", "AllowTunnels"));
	EXPECT_EQ(false, file.GetBool("Debug", "DisplayAllOverlay"));
}

TEST(CIniFileClass, IniDeleteSectionTest) {
	auto const fileName = "test.ini";
	IniTestHelper helper(fileName, R"(
[LUNARLimits]
TreeMax=999
TreeMin=999

[URBANLimits]
TreeMax=999
;TreeMax=27

[TEMPERATELimits]
TreeMax=999
;TreeMax=27

[SNOWLimits]
TreeMax=999
;TreeMax=27

[NEWURBANLimits]
TreeMax=999
;TreeMax=27

[DESERTLimits]
TreeMin=30
TreeMax=999
)");

	CIniFile file;
	ASSERT_EQ(file.LoadFile(std::string(fileName)), 0);

	EXPECT_EQ(999, file.GetInteger("SNOWLimits", "TreeMax"));
	EXPECT_EQ(999, file.GetInteger("LUNARLimits", "TreeMin"));

	EXPECT_EQ(6, file.Size());
	file.DeleteSection("SNOWLimits");

	EXPECT_EQ(0, file.GetInteger("SNOWLimits", "TreeMax"));
	EXPECT_EQ(nullptr, file.TryGetSection("SNOWLimits"));
	EXPECT_EQ(5, file.Size());
}

TEST(CIniFileClass, IniDeleteValueTest) {
	auto const fileName = "test.ini";
	IniTestHelper helper(fileName, R"(
[LUNARLimits]
TreeMax=999
TreeMin=999

[NewUrbanInfo]
Morphable2=114
Ramps2=117
Cliffs2=110
CliffsWater2=112

; tileset ini overwritings
; only used by FinalAlert
[IgnoreSetTEMPERATE]
0=77
1=78
2=79
)");

	CIniFile file;
	ASSERT_EQ(file.LoadFile(std::string(fileName)), 0);

	EXPECT_EQ(114, file.GetInteger("NewUrbanInfo", "Morphable2"));
	file.RemoveValueByKey("NewUrbanInfo", "Morphable2");

	EXPECT_EQ(0, file.GetInteger("NewUrbanInfo", "Morphable2"));
	EXPECT_EQ(false, file["NewUrbanInfo"].Exists("Morphable2"));

}

TEST(CIniFileClass, IniLowerBoundInsertTest) {
	auto const fileName = "test.ini";
	IniTestHelper helper(fileName, R"(
[Waypoints]
0=123456
1=456123
2=123654
5=789654
6=654789
10=159357

)");

	CIniFile file;
	ASSERT_EQ(file.LoadFile(std::string(fileName)), 0);

	EXPECT_EQ(123654, file.GetInteger("Waypoints", "2"));

	auto const pSec = file.TryGetSection("Waypoints");
	EXPECT_NE(pSec, nullptr);
	auto const pos = pSec->LowerBound("4");
	EXPECT_LE(pos.first, pSec->Size());
	// not existed
	EXPECT_EQ(pos.second, false);
	pSec->InsertAt(pos.first, "4", "432156");
	EXPECT_EQ(432156, file.GetInteger("Waypoints", "4"));
	EXPECT_EQ("432156", file["Waypoints"].Nth(3).second);
	EXPECT_EQ("789654", file["Waypoints"].Nth(4).second);
	pSec->InsertOrAssign("9", "149367");
	pSec->InsertOrAssign("11", "987654");
	pSec->InsertOrAssign("10", "159356"); // existed replace
	EXPECT_EQ(149367, file.GetInteger("Waypoints", "9"));
	EXPECT_EQ(159356, file.GetInteger("Waypoints", "10"));
	EXPECT_EQ(987654, file.GetInteger("Waypoints", "11"));
	EXPECT_EQ("987654", file["Waypoints"].Nth(pSec->Size() - 1).second);
	EXPECT_EQ("159356", file["Waypoints"].Nth(pSec->Size() - 2).second);
}


TEST(CIniFileClass, IniRegistryTest) {
	auto const fileName = "test.ini";
	IniTestHelper helper(fileName, R"(
[BuildingTypes]
0=GAPOWR
1=NAPOWR
2=GACNST
5=NACNST
6=GAPOWR
6=NAFAKE


)");

	CIniFile file;
	ASSERT_EQ(file.LoadFile(std::string(fileName)), 0);

	EXPECT_EQ("NAFAKE", file.GetString("BuildingTypes", "6"));

	auto const& sec = file.GetSection("BuildingTypes");
	EXPECT_EQ(5, sec.Size());

	EXPECT_EQ("GAPOWR", sec.Nth(0).second);
	EXPECT_EQ("NAPOWR", sec.Nth(1).second);
	EXPECT_EQ("GACNST", sec.Nth(2).second);
	EXPECT_EQ("NACNST", sec.Nth(3).second);
	EXPECT_EQ("NAFAKE", sec.Nth(4).second);
}