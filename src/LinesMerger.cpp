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

#include <algorithm>

 ///run by ctor
void LinesMerger::Init(const std::wstring& fileInWildcard)
{
 //files wildcard in
	std::wstring wsTmp(fileInWildcard);
	 //format to proper windows path(but dont allow super long paths)
	ReplaceAll(wsTmp, L"/", L"\\");
	 //dont allow super long paths
	while(wsTmp.find(L"\\\\?\\")== 0){
		wsTmp.replace(0, 4, L""); //remove that
	}
	ReplaceAll(wsTmp, L"\\\\", L"\\", 0);

	 //?has path?
	size_t lastBSlash= wsTmp.rfind(L"\\");
	if(lastBSlash!= wsTmp.npos) { //has a path
		_fileInPath= wsTmp.substr(0, lastBSlash);
		_fileInWildcard= wsTmp.substr(lastBSlash+1);
		if(_fileInWildcard.empty() && _fileInPath.size()) _fileInWildcard= L"*.*";
	}
	else
		_fileInWildcard= wsTmp;
	 //provided relative path(not absolute), prepend CWD
	if(_fileInPath.find(L":")== _fileInPath.npos){
		wsTmp.clear();
		wsTmp.resize(MAX_PATH);
		DWORD sized= GetCurrentDirectoryW(MAX_PATH, &wsTmp[0]);
		wsTmp.resize(sized);
		if(wsTmp.back()!='\\') wsTmp+= '\\'; //make sure that Path end with '\'
		_fileInPath.insert(0, wsTmp);
	}
	if(_fileInPath.back()!='\\') _fileInPath+= '\\'; //make sure that Path end with '\'

 //options
	 //make sure its a big letter
	if(_filesGatheringOpt.orderLetter>= 'a') _filesGatheringOpt.orderLetter-= ('a'-'A');
}

int LinesMerger::MergeLines()
{
	int filesFound= getFiles();
	printf("Found %i files, ready to merge in following order: [\n", filesFound);
	for(const auto& iterFile : filesGathered){
		wprintf(L"  %s\n", (iterFile).name.c_str());
	}
	printf("]\n");
	 //pause to confirm this file order
	if(_filesGatheringOpt.PauseToConfirm){
		printf("\n   Confirm this file order> ");
		system("pause");
	}

	size_t linesWriten= MergeFilesContent();
	printf("Merge Result:\n Lines= %u\n", linesWriten);
	_fileFOut.close();
	return -1;
}
 ///@return replaced count
int LinesMerger::ReplaceAll(std::wstring& strIO, const std::wstring& from, const std::wstring& to, size_t moveby)
{
	if(moveby> to.size()){
		moveby= to.size();
	}
	int retVal= 0;
	size_t lastFind= strIO.find(from);
	while( lastFind!= strIO.npos ){
		++retVal;
		strIO.replace(lastFind, from.size(), to); //replace from old to new
		lastFind+= moveby; //move to next find
		lastFind= strIO.find(from, lastFind); //find next
	}
	return retVal;
}

 ///@return number of files found
int LinesMerger::getFiles_rec(const std::wstring& pathAdd)
{
	int retVal= 0;
	std::wstring tempFileName;
	 tempFileName.reserve(MAX_PATH);
	WIN32_FIND_DATAW findData;
	HANDLE filesFinderHandle= FindFirstFileW( (_fileInPath+ pathAdd+ _fileInWildcard).c_str(), &findData );

	if( filesFinderHandle!= INVALID_HANDLE_VALUE ){
		do{ //get all files matching the wildcard
			tempFileName.assign(findData.cFileName);
			if(tempFileName== L"." || tempFileName== L".." ) continue;	//skip . and ..
			 ///alias to attribs from getItem
			const DWORD &dwAttrs= findData.dwFileAttributes;
			 //dir
			if(dwAttrs & FILE_ATTRIBUTE_DIRECTORY){
				 //traverse sub dir if /S used
				if(_filesGatheringOpt.SubdirsIncluded){
					retVal+= getFiles_rec(pathAdd+ tempFileName+ L"\\");
				}
			}
			 //file
			else {
				++retVal;
				 //push back the file name found
				filesGathered.emplace_back( DirFileI(pathAdd+ tempFileName, findData.ftCreationTime) ); //save file name
			}
		} while( FindNextFileW(filesFinderHandle, &findData) );
		FindClose(filesFinderHandle);
		return retVal;
	}
	return -1;
}
#include <array>
 ///@return number of files found
int LinesMerger::getFiles()
{
	int retVal= 0;
	retVal= getFiles_rec(L"");
	//sort files here based on options
	printf("# sort order:");
	if(_filesGatheringOpt.orderLetter== 'N'){
		printf(" by Name");
		std::sort(filesGathered.begin(), filesGathered.end(),
			[](const DirFileI& it1, const DirFileI& it2 ){
				int res= it1.name.compare(it2.name);
				 //name the same, compare by date
				if(!res) return (memcmp(&it1.ftCreationTime, &it2.ftCreationTime, sizeof(FILETIME) )< 0);
				return( res< 0 );
			}
		);
	}
	else if(_filesGatheringOpt.orderLetter== 'D'){
		printf(" by Date");
		std::sort(filesGathered.begin(), filesGathered.end(),
			[](const DirFileI& it1, const DirFileI& it2 ){
				int res= memcmp(&it1.ftCreationTime, &it2.ftCreationTime, sizeof(FILETIME) );
				 //date the same, compare by name
				if(!res) return (it1.name.compare(it2.name)< 0);
				return( res< 0 );
			}
		);
	}
	 //reverse order if '-' used
	if(_filesGatheringOpt.reversed){
		std::reverse(filesGathered.begin(), filesGathered.end());
		printf(" reversed");
	}
	printf("\n");

	return retVal;
}


 ///@return int: amount of lines found
 ///@param (ifstream): file_in file to read <b>(not CONST)</b>
 ///@param <u>VectorString</u>: lines_out stores lines read from file
int LinesMerger::getLinesFromFile( const std::wstring& file_in, VectorString& lines_out )
{
	fFile inFile;
	inFile.open(file_in, GENERIC_READ, OPEN_EXISTING);
	lines_out.clear();
	int retVal= 0;

	if(inFile.ok()){
		std::string tempLine;
		while( inFile.readLine(tempLine)!= size_t(-1) ){
			++retVal;
			lines_out.push_back(tempLine);
		}
		if(_fileFOut.is_DefaultEol())
			_fileFOut.setEol(inFile.getEol());
	}
	return retVal;
}


size_t LinesMerger::MergeFilesContent()
{
	LinesOf3files.resize(3);
	int fileNr= 0;
	size_t linesWriten= 0;
	for(const auto& iter: filesGathered){
		const DirFileI& iter_of= iter;
		if(fileNr){
			LinesOf3files[1]= std::move(LinesOf3files[2]);
		}
		getLinesFromFile( (_fileInPath+ iter_of.name), (LinesOf3files[2]) );
		if( fileNr == 0 ){ //copy to all
			LinesOf3files[0] =
			LinesOf3files[1] =
			LinesOf3files[2];
		}
		else {
			linesWriten+= _noneDuplicLines();
		}
		++fileNr;
	}
	linesWriten+= _writeLinesToFile(LinesOf3files[2]);
return linesWriten;
}

size_t LinesMerger::_noneDuplicLines()
{
	size_t linesWriten= 0;
	 ///alias lines
	VectorString& currLines= LinesOf3files[2];
	VectorString& lastLines= LinesOf3files[1];
 //remove duplicates with First
	for(auto iter= LinesOf3files[0].begin(); iter!= LinesOf3files[0].end(); ++iter){
	 //remove from beginning
		if(currLines.empty()) return linesWriten;
		if(currLines.at(0) == *iter){
			currLines.erase(currLines.begin());
		}
		else
			break;
	}
	for(auto iter= LinesOf3files[0].rbegin(); iter!= LinesOf3files[0].rend(); ++iter){
	 //remove from ending
		if(currLines.empty()) return linesWriten;
		if(currLines.back() == (*iter) ){
			currLines.pop_back();
		}
		else
			break;
	}
 //remove duplicates with Last
	//from lower lines find the line that intersects with last
	for(auto iter_curr= currLines.begin(); iter_curr!= currLines.end(); ++iter_curr){
		if(iter_curr->size()<=2)
			continue; //skip empty lines
		for(auto iter_last= lastLines.rbegin(); iter_last!= lastLines.rend(); ++iter_last){
			if( iter_last->size()>2 && (*iter_last) == (*iter_curr) ){ //skip empty lines
			 //lines are the same (the most bottom line of last, and the most top line of current)
				lastLines.erase( (iter_last+1).base(), lastLines.end());
				currLines.erase(currLines.begin(), iter_curr);
				linesWriten+= _writeLinesToFile(lastLines);
				return linesWriten;
			}
		}
	}
	linesWriten+= _writeLinesToFile(lastLines);
	return linesWriten;
}

size_t LinesMerger::_writeLinesToFile(const VectorString& Lines)
{
	if(_fileFOut.error()) return -1;

	size_t retVal= 0;
	for(auto& iter : Lines){
		const std::string& iterStr= iter;
		_fileFOut.write( iterStr.size(), &iterStr[0] );
		_fileFOut.writeNewLine();
		++retVal;
	}
	return retVal;
}



