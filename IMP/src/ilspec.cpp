#include "ilspec.h"
#include "ilobject.h"
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <assert.h>

using std::string;
using std::endl;
using std::cout; using std::cin;
using std::fstream; using std::ostream;

/******************************************************************************
 * InputLinkSpec Class Function Definitions
 *
 *
 ******************************************************************************
 */

/* Default Constructor
 *
 * Creates an InputLinkSpec object.  Not necessary yet. 
 */
InputLinkSpec::InputLinkSpec()
{
	//necessary yet?
	//Cory, this can probably read in the filename string, 
	//and then based on that , call the appropriate import function.
	//At least, that's one way this could go down
}

/* Destructor
 *
 * Cleans up an InputLinkSpec object.  Not necessary yet.  
 */
InputLinkSpec::~InputLinkSpec()
{
	//necessary yet?
}

extern void pause();
const int defaultLoopBegin = 0;
const int defaultLoopEnd = 1;

/* ImportDM
 *
 * This function creates an input link specification from the datamap
 * contained within "filename"
 * Returns true on success, false on failure.
 */
bool InputLinkSpec::ImportDM(string& filename)
{
	fstream file;
	file.open(filename.c_str());

	if(!file.is_open())
	{
		cout<<"Error: unable to open file "<<filename<<endl;
		return false;
	}

	file.close();
	return true;
}


//This is not quite cool.  The utility will remain the same, 
//but this may need to be a member
void printStage(eParseStage stage, ostream& stream)
{
	stream << "<+> ";
	switch(stage)
	{
		case READING_PRE_BEGIN:
			stream << "'Before Reading'" << endl;
			break;
		case READING_CONTROL_STRUCTURE:
			stream << "'Reading control structure'" << endl;
			break;
		case READING_PARENT_IDENTIFIER:
			stream << "'Reading identifier'" << endl;
			break;
		case READING_ATTRIBUTE:
			stream << "'Reading attribute'" << endl;
			break;
		case READING_VALUE_TYPE:
			stream << "'Reading value type'" << endl;
			break;
		case READING_START_VALUE:
			stream << "'Reading start value'" << endl;
			break;
		case READING_UPDATE:
			stream << "'Reading update'" << endl;
			break;
		case READING_CREATE_ON:
			stream << "'Reading 'create on' condition'" << endl;
			break;
		case READING_DELETE_ON:
			stream << "'Reading \"delete on\" condition'" << endl;
			break;
		case READING_ERROR:
			stream << "***Error in parse***" << endl;
			break;
		default:
			break;
	}
}

//TODO comment
string readOneWord(const string& source)
{
	string retVal;
	string::const_iterator sItr = source.begin();
	
	//the next word may be trailed by a space, a newline, or EOF
	int pos = source.find_first_of(" \n");

	//TODO...handle the EOF case
	if(pos == source.npos)
		return source;

	for(int counter = 0; counter < pos; ++counter, ++sItr)
		retVal += *sItr;
  return retVal;
}

//Erase all characters up to and including the first encountered space.
//TODO make this trim ALL leading spaces
void trimOneWord(string& source)
{
	int pos = source.find_first_of(" ");
	if(pos != source.npos)
		source.erase(0, pos + 1);	
}
//TODO comment
string readAndTrimOneWord(string& source, bool echo = false)
{
	string returnVal = readOneWord(source);
	trimOneWord(source);
	if(echo)
		cout << "read >" << returnVal << "< and trimmed it out." << endl;
	return returnVal;
}

void writeFileLineToString(fstream& source, string& dest, bool echo = false)
{
	char intermediate[MAX_IMP_LINE_LENGTH + 1];
	source.getline(intermediate, MAX_IMP_LINE_LENGTH + 1);//what is this + 1 for? looks like it might overflow
	dest = intermediate;
	if(echo)
		cout << "Entire line from file is:>" << dest << "<" << endl;
}

/* ImportIL
 *
 * This function creates an input link specification from the IL file passed.
 * Returns true on success, false on failure.
 */
bool InputLinkSpec::ImportIL(string& filename)
{
	fstream file;
	file.open(filename.c_str());

	if(!file.is_open())
	{
		cout << "Error: unable to open file " << filename << ". pausing now before exit." << endl;
		pause();
		return false;
	}

	//counts number of lines successfully imported from the file
	int lineNumber = 0;
	
	//this should create InputLinkObjects to hold each line of data read
	while(!file.eof())
	{
		eParseStage parseStage = READING_BEGIN_STAGE;
		eParseStage lastCompletedState = READING_PRE_BEGIN;
		int loopBegin = 0;//FIXME TODO shouldn't these be defaults?
		int loopEnd = 1;

		string line;
		writeFileLineToString(file, line, true);

		string firstWord = readOneWord(line);	 
		//cout << "first word is >" << firstWord << "<" << endl;
		
		//ensure that the line wasn't too long
		if(file.gcount() >= MAX_IMP_LINE_LENGTH+1)
		{
			cout<<"Error: line "<<lineNumber<<" too long"<<endl;
			return false;
		}

		//numCharsRead = sscanf(curLine, "%s", curWord);
//TODO fix these comments
    //the first read should always be a string, so if this fails, the input was
    //improperly formated.  The captured word will either be a control structure
    //or an identifier name

		if(firstWord == k_forToken)
		{
			int controlStartVal, controlEndVal = 0;

			cout << "'for' control structure recognized." << endl;

			//clear everything up to and including the first space
			trimOneWord(line);

			//read control variable name
			string controlName = readAndTrimOneWord(line);
			//cout << "control variable name is >" << controlName << "<" << endl;

			controlStartVal = atoi(readAndTrimOneWord(line).c_str());
			//cout << "control start val is >" << controlStartVal << "<" << endl;

			controlEndVal = atoi(line.c_str());
			//cout << "control end val is >" << controlEndVal << "<" << endl;

			cout << "After all the control nonsense, the values are: " << controlName << " "
				<< controlStartVal << " " << controlEndVal << endl;

			//set ACTUAL loop start and stop delimiters			
			loopBegin = controlStartVal;
			loopEnd = controlEndVal;

			//TODO (if doing nested control loops, add an entry to the control queue)

			//since we just consumed the first line, read in the next one 
			//for the identifier information
			writeFileLineToString(file, line, true);
			lastCompletedState = READING_CONTROL_STRUCTURE;
			parseStage = READING_PARENT_IDENTIFIER;			
		}
		else
			parseStage = READING_PARENT_IDENTIFIER;

		string curWord;
		InputLinkObject ilObj;

		//Begin parsing based on stage
		//while(parseStage != READING_FINAL_STAGE)
		//{
		for(; loopBegin < loopEnd; ++loopBegin)
		{
			printStage(parseStage, cout);
			bool readingFirstType = true, moreTypesLeft = true;

			switch(parseStage)
			{

				case READING_CONTROL_STRUCTURE:
					break;
				case READING_PARENT_IDENTIFIER:

					curWord = readAndTrimOneWord(line);
					cout << "ParentIdName is >" << curWord << "<" << endl;
					ilObj.setParentId(curWord);
					lastCompletedState = READING_PARENT_IDENTIFIER;
					parseStage = READING_ATTRIBUTE;
					break;
				case READING_ATTRIBUTE:
					curWord = readAndTrimOneWord(line);
					//trim off the attrib token
					curWord.replace(0, 1, "");
					ilObj.setAttribName(curWord);
					lastCompletedState = READING_ATTRIBUTE;
					parseStage = READING_VALUE_TYPE;
					break;
				case READING_VALUE_TYPE:
					curWord = readAndTrimOneWord(line);
					//if the first token hasn't been read, and there isn't one here, that's a problem
					if(curWord.find_first_of(k_typesOpenToken.c_str(),0, curWord.size()) == curWord.npos)
					{
						cout << "didn't find open token:>" << curWord << "<" << endl;
						parseStage = READING_ERROR;
						break;
					}

					//trim off the '<' token
					curWord.replace(0,1, "");

					do//TODO this may work better as a 'for' loop
					{
						//if it IS the first type listed, we already have a string to parse
						if(!readingFirstType)
							curWord = readAndTrimOneWord(line);
						else
							cout << "\tReading first type for this wme, which is >" << curWord << "<." << endl;

						readingFirstType = false;
						int pos = curWord.find(k_typesCloseToken.c_str());
						if(pos == curWord.npos)
							moreTypesLeft = true;
						else
						{ //TODO make sure any whitespace leading the '>' is taken care of...
							moreTypesLeft = false;
							//trim off the '>' token
							curWord.erase(pos);
						}
						cout << "Read value type as " << curWord << endl;
						ilObj.addElementType(curWord);

						//TODO tough error checking to do here---v
						//if the type is an ID, it should be listed alone
							parseStage = READING_START_VALUE;
						if(!stricmp(curWord.c_str(), k_idString.c_str()))
						{
							lastCompletedState = READING_VALUE_TYPE;
							//parseStage = READING_IDENTIFIER_UNIQUE_NAME;
							break;
						}

					}
					while(moreTypesLeft);

					lastCompletedState = READING_VALUE_TYPE;
					break;
				/*case READING_IDENTIFIER_UNIQUE_NAME:
				cout << "\t\tWe have an identifier, so reading the unique name." << endl;
				cout << "before next read and trim, line is:>" << line << "<" << endl;
					curWord = readAndTrimOneWord(line);//TODO error checking
				cout << "trying to set the start value"
					ilObj.setStartValue(curWord);
					lastCompletedState = READING_IDENTIFIER_UNIQUE_NAME;
					parseStage = READING_FINAL_STAGE;//IDs never get updated
					break;*/		
				case READING_START_VALUE:
//cout << "Top of Reading start value label...." << endl;
//cout << "before next read and trim, line is:>" << line << "<" << endl;
					curWord = readAndTrimOneWord(line, true);

//cout << "\tshould have just trimmed of start token here: " << curWord << endl;
//cout << "\tand left behind:" << line << endl;
					if(curWord != k_startToken)
					{
						parseStage = READING_ERROR;
						break;
					}
//cout << "\tnow should have just the start value left on this line..."	<< line << endl;
					//now that the token is out of the way, get the actual start value
					curWord = readAndTrimOneWord(line, true);

					ilObj.setStartValue(curWord);
					lastCompletedState = READING_START_VALUE;
					parseStage = READING_UPDATE;
					break;
				case READING_UPDATE:
					//this is optional.  if there is no -update token, bug out.
					//TODO make a note that this is not a literal string, but rather the
					//name of the variable whose value will be used for the update
					curWord	= readAndTrimOneWord(line);
					if(curWord == k_updateToken)
						curWord = readAndTrimOneWord(line);
					else
					{
						parseStage = READING_ERROR;
						break;
					}
					if(curWord == "\n" || curWord == "" )
					{
						ilObj.setUpdateValue(line);
						parseStage = READING_CREATE_ON;
					}
					lastCompletedState = READING_UPDATE;
					parseStage = READING_CREATE_ON;
					break;
				case READING_CREATE_ON:
					lastCompletedState = READING_CREATE_ON;
					parseStage = READING_DELETE_ON;
					break;
				case READING_DELETE_ON:
					lastCompletedState = READING_DELETE_ON;
					break;
				case READING_ERROR:
					cout << "Error encountered during parse. Last completed stage was ";
					printStage(lastCompletedState, cout);
					cout << endl;
					pause();
					exit(-1);
					break;
				default:
					cout << "What? ended up in the default case...." << endl;
					assert(false);
					break;
			}//end switch
		}// end control loop
cout << "\toutside of for loop, will read another line soon....." << endl;
		ilObjects.push_back(ilObj);
//		loopBegin = defaultLoopBegin;
//		loopEnd = defaultLoopEnd;

		++lineNumber;
//     cout << "just read line " << lineNumber << endl;
	}//while there are still lines to read

	cout << "Read " << lineNumber << " lines." << endl;
	cout << "Number of input link entries " << ilObjects.size() << endl;
	file.close();
	return true;
}

