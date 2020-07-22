/*
 * DisambigComp.h
 *
 *  Created on: Dec 9, 2010
 *      Author: ysun
 */

#ifndef DISAMBIGCOMP_H_
#define DISAMBIGCOMP_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
using std::string;
using std::map;
using std::pair;
using std::vector;

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
				std::cout << val << std::endl;
				if(colIdx == 0){
					// Look to see if key already in the map
					 it = nickname_map.find(val);

					// If it's not in the map, we add it
					if( it == nickname_map.end()){
                        map_entry.first = val;
                        std::cout << "Map key is: " << val << std::endl;
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

    std::cout << name1 << "\t" << name2 << std::endl;
	auto it = nickname_map.find(name1);
	// If we find a match in our map, then we probe the items for a match for name2
	if(it != nickname_map.end()){
        std::cout << "Name found: " << it->first << std::endl;
        std::cout << "Associated nicknames are: ";
		// We access our map iterator to loop through the sub-iterator of our
		//	item string vector
		for(auto it2 = it->second.begin(); it2 != it->second.end(); ++it2){
            std::cout << *it2 << std::endl;
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
            std::cout << it->first << ": " << std::endl;
            for(auto it2 = it->second.begin(); it2 != it->second.end(); ++it2){
                std::cout << *it2 << std::endl;
            }
        }
    }
    void interactive_key_find(void){
        std::cout << "Enter in some strings" << std::endl;
        string teststring;
        while(std::cin >> teststring){
            auto it = nickname_map.find(teststring);
            if(it != nickname_map.end()){
                std::cout << "Match_Found!" << std::endl;
            }
        }
    }

};
const unsigned int Jaro_Wrinkler_Max = 5;

template <typename Tp>
inline const Tp& max_val(const Tp& arg1, const Tp &arg2) {
	return ( arg1 < arg2 )? arg2 : arg1;
}

template <typename Tp>
inline const Tp& min_val(const Tp& arg1, const Tp &arg2) {
	return ( arg1 < arg2 )? arg1 : arg2;
}

template <typename Tp, typename Functor>
vector <Tp> Longest_Common_Subsequence(const vector <Tp> & s1, const vector <Tp> &s2, const Functor & func);


char * extract_initials(char * dest, const char * source) ;
int nospacecmp(const char* str1, const char* str2);
int jwcmp(const string & str1, const string& str2);
int midnamecmp(const string & str1, const string & str2 );
unsigned int langcmp(const string & lang1, const string & lang2);
int countrycmp(const string & country1, const string & country2 );
int streetcmp(const string& inputstreet1, const string& inputstreet2);
int latloncmp(const string & inputlat1, const string & inputlon1, const string & inputlat2, const string & inputlon2 );
int classcmp(const string &class1, const string& class2 );
int coauthorcmp(const string &coauthor1, const string& coauthor2 );
int asgcmp(const string & asg1, const string & asg2, const map<string, std::pair<string, unsigned int> > * const asg_table_pointer);
int asgcmp ( const string & s1, const string &s2) ;
int asgcmp_to_test(const vector <string> & asg1, const vector <string> & asg2,
		   const map<string, std::pair<string, unsigned int> > * const asg_table_pointer);
int name_compare( const string & s1, const string & s2, const unsigned int prev, const unsigned int cur);


class cSentence_JWComparator {
private:
	const double threshold;
public:
	bool operator() (const string * s1, const string * s2) const;
	explicit cSentence_JWComparator(const double inputthreshold): threshold(inputthreshold){};
};


template < typename Iter1, typename Iter2 >
unsigned int num_common_elements ( const Iter1 & p1begin, const Iter1 & p1e ,
									const Iter2 & p2begin, const Iter2 & p2e, const unsigned int max) {// containers must be sorted before use.
	// it has to be a sorted version container, like set, or sorted vector or list
	unsigned int cnt = 0;
	Iter1 p1b = p1begin;
	Iter2 p2b = p2begin;
	while ( p1b != p1e && p2b != p2e ) {
		if ( *p1b < *p2b ) {
			++p1b;
		}
		else if ( *p2b  < *p1b  ) {
			++p2b;
		}
		else {
			++cnt;
			++p1b;
			++p2b;
		}

		if ( cnt == max && max != 0 )
			break;
	}
	return cnt;
}

#endif /* DISAMBIGCOMP_H_ */
