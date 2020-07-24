#include "cFirstname_Cmp_Unit_Test.h"

using namespace std;
// Create our pointer to the nickname class
string path_to_file = "/Users/RobertTeresi/Documents/Github/Pubmed_Medline_Author_Disambiguation-orig/nickname_test.csv";
cNicknames* nnameptr = new cNicknames(path_to_file);

void compare(string & name1,string & name2,cNicknames* nnameptr){
    cout << namecmp2(name1,name2,nnameptr) << endl;;
}

int main(){
// Now run our tests, making sure we can reproduce every desired result

// Use dummy nicknames in the test file
string name1 = "One";
string name2 = "Two";

// Works
cout << "Nicknames should get 6 - Result is:" << endl;
compare(name1,name2,nnameptr);


// Exact matches return 11
name1 = "ExactMatch";
name2 = "ExactMatch";

cout << "Exact Match should get 11\nResult is:" << endl;
compare(name1,name2,nnameptr);

cout << "Exact matches except for a hyphen or space get 10.\n Results are:" << endl;

name1 = "Exact Match";
name2 = "ExactMatch";
compare(name1,name2,nnameptr);

name1 = "Exact-Match";
name2 = "ExactMatch";
compare(name1,name2,nnameptr);

cout << "Hyphenated names, vs. hyphen with initial (Jean-franc vs. Jean-f) get 9"
        "\n Results is:" << endl;
name1 = "Jean-franc", name2 = "Jean-f";
compare(name1,name2,nnameptr);

cout << "Hyphen with initial vs. only first part (Jean-f vs. Jean) get 8.\n"
        "Result is:" << endl;
name1 = "Jean-f", name2 = "Jean";
compare(name1,name2,nnameptr);

cout << "Hyphenated names vs. first part only (Jean-franc vs. Jean) get 7.\n"
        "Result is:" << endl;
name1 = "Jean-franc", name2 = "Jean";
compare(name1,name2,nnameptr);

cout << "One edit distance gets 5.\n"
        "Results are:" << endl;
name1 = "Jean-frfnc", name2 = "Jean-franc";
compare(name1,name2,nnameptr);

name1 = "Jean-francc", name2 = "Jean-franc";
compare(name1,name2,nnameptr);

cout << "More than two letters to start first name match other name get 4. (Zak vs. Zakry) \n"
        "Results are:" << endl;
name1 = "Jean-francc", name2 = "Jea";
compare(name1,name2,nnameptr);

cout << "Following should be 0:" << endl;
name1 = "Jean-francc", name2 = "Jeak";
compare(name1,name2,nnameptr);

cout << "Two letters to start first name match other name get 3. (Za vs. Zakry)\n"
        "Results are:" << endl;
name1 = "Jean-francc", name2 = "Je";
compare(name1,name2,nnameptr);


cout << "Following should be 0:" << endl;
name1 = "Jean-francc", name2 = "Jekk";
compare(name1,name2,nnameptr);

cout << "A full name matching initials gets 2 points (Jean-Frank vs. JF)\n" 
        "Results are:" << endl;
name1 = "Jean-francc", name2 = "Jf";
compare(name1,name2,nnameptr);
name1 = "Jean francc", name2 = "Jf";
compare(name1,name2,nnameptr);

cout << "One or both names missing gets 1 point.\n Result is:" << endl;
name1 = "", name2 = "";
compare(name1,name2,nnameptr);
cout << "Otherwise get 0.\n Result is:" << endl;
name1 = "gufrgle", name2 = "flurgle";
compare(name1,name2,nnameptr);

int keep_going = 1;
while(keep_going){
    cout << "Would you like to enter in custom names to test?'\n"
            "Press 1 for yes and 0 for no." << endl;
    cin >> keep_going;
    if(keep_going){
        cout << "Enter in the two names to compare" << endl;
        cin >> name1 >> name2;
        cout << "Thank you. Your result is."  << endl;
        compare(name1,name2,nnameptr);
    }
}
}