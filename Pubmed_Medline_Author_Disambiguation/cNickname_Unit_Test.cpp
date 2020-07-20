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

string path_to_file = "/Users/RobertTeresi/Documents/Github/Pubmed_Medline_Author_Disambiguation-orig/nickname_test.csv";
cNicknames Nicknames(path_to_file);

string name1 = "One";
string name2 = "Two";

int main(void){
    bool is_nickname = Nicknames.is_nickname(name1, name2);
    cout << is_nickname << "\n" << Nicknames.is_activated << endl;
    Nicknames.print_map();
    Nicknames.interactive_key_find();
    return 0;
}