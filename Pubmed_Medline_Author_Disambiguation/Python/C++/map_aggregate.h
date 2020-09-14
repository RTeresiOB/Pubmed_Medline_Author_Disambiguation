#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <tuple> // assign function result to two variables
#include <utility> // std::pair/std::make_pair

// Function is not yet completely functional
using namespace std;

void affil_stopword_find(vector<map<string, pair<unsigned int, map<string, unsigned int>>>> listofdicts, string outfilename);
void print_to_file(vector<pair<string, string>> stopword_vec, string outfilepath);
void read_file(string infilepath);
vector<pair<string, string>> stopword_gen(map<string, unsigned int> affilword_counts, // NRecords each affilword appears in
                                          map<string, pair< unsigned int, map<string, unsigned int>>> lname_map, // Number of records to lname and records for each associated  affilwords,
                                          unsigned int nRecords // total number of records
                                          );
map<string,unsigned int> affilword_count(map<string,pair<unsigned int, map<string, unsigned int>>> const &map_by_name);
tuple<unsigned int, map<string, pair<unsigned int, map<string, unsigned int> > > >
    mapcombine(vector<map<string, pair<unsigned int, map<string, unsigned int>>>> const &vecofmaps);
template<typename K, typename V>
void print_map(std::map<K,V> const &m);
