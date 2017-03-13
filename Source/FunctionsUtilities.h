/*
 * FunctionsUtilities.h
 *
 *  Created on: 2015-12-26
 *      Author: Pierre-Marc Bonneau
 */

#ifndef FUNCTIONSUTILITIES_H_
#define FUNCTIONSUTILITIES_H_

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdlib.h>

using namespace std;

namespace utilities
{
	string LittleToBigEndian(string LittleEndian);
	string HexStringToChars(string HexString);
	long getFileSize(FILE *file);
	int ConvertStringToDecInt(string EntryString);
	string ConvertDecIntToString(int EntryDecimal);
}

#endif /* FUNCTIONSUTILITIES_H_ */
