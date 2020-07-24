#ifndef CFIRSTNAME_CMP_UNIT_TEST_H_
#define CFIRSTNAME_CMP_UNIT_TEST_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <algorithm>
#include <initializer_list>
using std::string;
using std::map;
using std::pair;
using std::vector;
using std::cout;
using std::endl;

// Creating a cNickname dict to read, store, and check for matches in nickname database
class cNicknames{
	// Make map - each key may have more than one nickname attached to it, so the item is a vector
	map<string,vector<string>> nickname_map;

	public:
    // Is activated (We should be able to run the file without a nickname file, if we so choose)
	bool is_activated = false;

	// Constructing the class
	// Void construction will not load in data and comparator will always return false
	cNicknames(void);

	//	reads a .csv file mapping names to nicknames and stores them in our nickname_map
	cNicknames(const string &nicknames_file){
		vector<string> colnames;
		pair<string,vector<string>> map_entry;

		// Helper vars
		std::string line, colname;
		string val;

		// Create an input filestream
		std::ifstream nicknameFile(nicknames_file);

		// Make sure the file is open
		if(!nicknameFile.is_open()) throw std::runtime_error("Could not open file"); // Change this error 

		// Get column names and make sure the file is two columns wide
		if(nicknameFile.good())
		{
			// Extract the first line in the file
			std::getline(nicknameFile, line);

			// Create a stringstream from line
			std::stringstream ss(line);

			// Extract each column name
			while(std::getline(ss, colname, ',')){
				
				// Initialize and add <colname, int vector> pairs to result
				colnames.push_back(colname);
			}

			// Make sure the file has two columns
			if( colnames.size() != 2){
				throw std::runtime_error("Nickname File had an unexpected number of columns (expected 2)"); // Change this error 
			}
		}

		// Now read names into map - assume first column is full name - second column is nickname
		while(std::getline(nicknameFile, line))
		{
			// Create a stringstream of the current line
			std::stringstream ss(line);
			
			// Keep track of the current column index
			int colIdx = 0;
			bool keyinmap = false;
			map<string,vector<string>>::iterator it;
			
			// Extract each string
			while(std::getline(ss, val, ',')){
				cout << val << endl;
				if(colIdx == 0){
					// Look to see if key already in the map
					 it = nickname_map.find(val);

					// If it's not in the map, we add it
					if( it == nickname_map.end()){
                        map_entry.first = val;
                        cout << "Map key is: " << val << endl;
                    } else keyinmap = true;
				
				// If we find the key in the map, then we add the item to the back of that entries item vector
				} else if(keyinmap){
					it->second.push_back(val);
				} else{
					map_entry.second.push_back(val);
					nickname_map.insert(map_entry);
				}
				// Increment the column index
				++colIdx;
			}
		}

		// Close file - should now successfully have a mapping of names to nicknames
		nicknameFile.close();
        is_activated = true;
	}
	// Create a bool function that will say whether or not two strings are nicknames of each other
	bool is_nickname(const string &name1, const string &name2){
		/* We will look for the first name in our keys, and see if the second name is in the item vector.
			Then, if that is not successful, check to see if the second name is in the keys and if the first
			 name is in the item vector. If this is not successful, return false. Otherwise, return true.
		*/

	// If class is not activated, return false
	if(!is_activated){
		return false;
	}

    cout << name1 << "\t" << name2 << endl;
	auto it = nickname_map.find(name1);
	// If we find a match in our map, then we probe the items for a match for name2
	if(it != nickname_map.end()){
        cout << "Name found: " << it->first << endl;
        cout << "Associated nicknames are: ";
		// We access our map iterator to loop through the sub-iterator of our
		//	item string vector
		for(auto it2 = it->second.begin(); it2 != it->second.end(); ++it2){
            cout << *it2 << endl;
			if(!name2.compare(*it2)) return true;
		}
	}
	// Now we try the reverse (look for name2 in the keys)
	it = nickname_map.find(name2);
		if(it != nickname_map.end()){
		for(auto it2 = it->second.begin(); it2 != it->second.end(); ++it2){
			if(!name1.compare(*it2)) return true;
		}
	}
	// If we get this far, then the names are not matches of each other -- return false
	return false;
	}

    void print_map(void){
        for(auto it = nickname_map.begin(); it != nickname_map.end(); ++it){
            cout << it->first << ": " << endl;
            for(auto it2 = it->second.begin(); it2 != it->second.end(); ++it2){
                cout << *it2 << endl;
            }
        }
    }
    void interactive_key_find(void){
        cout << "Enter in some strings" << endl;
        string teststring;
        while(std::cin >> teststring){
            auto it = nickname_map.find(teststring);
            if(it != nickname_map.end()){
                cout << "Match_Found!" << endl;
            }
        }
    }

};

// Returns 3 if one character difference
int is_misspell( const char * s1, const char * s2 ) {
	const int size_diff = strlen(s1)- strlen(s2);
	const char * plong = NULL, *pshort = NULL;
	int hit = 0;


	if ( size_diff == 1 || size_diff == -1  ) {
		// one character is missing
		if ( size_diff == 1 ) {
			plong = s1;
			pshort = s2;
		}
		else {
			plong = s2;
			pshort = s1;
		}

		while ( *pshort != '\0' ) {
			if ( *plong++ != *pshort++ ) {
				if ( hit )
					return 0;
				++plong;
				hit = 1;
			}
		}

		if ( *pshort != '\0' && *plong != '\0' )
			return 0;
		else
			return 1;
	}

	else if ( size_diff == 0) {
		//switch or misspell
		while ( *s1 != '\0' ) {
			if ( *s1 != *s2 ) {
				if ( hit )
					return 0;
				else {
					hit = 1;
					plong = s1;
					pshort = s2;
					++s1;
					++s2;
					if ( *s1 == '\0' )
						return 3; //misspelling of last char
				}

			}
			++s1;
			++s2;
		}
		if ( hit == 0 )
			return 4; //exact match
		else {
			if ( *plong != *pshort && *( plong + 1 ) == *(pshort + 1) )
				return 3; //misspelling
			else if ( *plong == * (pshort + 1) && *pshort == *(plong + 1 ))
				return 2; //switch of 2 chars
			else
				return 0;
		}
	}
	else
		return 0;

}

// Helper template function
template <typename T>
bool is_in(const T& v, std::initializer_list<T> lst)
{
    return std::find(std::begin(lst), std::end(lst), v) != std::end(lst);
}

int namecmp2(const string & s1, const string & s2, cNicknames* nnameptr){
    // Initialize some variables
    size_t count = 0;
    bool is_initials = false;
    const char * cs1 = s1.c_str(), * cs2 = s2.c_str(); // c-string versions of inputs
    // First check if either string is empty
    if(s1.empty() || s2.empty()){
        return 1;
    } 
    
    // If they're not empty, but are the same, return max score of 11.
    else if(s1 == s2){
        return 11;
    }

    // If one of them is the same as the other without '-' or ' ', return 10
    std::string output1, output2;
    output1.reserve(s1.size()); output2.reserve(s2.size()); // avoids buffer reallocations in the loop
    for(size_t i = 0; i != s1.size(); ++i){
        if(s1[i] != ' ' && s1[i] != '-') output1 += s1[i];
    }
    for(size_t i = 0; i != s2.size(); ++i){
        if(s2[i] != ' ' && s2[i] != '-') output2 += s2[i];
    }
    if(output1 == output2) return 10;

    // Erase content of strings (so that we can use the same variable again below) 
    output1.clear(); output2.clear(); // This does not free up reserved memory
    
    // Jean-francois vs. Jean-f return 9 / Jean-francois vs. Jean return 7/ Jean-f vs. Jean return 8
    // Steps:   1) Verify both names match up to delimiter ('-' or ' ') (or that one delimits, and the other ends)
    //          2) Check lengths of second segments

    // Check for delimiters

    // If they both have it, make a vector of names?
    if((s1.find(' ') != string::npos || s1.find('-') != string::npos) ||
       (s2.find(' ') != string::npos || s2.find('-') != string::npos)){
            vector<string> v1, v2;
            for(size_t i = 0; i != s1.size(); ++i){
                if(s1[i] != ' ' && s1[i] != '-') output1 += s1[i];
                else{
                    v1.push_back(output1);
                    output1.clear();
                }
            }
            // It's possible that the last character in a name is '-'
            if(!output1.empty()){
                v1.push_back(output1);
                output1.clear();
            }
            for(size_t i = 0; i != s2.size(); ++i){
                if(s2[i] != ' ' && s2[i] != '-') output2 += s2[i];
                else{
                    v2.push_back(output2);
                    output2.clear();
                }
            }
            if(!output2.empty()){
                v2.push_back(output2);
                output2.clear();
            }
        // Make sure vectors have same length (Not sure if a case like jean-francois-ross and jean-f should be matches)
        if(v1.size() == v2.size()){
            // For each word in the name, make sure they're the same up to the last part
            for(size_t i = 0; i != v2.size();++i ){
                // If we get to the end...
                if(i == (v2.size() - 1)){
                    // Make sure one of the strings is one character long
                    if((v1.back().size() == 1 || v2.back().size() == 1) &&
                        v1.back()[0]==v2.back()[0]) 
                        return 9;
                }
                else if(v1[i] != v2[i]) break;
            }
        } 
        // If one of the strings didn't have a delimiter
        else if((v1.size() == 1 || v2.size() == 1)){
            // First check that their first word matches
            if(v1[0] == v2[0] && v1.size() == 1)
                // If the last part is long (like francois), return 7, if -f, return 8
                return (v2.back().size() > 1) ? 7 : 8;
            else if(v1[0] == v2[0] && v2.size() == 1)
                return (v1.back().size() > 1) ? 7 : 8;
            // This is a good place to check for initials
            string initials;
            // If one of the names has no '-'/' ' and is the length of number of words in longer name
            if(v1.size() == 1 && v1[0].size() == v2.size()){
                for(auto j : v2) initials.push_back(j[0]);
                if(v1[0] == initials) is_initials = true;
            }
            else if(v2.size() == 1 && v2[0].size() == v1.size()){
                for(auto j : v1) initials.push_back(j[0]);
                if(v2[0] == initials) is_initials = true;
            }
        }
    }
    // If neither of them true, do nothing and go on in the function
    else{ ;}

    // Next is nicknames, that return 6 points
    if(nnameptr->is_nickname(s1, s2)) return 6;

    // If the names are one character off (after considering higher-scoring options), score of 5
    // Reuse is_misspell function
    if(is_in(is_misspell(cs1,cs2),{1,3})) return 5;

    // Next check that a name matches the beginning of the longer name
    // 4 points if short name at least 3 characters, 3 points otherwise
    
    // If names are the same size, it gets a zero at this point
    string  lstr, sstr;
    if(s1.size() == s2.size()){
        return 0;
    } else if(s1.size() > s2.size()){
        lstr = s1, sstr = s2;
    } else{
        lstr = s2, sstr = s1;
    }

    // Loop through short string. If every index matches the longer index, then check length and return score
    for(size_t i = 0; i < sstr.size(); ++i){
        if(sstr[i] != lstr[i]){
           count = 0;
           break; 
        }
        ++count;
    }
    if(count > 2) return 4;
    else if(count == 2) return 3;
    //Check if shorter name is initials of longer name
    else if(is_initials) return 2;
    else return 0;
}
#endif