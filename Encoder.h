#pragma once
#include <string>
#include <chrono>
#include <ranges>
#include <execution>
#include <filesystem>
#include <unordered_set>
#include "Huffman.h"

namespace fs = std::filesystem;


#define BLOCK_SIZE ((1 << 10) * 10)

#define END_OF_BLOCK 255 //I assume that the 255th ASCII doesn't appear in the input text

struct suffix {
	char* ch{};
	uint16_t index{};
	uint16_t blockSize{};
};

struct suffixCompare {
	bool operator() (suffix& s1, suffix& s2) {
		uint16_t blockSize = s1.blockSize;
		uint16_t i = s1.index;
		uint16_t j = s2.index;
		int res = strcmp_(*(s1.ch + i), *(s2.ch + j));
		while (--blockSize && res == 0) {
			i = (i + 1) % s1.blockSize;
			j = (j + 1) % s1.blockSize;
			res = strcmp_(*(s1.ch + i), *(s2.ch + j));
		}
		return res < 1 ? true : false;
	}
	int strcmp_(char c1, char c2) {
		if (c1 == c2) return 0;
		if (c1 < c2) return -1;
		return 1;
	}
};

//**************************************************************************************************************************
//BWT Forward Transform
char* getLastChars(std::vector<suffix> const& sortedChars, char* originalString, uint16_t& originalStringLocation) {
	uint16_t len = sortedChars.size();
	char* bwtString = new char [len];
	for (uint16_t i{ 0 }; i < len; ++i) {
		uint16_t j = sortedChars[i].index;
		if (j == 0) {
			j += len;
			originalStringLocation = i;
		}
		bwtString[i] = originalString[j - 1];
	}
	return bwtString;
}

char* burrowsWheelerForwardTransform(char* inputString, uint16_t length, uint16_t& originalStringLocation) {
	std::vector<suffix> vec(length);
	for (int i{ 0 }; i < length; ++i) {
		vec[i].ch = inputString;
		vec[i].index = i;
		vec[i].blockSize = length;
	}
	std::sort(std::execution::par, std::begin(vec), std::end(vec), suffixCompare());
	return getLastChars(vec, inputString, originalStringLocation);
}

uint16_t* generateTempArray(char* bwtString, char* firstColStr, uint16_t length) {
	uint16_t* tempArray = new uint16_t[length];
	std::unordered_set<int> set;
	for (int i = 0; i < length; ++i) { //loop through bwtStrng
		for (int j = 0; j < length; ++j) { //loop through firstColumnStr
			if (bwtString[i] == firstColStr[j] && set.find(j) == set.end()) {
				tempArray[i] = j;
				set.insert(j);
				break;
			}
		}
	}
	return tempArray;
}

char* burrowsWheelerReverseTransform(char* bwtString, uint16_t length, uint16_t position) {
	char* originalString = new char[length];
	char* firstColunmStr = new char[length];
	for (uint16_t i = 0; i < length; i++) {
		firstColunmStr[i] = bwtString[i];
	}
	std::sort(std::execution::par, firstColunmStr, firstColunmStr + length);
	uint16_t* tempArray = generateTempArray(bwtString, firstColunmStr, length);
	uint16_t n = length - 1, i = 0, T = position;
	originalString[n] = bwtString[T];
	for (i = 1; i < length; ++i) {
		originalString[--n] = bwtString[tempArray[T]];
		T = tempArray[T];
	}
	delete[] firstColunmStr;
	delete[] tempArray;
	return originalString;
}

//***************************************************************************************************************************



//***************************************************************************************************************************
//Move to front encoding
unsigned char* mtfEncode(char* bwtString, uint16_t length) {
	unsigned char alphabets[256];
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

//move to front decoding
char* mtfDecode(unsigned char* mtfString, uint16_t length) {
	char alphabets[256];
	for (unsigned i = 0; i < 256; ++i)
		alphabets[i] = (char)i;
	char* bwtString = new char[length];
	unsigned index;
	for (unsigned i{ 0 }; i < length; ++i) {
		index = (unsigned)mtfString[i];
		bwtString[i] = alphabets[index];
		memmove(alphabets + 1, alphabets, index);
		alphabets[0] = bwtString[i];
	}
	return bwtString;
}
//****************************************************************************************************************************




void compressFile(std::fstream& input, std::unique_ptr<stl::BitFile>& output)  {
	char* originalString = new char[BLOCK_SIZE]; //additional space for length and position
	uint16_t length{};
	uint16_t originalStringLocation{};
	do {
		input.read(originalString, BLOCK_SIZE);
		length = input.gcount();
		char* bwtString = burrowsWheelerForwardTransform(originalString, length, originalStringLocation);
		unsigned char* mtfString = mtfEncode(bwtString, length);
		output->file.write(reinterpret_cast<char*>(&originalStringLocation), sizeof(uint16_t));
		output->file.write(reinterpret_cast<char*>(&length), sizeof(uint16_t));
		huffCompress(bwtString, length, output);
		delete[] bwtString;
		delete[] mtfString;
	} while (length == BLOCK_SIZE);

	delete []originalString;
}

void expandFile(std::unique_ptr<stl::BitFile>& input, std::fstream& output) {
	uint16_t originalStringLocation{}, length{};
	input->file.read(reinterpret_cast<char*>(&originalStringLocation), sizeof(uint16_t));
	while (!input->file.eof()) {
		input->file.read(reinterpret_cast<char*>(&length), sizeof(uint16_t));
		std::cout << "here1\n";
		unsigned char* mtfString = new unsigned char [length];
		huffExpand(input, mtfString);
		std::cout << "here2\n";
		char* bwtString = mtfDecode(mtfString, length);
		for (int i = 0; i < length; ++i)
			std::cout << bwtString[i] << " ";
		std::cout << "here3\n";

		char* originalString = burrowsWheelerReverseTransform(bwtString, length, originalStringLocation);
		std::cout << "here4\n";
		output.write(originalString, length);
		delete []mtfString;
		delete []bwtString;
		delete []originalString;
		input->file.read(reinterpret_cast<char*>(&originalStringLocation), sizeof(uint16_t));
	}
}