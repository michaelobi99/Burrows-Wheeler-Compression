#pragma once
#include <string>
#include <chrono>
#include <ranges>
#include <execution>
#include "BitIO.h"
#include "qsort.h"


#define BLOCK_SIZE (1 << 20)

struct charLocation {
	char ch;
	size_t index;
	bool operator < (charLocation const& other) const {
		return this->ch < other.ch;
	}
	bool operator > (charLocation const& other) const {
		return this->ch > other.ch;
	}
	bool operator <= (charLocation const& other) const {
		return this->ch <= other.ch;
	}
	bool operator >= (charLocation const& other) const {
		return this->ch >= other.ch;
	}
	bool operator == (charLocation const& other) const {
		return this->ch == other.ch;
	}
	bool operator != (charLocation const& other) const {
		return !(this->ch == other.ch);
	}
};

std::string getLastChars(std::vector<charLocation> const& sortedChars, char* originalString) {
	size_t len = sortedChars.size();
	std::string bwtString;
	bwtString.resize(len);
	for (size_t i{ 0 }; i < len; ++i) {
		size_t j = sortedChars[i].index - 1;
		if (j < 0) j += len;
		bwtString[i] = originalString[j];
	}
	return bwtString;
}

std::string bwtTransform(char* inputString, size_t length) {
	std::vector<charLocation> vec(length);
	for (int i{ 0 }; i < length; ++i) {
		vec[i].ch = inputString[i];
		vec[i].index = i;
	}
	//stl::quickSort(vec, 0, vec.size() - 1);
	//stl::iSort(vec);
	//std::ranges::sort(vec);
	std::sort(std::execution::par, std::begin(vec), std::end(vec));
	return getLastChars(vec, inputString);
}

void compressFile(std::fstream& input, std::fstream& output)  {
	char* inputString = new char[BLOCK_SIZE];
	size_t length{};
	char ch{};
	while (true) {
		input.read(inputString, BLOCK_SIZE);
		length = input.gcount();
		std::string bwtString = bwtTransform(inputString, length);
		output.write(reinterpret_cast<char*>(bwtString.data()), length);
		if (length < BLOCK_SIZE)
			break;
	}
}