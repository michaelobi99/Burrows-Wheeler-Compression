#pragma once
#include <string>
#include <chrono>
#include <ranges>
#include <execution>
#include <filesystem>
#include <unordered_set>
#include "Huffman.h"

namespace fs = std::filesystem;


#define BLOCK_SIZE (1 << 20)

#define END_OF_BLOCK 255 //I assume that the 255th ASCII doesn't appear in the input text

struct suffix {
	char* ch{};
	size_t index{};
	size_t blockSize{};
};

struct suffixCompare {
	bool operator() (suffix& s1, suffix& s2) {
		size_t blockSize = s1.blockSize;
		size_t i = s1.index;
		size_t j = s2.index;
		int res = strcmp_(*(s1.ch + i), *(s2.ch + j));
		while (res == 0 && --blockSize) {
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
char* getLastChars(std::vector<suffix> const& sortedChars, char* originalString, size_t& originalStringLocation) {
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

char* burrowsWheelerForwardTransform(char* inputString, size_t length, size_t& originalStringLocation) {
	std::vector<suffix> vec(length);
	for (int i{ 0 }; i < length; ++i) {
		vec[i].ch = inputString;
		vec[i].index = i;
		vec[i].blockSize = length;
	}
	std::sort(std::execution::par, std::begin(vec), std::end(vec), suffixCompare());
	return getLastChars(vec, inputString, originalStringLocation);
}

size_t* generateTempArray(char* bwtString, char* firstColStr, size_t length) {
	size_t* tempArray = new size_t[length];
	std::unordered_set<int> set;
	for (int i = 0; i < length; ++i) { //loop through btwStrng
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

char* burrowsWheelerReverseTransform(char* bwtString, size_t length, size_t position) {
	char* originalString = new char[length];
	char* firstColunmStr = new char[length];
	for (size_t i = 0; i < length; i++) {
		firstColunmStr[i] = bwtString[i];
	}
	std::sort(std::execution::par, firstColunmStr, firstColunmStr + length);
	size_t* tempArray = generateTempArray(bwtString, firstColunmStr, length);
	size_t n = length - 1;
	size_t i = 0;
	size_t T = position;
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
unsigned char* mtfEncode(char* bwtString, size_t length) {
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
char* mtfDecode(unsigned char* mtfString, size_t length) {
	char alphabets[256];
	for (unsigned i = 0; i < 256; ++i)
		alphabets[i] = (char)i;
	char* bwtString = new char[length];
	for (unsigned i{ 0 }; i < length; ++i) {
		unsigned index = (unsigned)mtfString[i];
		bwtString[i] = alphabets[index];
		memmove(alphabets + 1, alphabets, index);
		alphabets[0] = bwtString[i];
	}
	return bwtString;
}
//****************************************************************************************************************************




void compressFile(std::fstream& input, std::unique_ptr<stl::BitFile>& output)  {
	std::fstream mtfFile{ "temp.txt", std::ios_base::out | std::ios_base::binary };
	char* originalString = new char[BLOCK_SIZE];
	size_t length{};
	size_t originalStringLocation{};
	do {
		input.read(originalString, BLOCK_SIZE);
		length = input.gcount();
		char* bwtString = burrowsWheelerForwardTransform(originalString, length, originalStringLocation);
		unsigned char* mtfString = mtfEncode(bwtString, length);
		mtfFile.write(reinterpret_cast<char*>(&originalStringLocation), sizeof(size_t)); //write position of original string
		mtfFile.write(reinterpret_cast<char*>(&length), sizeof(size_t)); //write size of block
		mtfFile.write(reinterpret_cast<char*>(mtfString), length); //write block
		printf("got here\n");
		delete[] bwtString;
		delete[] mtfString;
	} while (length == BLOCK_SIZE);

	delete []originalString;
	mtfFile.clear();
	mtfFile.close();
	mtfFile.open("temp.txt", std::ios_base::in | std::ios_base::binary);
	huffCompress(mtfFile, output);
	mtfFile.close();
	std::error_code err;
	fs::remove("temp.txt", err);
}

void expandFile(std::unique_ptr<stl::BitFile>& input, std::fstream& output) {
	std::fstream mtfFile{ "temp.txt", std::ios_base::out | std::ios_base::binary };
	huffExpand(input, mtfFile);
	mtfFile.clear();
	mtfFile.close();
	mtfFile.open("temp.txt", std::ios_base::in | std::ios_base::binary);
	size_t length{}; //block length
	size_t originalStringLocation{};
	mtfFile.read(reinterpret_cast<char*>(&originalStringLocation), sizeof(size_t));
	while (!mtfFile.eof()) {
		mtfFile.read(reinterpret_cast<char*>(&length), sizeof(size_t));
		unsigned char* mtfString = new unsigned char[length];
		mtfFile.read(reinterpret_cast<char*>(mtfString), length);
		char* bwtString = mtfDecode(mtfString, length);
		char* originalString = burrowsWheelerReverseTransform(bwtString, length, originalStringLocation);
		output.write(originalString, length);
		delete []mtfString;
		delete []bwtString;
		delete []originalString;
		mtfFile.read(reinterpret_cast<char*>(&originalStringLocation), sizeof(size_t));
	}
	mtfFile.close();
	std::error_code err;
	fs::remove("temp.txt", err);
}