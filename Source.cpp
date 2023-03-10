#include <chrono>
#include <format>
#include "Encoder.h"


struct Timer {
public:
	Timer() = default;
	void Start() {
		start = std::chrono::high_resolution_clock::now();
	}
	void Stop() {
		stop = std::chrono::high_resolution_clock::now();
	}
	float time() {
		elapsedTime = std::chrono::duration<float>(stop - start).count();
		return elapsedTime;
	}
private:
	float elapsedTime{};
	std::chrono::time_point<std::chrono::high_resolution_clock> start, stop;
};


uintmax_t fileSize(fs::path const& path) {
	auto lengthInbytes = fs::file_size(path);
	return lengthInbytes;
}


int main() {
	auto timer = Timer();
	try {
		std::fstream input(R"(..\Burrows-Wheeler-Transfrom\caglary corpus\book1)", std::ios_base::in | std::ios_base::binary);
		auto output = stl::OpenOutputBitFile(R"(..\Burrows-Wheeler-Transfrom\testFile2.txt)");
		std::cout << "compression started....\n";
		timer.Start();
		compressFile(input, output);
		timer.Stop();
		printf("\nFile compression complete\n");
		printf("BWT encoding time = %f seconds\n\n", timer.time());
		input.close();
		stl::closeOutputBitFile(output);

		auto input1 = stl::OpenInputBitFile(R"(..\Burrows-Wheeler-Transfrom\testFile2.txt)");
		std::fstream output1(R"(..\Burrows-Wheeler-Transfrom\testFile3.txt)", std::ios_base::out | std::ios_base::binary);
		printf("Expansion started....\n");
		timer.Start();
		expandFile(input1, output1);
		timer.Stop();
		printf("\nFile expansion complete\n");
		printf("BWT decoding time = %f seconds\n\n", timer.time());
		stl::closeInputBitFile(input1);
		output1.close();

		//print file sizes
		
		std::cout << std::format("Original file size = {} bytes\n", fileSize(fs::path(R"(..\Burrows-Wheeler-Transfrom\caglary corpus\book1)")));
		std::cout << std::format("Compressed file size = {} bytes\n", fileSize(fs::path(R"(..\Burrows-Wheeler-Transfrom\testFile2.txt)")));
		std::cout << std::format("Expanded file size = {} bytes\n", fileSize(fs::path(R"(..\Burrows-Wheeler-Transfrom\testFile3.txt)")));
		std::cout << std::format("compression efficiency = {} \n", (8 * fileSize(fs::path(R"(..\Burrows-Wheeler-Transfrom\testFile2.txt)"))) 
			/ float(fileSize(fs::path(R"(..\Burrows-Wheeler-Transfrom\caglary corpus\book1)"))));
	}
	catch (stl::FileError const& error) {
		std::cout << error.what();
		std::cout << "File compression failed\n";
	}
	catch (...) {
		std::cout << "An error occurred during compression or expansion\n";
	}
}