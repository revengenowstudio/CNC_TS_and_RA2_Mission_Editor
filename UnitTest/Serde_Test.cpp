
#include "stdafx.h"
#include "MissionEditorPackLib.h"

using std::cout;
using std::endl;

#if 0
void printBin(const std::span<BYTE> input, const char* fileName)
{
	std::ofstream file(fileName);
	std::ostringstream line_stream;
	size_t count = 0;

	for (const auto& byte : input) {
		// to hex
		line_stream << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(byte) << " ";
		count++;

		// switch line every 16 bytes
		if (count % 16 == 0) {
			file << line_stream.str() << std::endl;
			line_stream.str(""); // clear stream
			line_stream.clear(); // reset stream
		}
	}

	// for the rest of bytes
	if (count % 16 != 0) {
		file << line_stream.str() << std::endl;
	}

	file.close();
}
#endif


TEST(SerdeTest, OverlayDataPackSerde)
{
	static const char* rawInput[] = {
		R"(1=BQAAIP4AIACAeAAAIP5iDACBBP4EAAeBBv76AQCHBwsLCgoIB/75AQCHBgoLCgAKCv/6AW)",
		R"(2=cMgQgEAIMKCwj//AFpDP4DAAsGA//3AWkQMgAyA4EE//YBbw7+CQAL//UBaA6BCEP///gB)",
		R"(3=ZhLBYgyBCCwGgQv/+gFoEIEH/gQACv+SAW0YgIkAACD+WgAAgQP+DwAAgQj+BQALgQf+6A)",
		R"(4=EAhQQEAAIM/gwAAIEH/+4BawCBBv4DAACBBf/8AV4ChAgGAgnMXgSECggHBP/qAXEEzG4G)",
		R"(5=gwQHBxQU/ukBAIMHAAr/6QFwBNJFCIIKCv/9AV0KhAQICgj+/gEAggYGzVoA/usLAMtcAv)",
		R"(6=7zAQDKXQT+mQEAgFIAACD+VAAAgQT+BAAAggIJ/vkBAIEF/vsBAIEC/gMACv/6AVoA/v8F)",
		R"(7=AIMGCgz+/QEAgwUAAf//AVAM/v4BAP8AAk0QhAMKCgj+DwQAwlQA/5sHUg6AuwAAIP5UAg)",
		R"(8=CEBgoKDP78AQCBBSAD//QBYwAz+P/3AVQEEfQh+IQDCgoJ//wBWAb/+gFdAoEC//QBVwj+)",
		R"(9=CAAAgwEABP/mAWYE/gMAB4EG1jYK/+MBagT+AwALggoH0zcO/uMBAIEH/gUAC9M3Ev/qAU)",
		R"(10=4SUgH+BwAB//IBRxSCCwooBP4HAAL+EQAAgQL/2QFXAlIAgQvbORT/2QFNEIIECBoB/gMA)",
		R"(11=C4EI//YBPBqDAAAIyTQYziIcRgD/pwFPDIAVAQAg/icAAIMGCgzJHQD+BgALgQj+FAAAgQ)",
		R"(12=X+BAAAgQT+zwEAhAYJAAPIKQCBCP4GAAuCCgbUPgCEAgkGBv/PAVoAggAAyCkCggQINAKB)",
		R"(13=B/4TAACCAwoEF4UGCgoIB//JAVYAAf8B0YIKCMkpAP4FAAfWWwSBB9U5BP+4AXECJ9bJKQ)",
		R"(14=L+HgAAgQj+AwAK/8kBWgYR/CIA5isIBiCBAP/KAVkIa9YAB4EC4i4KggQHBAH/ywFaCoED)",
		R"(15=CdYIABYKgQHiAQ6CCAjUOgL/tAFwBv4GAAoD+YEG51sEwDQA0jwA/7cBdAqBAxAHggYJRg)",
		R"(16=biMxD/yQFaBGf7ggoKwSYE/vEBAFIA//UBLRTCTgT+BQAK//gBKhb+3wUAgAUAACD+ACAA)",
		R"(17=gAUAACD+ACAAgL4AACD+KAAAgQT+/QEAhAEGBwf+/gEAhggKCAYEAf76AQCHBwsKBwYHBv)",
		R"(18=/6AS8EhAgHBgoCBP73AQCDBAAIBAOCCwv/+AEuCIIEB/4DAASCCAoCAYEG//UBLQSBAxwF)",
		R"(19=gQACAIILCP/6AS0ECAEEAYEI//kBLgYSAP4EAAr/+gEzEIMICgoGBYMIAQH/+AEvAIEH/g)",
		R"(20=MACv/8ATAQggMHGAQMBv//ATcWgQf/+QEyDoIBCCIAHhH/+AE2EAoG/8kBNxqAGQAAIP4y)",
		R"(21=AACGAwAEAAgH/vwBAIQBAQQE/sgdAIAFAAAg/gAgAIAFAAAg/gAgAIAFAAAg/gAgAIAFAA)",
		R"(22=Ag/gAgAIAFAAAg/gAgAIAFAAAg/gAgAIAFAAAg/gAgAIAFAAAg/gAgAIAFAAAg/gAgAIAF)",
		R"(23=AAAg/gAgAIAFAAAg/gAgAIAFAAAg/gAgAIAFAAAg/gAgAIAFAAAg/gAgAIAFAAAg/gAgAI)",
		R"(24=AFAAAg/gAgAIAFAAAg/gAgAIAFAAAg/gAgAIAFAAAg/gAgAIAFAAAg/gAgAIAFAAAg/gAg)",
		R"(25=AIAFAAAg/gAgAIA=)"
	};

	std::stringstream input;
	std::string line;

	for (auto const& line : rawInput) {
		size_t pos = std::string_view(line).find('=');
		if (pos != std::string::npos) {
			input << std::string_view(line).substr(pos + 1);
		}
	}

	const std::string b64Input = input.str();

	const size_t VALUESIZE = 262144;
	std::vector<BYTE> values(VALUESIZE, 0xFF);

	std::vector<BYTE> hex;
	auto const hexlen = FSunPackLib::DecodeBase64(b64Input.data(), hex);
	FSunPackLib::DecodeF80(hex.data(), hexlen, values, VALUESIZE);
	values.resize(VALUESIZE, 0xFF);  // fill rest


	//printBin(values, "output_x64.txt");

	auto const encoded = [&values]
	{
		BYTE* hexpacked = nullptr;
		const size_t hexpackedLen = FSunPackLib::EncodeF80(values.data(), values.size(), 32, &hexpacked);

		//printBin({ hexpacked , hexpackedLen }, "encoded_bin_x64.txt");


		auto const base64 = FSunPackLib::EncodeBase64(hexpacked, hexpackedLen);
		cout << endl
			<< "encoded:"
			<< endl
			<< base64
			<< endl;

		delete[] hexpacked;
		return std::unique_ptr<BYTE>(base64);
	} ();

	ASSERT_EQ(b64Input.size(), strlen(reinterpret_cast<char*>(encoded.get())));
	ASSERT_TRUE(0 == memcmp(encoded.get(), b64Input.data(), b64Input.size()));
}