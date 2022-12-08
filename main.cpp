/** Copyright (C) 2022 fafik77 ( https://github.com/fafik77/Auto-Merge-Lines-OCR )
	"Auto Merge Lines.exe" is used to smart merge multiple text files into one

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see https://www.gnu.org/licenses/gpl-3.0.html or https://www.gnu.org/licenses/.
**/

#include "LinesMerger.h"


static const std::string _version("1.1");
static const std::string _onDate("2022-12-08");


void showHelp()
{
	printf("Auto Merge Lines -smart merge text lines from multiple files\n Version %s Released %s\n Download: github.com/fafik77/Auto-Merge-Lines-OCR\n", _version.c_str(), _onDate.c_str());
	printf("\t[1]: (string) input files wildcard like \"*.txt\"\n");
	printf("\t[2]: (string) output file name\n");
	printf("\t[* options]\n");

	printf("  options *:\n\
   /S\twith subdirectories (files order will be automatic)\n\
   /O:\tSortOrder:\n\tN  by Name(alphabetic)\tD  by Date/time (oldest first)\n\
\t-  Prefix to reverse order\n\
   /P\tPause to confirm file order\n\
Example: \"files\\*.txt\" MergedLines.txt /O:N\n\n");
}




//int main(int argc, char *argv[])
int main()
{
	 //get proper arguments
	int argc= 0;
	LPWSTR* argv= CommandLineToArgvW(GetCommandLineW(), &argc);
//for(int i=1; i!=argc; ++i) {
// wprintf(L"[%i] |%s|\n", i, argv[i]);
// }

	if(argc < 3){ //not enough arguments -show help end exit
		showHelp();
		system("pause");
		return 0;
	}

	LinesMerger::filesGatheringOptionsSt options;
     //parse options
    if(argc >= 4){
    	int argNum= 3;
    	std::wstring argStr;
		while( argNum< argc ){
			argStr.assign(argv[argNum]);
			if(argStr==L"/S" || argStr==L"/s" ) { //with SubDirs
				options.SubdirsIncluded= true;
			}
			else if(argStr.find(L"/O:")== 0 || argStr.find(L"/o:")== 0) { //sort order
				unsigned nextPos= 3;
				if(nextPos< argStr.size() && argStr[nextPos]== '-'){
					options.reversed= true; //- to reverse
					++nextPos;
				}
				if(nextPos< argStr.size())
					options.orderLetter= argStr[nextPos];
			}
			else if(argStr.find(L"/P")== 0 || argStr.find(L"/p")== 0) { //PauseToConfirm
				options.PauseToConfirm= true;
			}
			++argNum;
		}
    }

     //run merger with provided arguments and options
	LinesMerger mergeFileLines( argv[1], argv[2], options );
	int retVal= mergeFileLines.run();
system("pause");

	return retVal;

}




