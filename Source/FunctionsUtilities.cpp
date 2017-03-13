/*
 * FunctionsUtilities.cpp
 *
 *  Created on: 2015-12-26
 *      Author: Pierre-Marc Bonneau
 */

#include "FunctionsUtilities.h"
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <stdio.h>

namespace utilities
{
	string LittleToBigEndian(string LittleEndian)
	{
		stringstream ssBigEndian;
		string BigEndian = "";
		ssBigEndian << LittleEndian[6] << LittleEndian[7] << LittleEndian[4] << LittleEndian[5]
			<< LittleEndian[2] << LittleEndian[3] << LittleEndian[0] << LittleEndian[1];
		ssBigEndian >> BigEndian;
		return BigEndian;
	}

	string HexStringToChars(string HexString)
	{
		stringstream ssOutputString;
		string OutputString;
		char Convert;

		for(unsigned int i = 0; i < HexString.length(); i = i + 2)
		{
		    string byte = HexString.substr(i,2);
		    Convert = (char) (int)strtol(byte.c_str(), 0, 16);
		    OutputString.push_back(Convert);
		}
		return OutputString;
	}

	long getFileSize(FILE *file)
	{
		long lCurPos, lEndPos;
		lCurPos = ftell(file);
		fseek(file, 0, 2);
		lEndPos = ftell(file);
		fseek(file, lCurPos, 0);
		return lEndPos;
	}

	int ConvertStringToDecInt(string EntryString)
	{
		int Output = 0x0;
		bool FirstDigit = false;
		string FormattedEntryString = "";
		for (unsigned int i = 0; i <= EntryString.length(); i++)
		{
			if (EntryString[i] != 48)
			{
				FirstDigit = true;
				FormattedEntryString = FormattedEntryString + EntryString[i];
			}
			else if (EntryString[i] == 48 && FirstDigit == true)
			{
				FormattedEntryString = FormattedEntryString + EntryString[i];
			}
		}
		stringstream ConvertToInt;
		ConvertToInt << FormattedEntryString;
		ConvertToInt >> hex >> Output;
		return Output;
	}

	string ConvertDecIntToString(int EntryDecimal)
	{
		stringstream ConvertToString;
		string Output;
		ConvertToString << hex << EntryDecimal;
		ConvertToString >> Output;
		cout << Output << endl;
		return Output;
	}
}


