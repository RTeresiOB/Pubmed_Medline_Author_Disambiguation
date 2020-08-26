#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <tuple> // assign function result to two variables
#include <utility> // std::pair/std::make_pair

using namespace std;

// Composite function that takes in list of dictionaries from python and writes result to file
void affil_stopword_find(vector<map<string, pair<unsigned int, map<string, unsigned int>>>> listofdicts, string outfilename){

    // Aggregate vector of maps into one
    map<string, pair<unsigned int, map<string, unsigned int>>> aggmap;
    unsigned int nRecords;
    tie(nRecords, aggmap) = mapcombine(listofdicts); // Function has tuple output

    // Now run affilword_count to get map of total number of affilword usages
    map<string, unsigned int> affilmap = affilword_count(aggmap);

    // Get stopword list
    vector<pair<string, string>> stopwords = stopword_gen(affilmap, aggmap, nRecords);

    // Print to file
    print_to_file(stopwords, outfilename);
}

// Function to print vector of stopwords to file
void print_to_file(vector<pair<string, string>> stopword_vec, string outfilepath = "/Users/RobertTeresi/Downloads/my_file.txt"){
    ofstream outFile(outfilepath);
    // the important part
    for (const auto &pair : stopword_vec) outFile << pair.first << "," << pair.second << "\n";
}

void read_file(string infilepath = "/Users/RobertTeresi/Downloads/my_file.txt"){
    
    // Initialize vector of stopword pairs
    vector<pair<string, string>> stopword_vec;

    // Helper vars
    std::string line, colname;
    string val;

    // Create an input filestream
    std::ifstream inputFile(infilepath);

    if(!inputFile.is_open()) throw std::runtime_error("Could not open file");

    if(inputFile.good()){
        while(getline(inputFile, line))
		{
		// Create a stringstream of the current line
		std::stringstream ss(line);
        int colIdx = 0;
        string first, second;
        while(std::getline(ss, val, ',')){
            if(colIdx == 0){
                first = val;
            } else if(colIdx == 1){
                second = val;
                stopword_vec.push_back(make_pair(first,second));
            } else{
                throw std::runtime_error("Expected two entries per line. Encountered more");
            }
            ++colIdx;
        }
        }
    } else{
        throw std::runtime_error("File not good.");
    }

    // Now print the vector
    for( auto & i : stopword_vec)
        cout << i.first << "," << i.second << endl; 
}

// Remove if:
//          1) Affilword appears in over 30% of records
//          2) 10 times more than one would expect (totalNaffiliation/totalRecords)*10 < lnameNaffiliationword
vector<pair<string, string>> stopword_gen(map<string, unsigned int> affilword_counts, // NRecords each affilword appears in
                                          map<string, pair< unsigned int, map<string, unsigned int>>> lname_map, // Number of records to lname and records for each associated  affilwords,
                                          unsigned int nRecords // total number of records
                                          ){
    // Initialize vector that will hold stopword pairs
    vector<pair<string,string>> stopword_pairs;

    // Loop through each author and associated affiliation words
    for(map<string, pair< unsigned int, map<string, unsigned int>>>::const_iterator it = lname_map.begin();
         it != lname_map.end();
         ++it){

        // We skip the author if their name has less than 100 records 
        // (small number of authors likely to have frequent affiliation words)
        unsigned int const & lnameRecordCount = it->second.first;
        if( lnameRecordCount < 100){
            continue;
        } else{
            // Otherwise we need to go through every affiliation word of each lname
            for(map<string, unsigned int>::const_iterator lnameAffilMap = it->second.second.begin();
                lnameAffilMap != it->second.second.end();
                ++lnameAffilMap){
                // If the name is in more than 30% percent of records add it to our vector
                if(lnameAffilMap->second > lnameRecordCount*.30){
                    // Push back of stopword vector with lname and affilword
                    stopword_pairs.push_back(make_pair(it->first,lnameAffilMap->first));
                    continue; // Go forward in loop
                }

                // Add to stopword vector if it occurs 10x more than average
                // Create reference to number of time affilword occurs overall
                unsigned int const & affilCount = affilword_counts.find(lnameAffilMap->first)->second;
                if( (affilCount/nRecords)*10  < (lnameAffilMap->second / lnameRecordCount) ){
                    stopword_pairs.push_back(make_pair(it->first,lnameAffilMap->first));
                    continue; // Go forward in loop
                }
                // If neither of those conditions are met, we go forward in our loops
            }
        }
    }
    // Return our list of lname, stopword pairs
    return(stopword_pairs);
}

// Works - aggregate map < lastname , map<affilword, count>>> to map<affilword, count>>
map<string,unsigned int> affilword_count(map<string,pair<unsigned int, map<string, unsigned int>>> const &map_by_name){
    // Initialize  map that we will output
    map<string,unsigned int> affilmap; // Count total number of records with affiliation words

    // Double loop through the maps
    for(map<string,pair< unsigned int, map<string, unsigned int>>>::const_iterator it = map_by_name.begin(); it != map_by_name.end(); ++it){
        for(map<string, unsigned int>::const_iterator it2 = it->second.second.begin(); it2 != it->second.second.end(); ++it2){
            // If we can find the element we add to it, otherwise we are inserting it
            affilmap[it2->first] += it2->second;
        }
    }
    return(affilmap);
}

// Function that aggregates multiple map < lastname , pair< recordcount, map< affilword, count> > > to map of total affiliation words for each lname
//      and total number of records for each lname
// Also return total number of records
tuple<unsigned int, map<string, pair<unsigned int, map<string, unsigned int> > > >
    mapcombine(vector<map<string, pair<unsigned int, map<string, unsigned int>>>> const &vecofmaps){

        // Initialize the result map and number of records count
        map<string,pair<unsigned int,  map<string,unsigned int>>> finalmap;
        unsigned int nRecords = 0;

        // Loop through the vector of maps
        for(vector<map<string, pair<unsigned int, map<string, unsigned int>>>>::const_iterator it = vecofmaps.begin(); it != vecofmaps.end(); ++it){
            // First time in the loop we just take the entire map and assign it to final map (since it's empty beforehand)
            if(it == vecofmaps.begin()){
                finalmap = *it;
                continue;
            } else{
                // Loop through pairs of each subsequent map to insert keys/iterate affiliation counts
                for(map<string, pair<unsigned int, map<string, unsigned int>>>::const_iterator it2 = it->begin(); it2 != it->end(); ++it2){
                    if(finalmap.find(it2->first) == finalmap.end()){
                        // If key of map is not in finalmap, insert it
                        finalmap.insert(*it2);
                    } else{
                        // Define reference to inner pair for easier-to-read code
                        pair<unsigned int, map<string,unsigned int>> & lnamepair = finalmap.find(it2->first)->second;

                        // Add count of new records associated with last name
                        lnamepair.first += it2->second.first;

                        // Define reference to affiliation map
                        map<string, unsigned int> & affilmap = lnamepair.second;
                        // Otherwise enter into affiliations loop
                        for(map<string, unsigned int>::const_iterator it3 = it2->second.second.begin(); it3 != it2->second.second.end();  ++it3){       
                            // If we can find the element we add to it, otherwise we are inserting it
                            affilmap[it3->first] += it3->second;
                        }
                    }
                }
            }

        }
        return(make_tuple(nRecords, finalmap));
}

// Works
template<typename K, typename V>
void print_map(std::map<K,V> const &m)
{
	for (auto const& pair : m) {
		std::cout << "{" << pair.first << ": " << pair.second << "}\n";
	}
}


int main(){
    /*
    map<string,map<string, unsigned int>> first;
    map<string,map<string,unsigned int>> second;
    map<string,map<string,unsigned int>> finalmap;
    map<string,unsigned int> aggmap;

    first["a"].emplace("a", 10);
    first["b"].emplace("a",30);
    first["c"].emplace("a",50);
    first["d"].emplace("a",70);
    first["a"].emplace("b",10);
    first["b"].emplace("c",30);
    first["c"].emplace("c",30);
    first["d"].emplace("c",30);

    second["a"].emplace("a", 10);
    second["b"].emplace("a",30);
    second["c"].emplace("a",50);
    second["d"].emplace("a",70);
    second["a"].emplace("b",30);

    vector<map<string, map<string,unsigned int>>> mapvec{first,second};
    aggmap = mapagg(first);
    finalmap = mapcombine(mapvec);

    print_map(aggmap);
    cout << "Final map under key a" << endl;
    print_map(finalmap["a"]);
    cout << "Final map under key b" << endl;
    print_map(finalmap["b"]);
    */
   vector<pair<string,string>> vec{make_pair("One","huh"), make_pair("Two","bug"),make_pair("Three","Tree")};
   print_to_file(vec);
   read_file();
}
