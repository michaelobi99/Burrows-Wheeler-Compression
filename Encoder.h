#pragma once
#include <string>
#include <chrono>
#include <ranges>
#include <execution>
#include "BitIO.h"

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

//**************************************************************************************************************************
//BWT Forward Transform
char* getLastChars(std::vector<charLocation> const& sortedChars, char* originalString, size_t& originalStringLocation) {
	size_t len = sortedChars.size();
	char* bwtString = new char [len];
	for (size_t i{ 0 }; i < len; ++i) {
		size_t j = sortedChars[i].index;
		if (j == 0) {
			j += len;
			originalStringLocation = i;
		}
		bwtString[i] = originalString[j - 1];
	}
	return bwtString;
}

char* bwtTransform(char* inputString, size_t length, size_t& originalStringLocation) {
	std::vector<charLocation> vec(length);
	for (int i{ 0 }; i < length; ++i) {
		vec[i].ch = inputString[i];
		vec[i].index = i;
	}
	//std::ranges::sort(vec);
	std::sort(std::execution::par, std::begin(vec), std::end(vec));
	return getLastChars(vec, inputString, originalStringLocation);
}
//***************************************************************************************************************************

//***************************************************************************************************************************
//Move to front encoding
unsigned char* mtfEncoding(char* bwtString, size_t length) {
	char alphabets[256];
	for (unsigned i = 0; i < 256; ++i)
		alphabets[i] = (unsigned char)i;
	unsigned char* mtfString = new unsigned char[length];
	for (unsigned i{ 0 }; i < length; ++i) {
		for (unsigned j = 0; j < 256; ++j) {
			if (bwtString[i] == alphabets[j]) {
				mtfString[i] = (unsigned char)j;
				unsigned char temp = alphabets[j];
				memmove(alphabets + 1, alphabets, j);
				alphabets[0] = temp;
				break;
			}
		}
	}
	return mtfString;
}


void compressFile(std::fstream& input, std::fstream& output)  {
	char* inputString = new char[BLOCK_SIZE];
	size_t length{};
	char ch{};
	size_t originalStringLocation{};
	while (true) {
		input.read(inputString, BLOCK_SIZE);
		length = input.gcount();
		char* bwtString = bwtTransform(inputString, length, originalStringLocation);
		output.write(bwtString, length);
		unsigned char* mtfString = mtfEncoding(bwtString, length);
		//output.write(reinterpret_cast<char*>(mtfString), length);
		std::cout << "mtfString: ";
		for (int i = 0; i < length; ++i)
			std::cout << (int)mtfString[i]<<" ";
		delete[] bwtString;
		delete[] mtfString;
		if (length < BLOCK_SIZE)
			break;
	}
	delete []inputString;
}