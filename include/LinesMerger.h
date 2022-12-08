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

#ifndef LINESMERGER_H
#define LINESMERGER_H

#include <cstdio>
#include <cstdlib>
#include <vector>

#include "fFile.h"


class LinesMerger
{
 public:
 	struct filesGatheringOptionsSt;

	LinesMerger(const std::wstring& fileInWildcard, const std::wstring& fileOutName, const filesGatheringOptionsSt& options)
		:_filesGatheringOpt(options) {_fileFOut.open(fileOutName, GENERIC_WRITE, CREATE_ALWAYS); Init(fileInWildcard);}
	~LinesMerger() {}
 //def
	typedef std::vector<std::string> VectorString;
	typedef std::vector<VectorString> FilesLines;
 //struct
	struct DirFileI {
		std::wstring name;
		FILETIME ftCreationTime;
		DirFileI() {}
		DirFileI(std::wstring name_, FILETIME ftCreationTime_)
		 :name(name_), ftCreationTime(ftCreationTime_) {}
	};
	struct filesGatheringOptionsSt{
		filesGatheringOptionsSt(){}
		 ///the /S option
		bool SubdirsIncluded= false;
		bool PauseToConfirm= false;

		 ///the - prefix
		bool reversed= false;
		char orderLetter= 0x00;

	};
 //functions
	 ///main function to merge lines from multiple files
	int MergeLines();
	 ///alias: main function to merge lines from multiple files
	int run() {return MergeLines();}
	int ReplaceAll(std::wstring& strIO, const std::wstring& from, const std::wstring& to, size_t moveby= -1);


 protected:
 //def
 //functions
	void Init(const std::wstring& fileInWildcard);
	int getFiles_rec(const std::wstring& pathAdd);
	int getFiles();
//	int getLinesFromFile( std::wifstream& file_in, VectorString& lines_out );
	int getLinesFromFile( const std::wstring& file_in, VectorString& lines_out );
	 ///@return lines written amount
	size_t MergeFilesContent();
	size_t _noneDuplicLines();
	size_t _writeLinesToFile(const VectorString& Lines);
 //store, data
	 ///store @b 3 files worth of lines to write( @b first file, @b prev file, @b curr file)
	FilesLines LinesOf3files;
	 ///auto created by Init() from provided path-wildcard
	std::wstring _fileInPath;
	 ///expecting path and wildcard like "*.txt"
	std::wstring _fileInWildcard;
//	 ///file to write result into
//	std::wstring _fileOutName;
	fFile _fileFOut;

	std::vector<DirFileI> filesGathered;

	 ///standard is by filename
	filesGatheringOptionsSt _filesGatheringOpt;
};

#endif // LINESMERGER_H
