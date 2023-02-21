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

//#define min(a,b) ((a)<(b)?(a):(b))
//#define max(a,b) ((a)>(b)?(a):(b))

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
		if(fileNr){ //if not 1st file
			LinesOf3files[1]= std::move(LinesOf3files[2]);
		}
		getLinesFromFile( (_fileInPath+ iter_of.name), (LinesOf3files[2]) );

		if( fileNr == 0 ){ //if 1st file copy to all
			LinesOf3files[0] =
			LinesOf3files[1] =
			LinesOf3files[2];
		}
		else {
			linesWriten+= _noneDuplicLines();
		}
		++fileNr;
	}
	linesWriten+= _writeLinesToFileFix(LinesOf3files[2]);
return linesWriten;
}

size_t LinesMerger::ApplyFixes(VectorString& io_lines)
{
	size_t retVal= 0;
  //alias
	const bool f_WA2L= _filesGatheringOpt.FixWordAcross2Lines;
	const bool f_MLS= _filesGatheringOpt.FixMultiLineSentence;
	const bool f_FP= _filesGatheringOpt.FixParagraph;
  //run this when we need to merge remove lines
	if(f_WA2L || f_MLS) {
		size_t curr_pos= 0;
		while(curr_pos!= io_lines.size()){
			std::string &str_lineCurr= io_lines[curr_pos];
			if(f_WA2L && str_lineCurr.size() && str_lineCurr.at(str_lineCurr.size()-1)== '-' &&
				((curr_pos+1)!=io_lines.size() && io_lines.at(curr_pos+1).size()) ){
			  //if FixWordAcross2Lines, line ends with '-', next line exists as non empty
				str_lineCurr.pop_back(); //remove ending '-'
				str_lineCurr.append(io_lines.at(curr_pos+1)); //append next line
				io_lines.erase( io_lines.begin()+curr_pos+1 ); //remove next line
				++retVal;
				continue; //ok
			}
			else if(f_MLS && str_lineCurr.size() &&
				((curr_pos+1)!=io_lines.size() && io_lines.at(curr_pos+1).size()) ){
			  //if FixMultiLineSentence, this and next line is non empty
			  //check if next line should actually be separate?
				const char fChar= io_lines.at(curr_pos+1).at(0);
				char fChar2= 0x00;
				if(str_lineCurr.size()>1) fChar2= io_lines.at(curr_pos+1).at(1);
				if( (fChar>='0' && fChar<='9') || fChar=='-' || fChar=='*' || fChar2==')' ){
				  //(next)line starts with enumerator "1. 1) a) *"
				}
				else {
					str_lineCurr.append(" "); //append space
					str_lineCurr.append(io_lines.at(curr_pos+1)); //append next line
					io_lines.erase( io_lines.begin()+curr_pos+1 ); //remove next line
					++retVal;
					continue; //ok
				}
			}
			//default
			++curr_pos;
		}
	}
  //run this for FixParagraph
	if(f_FP){
		for(auto curr_line= io_lines.begin(); curr_line!= io_lines.end(); ++curr_line){
			std::string &str_lineCurr= *curr_line;
			if(str_lineCurr.size()<2) continue; //cant work on empty lines
			const char &ch_1st= str_lineCurr[0];
			BYTE needsFix= 0;

			if(ch_1st== '$' || ch_1st== '8' || ch_1st== '5' ) //?starts with $85
				++needsFix;
			if(needsFix){ //check if not actually a number
				if(str_lineCurr[1]<='0' || str_lineCurr[1]>='9') //not 0-9
					++needsFix;
			}
			if(needsFix == 2){ //check if followed by number
				int leftTries= 5;
				for(auto ch_curr= str_lineCurr.begin()+2; ch_curr!=str_lineCurr.end(); ++ch_curr){
					const char &ch_c= *ch_curr;
					if(ch_c>='0' && ch_c<='9'){ //is a digit
						needsFix= 3;
						break;
					}
					--leftTries;
					if( !leftTries ) break;
				}
			}
			if(needsFix==3){
				str_lineCurr.replace(0, 1, Paragraph_sign, 2);
			}
		}
	}

	return retVal;
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
			if( iter_last->size()>2 && //skip empty lines
			  MatchingStringMin(*iter_last, *iter_curr, _filesGatheringOpt.LineMatchPerc) ){ //if matching str
			  //check for #CQ consecutive matches
			  	int conseqMatches= 0;
			  	const int conseqMatchesReq= _filesGatheringOpt.CQMatches; //translate matches required /CQ
			  	 ///iter_last
			  	auto it_1= iter_last- 1;
			  	 ///iter_curr
			  	auto it_2= iter_curr+ 1;
			  	while(conseqMatches<conseqMatchesReq && it_1!=lastLines.rend() && it_2!=currLines.end() && it_1!=lastLines.rbegin()){
					if (it_1->empty() || it_2->empty()); //dont work on empty lines
					else if(MatchingStringMin(*it_1, *it_2, _filesGatheringOpt.LineMatchPerc)){
						++conseqMatches; //a match
					} else
						break; //no match
					--it_1;
					++it_2;
			  	}
			  	if(conseqMatches>= conseqMatchesReq){ //there are at least 1+#CQ lines overlapping
			   //(multiple)lines are the same (the most bottom line(s) of last, and the most top line(s) of current)
					lastLines.erase( (iter_last+1).base(), lastLines.end());
					currLines.erase(currLines.begin(), iter_curr);
					linesWriten+= _writeLinesToFileFix(lastLines);
					return linesWriten;
			  	}
			}
		}
	}
	linesWriten+= _writeLinesToFileFix(lastLines);
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
size_t LinesMerger::_writeLinesToFileFix(const VectorString& Lines, size_t *o_resFix)
{
	VectorString LinesFixed= Lines;
  //apply Fixes & write to file
	size_t resFix= ApplyFixes( LinesFixed );
	if(o_resFix) (*o_resFix)= resFix;
	return _writeLinesToFile(LinesFixed);
}

bool LinesMerger::MatchingStringMin(const std::string& str1, const std::string& str2, const size_t minPerc )
{
	if(minPerc==100 && str1.size()!=str2.size()) return false; //at 100% line size can not be different
	size_t gotMathing= MatchingString(str1, str2);
	size_t bySize= str1.size();
	if(!bySize) bySize=1;
	if(str2.size()> bySize) bySize= str2.size();

	return ( (gotMathing*100) / bySize)>= minPerc;
}
size_t LinesMerger::MatchingString(const std::string& str1, const std::string& str2 )
{
	size_t retVal= 0;
	int sizeMinReq= str2.size()-6;
	if(sizeMinReq< 0) sizeMinReq= 0;
	int sizeMaxReq= str2.size()+6;
	if( !(str1.size()>= (size_t)sizeMinReq && str1.size()<= (size_t)sizeMaxReq) )
		return 0;
	std::string::const_iterator iter1= str1.begin();
	std::string::const_iterator iter2= str2.begin();

	while(iter1!= str1.end() && iter2!=str2.end()){
	 //compare 2 chars
		if( CharMatchesOCR(*iter1, *iter2) ){ //char matches
			++retVal;
		}
//		BYTE isAnsi= isCharAnsi(*iter1) | isCharAnsi(*iter2)<<1;
//		else if(isAnsi && isAnsi<3){ //only one of them is ansi
//			std::wstring ws;
//			ws.resize(4);
//			ws.resize(std::mbstowcs(&ws[0], &*iter1, 4)); // Shrink to fit.
//		}
	 //move to next char
		++iter1;
		++iter2;
	}

return retVal;
}

bool LinesMerger::CharMatchesOCR(const char& ch1, const char& ch2)
{
	if(ch1==ch2) return true;
	if( (ch1=='.' || ch1==',') && (ch2==',' || ch2=='.') ) return true;
	if( (ch1==';' || ch1==':') && (ch2==':' || ch2==';') ) return true;
	return false;
}


