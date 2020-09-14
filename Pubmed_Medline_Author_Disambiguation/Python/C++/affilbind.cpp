#include"pybind11/pybind11.h"
#include"pybind11/stl.h"
#include"map_aggregate.h"

PYBIND11_MODULE(affilbind, m){
    m.doc() = "Take in vector of dictionaries and output affiliation stopwords to .txt file."; // Optional module docstring
    m.def("affil_stopword_find", &affil_stopword_find,
    "Take in vector of dictionaries and output affiliation stopwords to .txt file.\n"
    "Argument 1: Vector/List of dictionaries of format {last name : [recordcount,  {affiliationword: count}]}).\n"
    "Argument 2: (String) Path to file where stopwords will be output.");
}
