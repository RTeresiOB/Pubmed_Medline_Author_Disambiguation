// Here we will test the triplet correction prototype
#include "Triplet_Correction_Prototype_and_Test.h"

#include<iostream>
#include<vector>

using namespace std;
int main(void){
    vector<double> violating_probs1{.95,.95,.6};

  vector<double> corrected_probs1 = triplet_correct(violating_probs1);

    for (double x : corrected_probs1)
        std::cout << x << " ";
 
    return 0;

}