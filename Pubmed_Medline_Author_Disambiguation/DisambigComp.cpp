/*
 * DisambigComp.cpp
 *
 *  Created on: Dec 9, 2010
 *      Author: ysun
 */

#include "DisambigComp.h"
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <list>
#include <functional>
#include <stdexcept>

extern "C" {
	#include "strcmp95.h"
}

using std::list;

//this function is to get the incontinuous longest common subsequence of two vectors.
//for example, the mid name comparision uses the function | or the following continuous function.
template <typename Tp, typename Functor>
vector <Tp> Longest_Common_Subsequence_Incontinuous(const vector <Tp> & s1, const vector <Tp> &s2, const Functor & func)
{
    
	static const vector < Tp > emptyresult;
    if(s1.empty()||s2.empty())
        return emptyresult;
    const int m=s1.size()+1;
    const int n=s2.size()+1;
	vector <int> row(n, 0);
	vector < vector <int> > lcs(m, row);
    //int lcs[100][100];
    int i,j;
    for(i=0;i<m;i++)
        for(j=0;j<n;j++)
            lcs[i][j]=0;
	
	
    for(i=1;i<m;i++) {
        for(j=1;j<n;j++)
        {
            //if(s1[i-1]==s2[j-1])
			if ( func( s1[i-1], s2[j-1] ) )
                lcs[i][j]=lcs[i-1][j-1]+1;
            else
                lcs[i][j]=lcs[i-1][j]>=lcs[i][j-1]?lcs[i-1][j]:lcs[i][j-1];//get the upper or lefter max value
        }
    }
	i=m-2;
	j=n-2;
	list < Tp > ss;
	while(i!=-1 && j!=-1)
	{
		//if(s1[i]==s2[j])
		if ( func( s1[i], s2[j] ) )
		{
			ss.push_front(s1[i]);
			i--;
			j--;
		}
		else
		{
			if(lcs[i+1][j+1]==lcs[i][j])
			{
				i--;
				j--;
			}
			else
			{
				if(lcs[i][j+1]>=lcs[i+1][j])
					i--;
				else
					j--;
			}
		}
	}
	
    vector < Tp > ans (ss.begin(), ss.end());
    return ans;
}

template <typename Tp, typename Functor>
vector <Tp> Longest_Common_Subsequence_Continuous(const vector <Tp> & s1, const vector <Tp> &s2, const Functor & func) {
	static const vector < Tp > emptyresult;
    if (s1.empty() || s2.empty() )
        return emptyresult;

    const int m = s1.size();
    const int n = s2.size();

    vector < int > c(m, 0);
    int max, maxj,i,j;
    maxj = 0 ;
    max = 0;
    for( i = 0; i < n ; ++i )   {
		  for( j = m - 1 ; j >= 0 ; --j )   {
			  if( func (s2[i], s1[j] ) )   {
				  if ( i == 0 || j == 0 )
					  c[j] = 1;
				  else
					  c[j] = c[j-1] + 1;
			  }
			  else
				  c[j]=0;
			  if( c[j] > max )   {
				  max = c[j];
				  maxj = j;
			  }
		  }
    }

    if( max == 0 )
    	return emptyresult;
    vector <Tp> ss ( emptyresult);
	for( j = maxj - max + 1; j <= maxj ; ++j )
		ss.push_back( s1[j] );
	return ss;
}
















inline bool cSentence_JWComparator:: operator()(const string * ps1, const string * ps2) const {
	const double compres = strcmp95_modified(ps1->c_str(), ps2->c_str());
	return compres > threshold;
};




char * extract_initials(char * dest, const char * source) {
	if ( source == NULL || dest == NULL )
		return NULL;
	char * ret = dest;
	static const char delim = ' ';
	while ( *source != '\0') {
		while ( *source == delim )
			++source;
		*dest++ = *source;
		while ( *source != delim && *source != '\0' )
			++source;
	}
	*dest = '\0';
	return ret;
};

int nospacecmp(const char* str1, const char* str2){
    const char *c1, *c2;
    const char delim = ' ';
    for(c1 = str1, c2=str2; (*c1 != '\0') && (*c2 != '\0'); ++c1, ++c2 ){
        while (*c1 == delim ) ++c1;
        while (*c2 == delim ) ++c2;
        if(*c1!=*c2){
            return -1 + (*c1 > *c2)*2;
        }
    }
    return  ( *c1!='\0' ) - ( *c2!='\0' );
}

int jwcmp_old(const string & str1, const string& str2){
    const char *delim= " ";
    const unsigned int delim_size = strlen(delim);
    const double threshold  = 0.95;

    int tok_len1, tok_len2, num_tok1, num_tok2;
    double tok_score, score = 0;
    if( 0 == str1.size() || 0 == str2.size() )
        //missing!
        return 1;

    if( ( str1 == str2 ) && 0==nospacecmp( str1.c_str(), str2.c_str() )){
        return 4; //JW100
    }
    size_t pos1, prev_pos1, pos2, prev_pos2;
    pos1 = prev_pos1 = 0;
    num_tok1 = 0;
	do {
		tok_score = 0;
		pos1 = str1.find(delim, prev_pos1);
		string temp1 = str1.substr(prev_pos1, pos1 - prev_pos1);
		tok_len1 = temp1.size();
		num_tok1 += (tok_len1 > 1);

		pos2 = prev_pos2 = 0;
		num_tok2 = 0;
		do {
			pos2 = str2.find(delim, prev_pos2);
			string temp2 = str2.substr(prev_pos2, pos2 - prev_pos2);
			tok_len2 = temp2.size();
			num_tok2 += (tok_len2 > 1);
			tok_score = max_val<int>(tok_score,
					((min_val<int>(tok_len1, tok_len2) <= 1) ? 0 : strcmp95_modified(temp1.c_str(), temp2.c_str())));

			prev_pos2 = pos2 + delim_size;
		} while ( pos2!= string::npos);
		score += (tok_score > threshold);

		prev_pos1 = pos1 + delim_size;
	} while ( pos1!= string::npos);

	int min_num_tok = min_val<int>(num_tok1, num_tok2);
	double myres = ( min_num_tok == 0) ? 0 : score/min_num_tok;
	int is_same_len = (num_tok1 == num_tok2) ? 1 : 0;
	return( 2*(myres >= 0.33) + (myres >= 0.66) + (myres > 0.99) + (myres > 0.99 && min_num_tok >= 2) + (myres > 0.99 && is_same_len));

}

int jwcmp(const string & str1, const string& str2) {
	if ( str1.empty() || str2.empty() )
		return 0;
	double cmpres = strcmp95_modified(str1.c_str(), str2.c_str());
	register int score = 0;
	if ( cmpres > 0.7 )
		++score;
	if ( cmpres > 0.8 )
		++score;
	if ( cmpres > 0.9 )
		++score;
	if ( cmpres > 0.95 )
		++score;
	if ( cmpres > 0.99 )
		++score;
	
	return score;
}

int midnamecmp_old(const string & str1, const string & str2 ){

	const char * delim = " ";
	const unsigned int delim_size = strlen(delim);
	int num_names_1 = 0, num_names_2 = 0;
	double matches = 0;
	size_t pos1, prev_pos1, pos2, prev_pos2;
	pos1 = prev_pos1 = 0;
	while ( ( pos1 = str1.find(delim, prev_pos1)) != string::npos ) {
		++ num_names_1;
		pos2 = prev_pos2 = 0;
		while ( ( pos2 = str2.find(delim, prev_pos2)) != string::npos ) {
			++num_names_2;
			if ( str1.at(pos1 + delim_size) == str2.at(pos2 + delim_size) )
				matches += 1;
			prev_pos2 = pos2 + delim_size;
		}
		prev_pos1 = pos1 + delim_size;
	}

	int min_num =  min_val<int>(num_names_1, num_names_2);
	int missing = ( min_num == 0 )? 1:0 ;
	double raw = missing? 0: matches/min_num ;
	return (missing + 2*(raw > 0.33) + (raw > 0.67) + (raw > 0.99));
}

int midnamecmp_old2(const string & str1, const string & str2 ){
	
	static std::equal_to<char> char_compare;
	/*
	const char * delim = " ";
	const unsigned int delim_size = strlen(delim);

	size_t pos, prev_pos;
	pos = prev_pos = 0;
	vector < char > vec1, vec2;
	while ( ( pos = str1.find(delim, prev_pos)) != string::npos ) {
		prev_pos = pos + delim_size;
		vec1.push_back(str1.at(prev_pos));
	}
	pos = prev_pos = 0;
	while ( ( pos = str2.find(delim, prev_pos)) != string::npos ) {
		prev_pos = pos + delim_size;
		vec2.push_back(str2.at(prev_pos));
	}
	 */
	const vector < char > vec1(str1.begin(), str1.end() );
	const vector < char > vec2(str2.begin(), str2.end() );
	
	if ( vec1.empty() && vec2.empty() )
		return 2;

	if ( vec1.empty() || vec2.empty() )
		return 1;
	
	int score;
	const int matches = Longest_Common_Subsequence_Continuous<char, std::equal_to<char> >(vec1, vec2, char_compare).size();

	if ( matches == min_val<int>(str1.size(), str2.size() ) )
		score = 3;
	else
		score = 0;
		
	return score;
}

int midnamecmp ( const string & s1, const string & s2) {
	if ( s1.empty() && s2.empty() )
		return 2;

	if ( s1.empty() || s2.empty() )
		return 1;

	const char * p1 = s1.c_str();
	const char * p2 = s2.c_str();

	while ( *p1 != '\0' && *p2 != '\0') {
		if ( *p1++ != * p2++)
			return 0;
	}
	return 3;
}

unsigned int langcmp(const string &lang1, const string & lang2){
	// Return 2 if both languages are english
    if(lang1 == "eng" && lang2 == "eng") return 2;
    // Return 1 if one of the languages is English
    else if(lang1 == "eng" || lang2 =="eng") return 1;
    // Return 3 if it is a non-english match - "und" treated as match for any non-english
    else if ( (lang1 == lang2) ||
             (lang1 == "und" || lang2 == "und")) return 3;
    // Return 0 if both non-english languages that don't match
    else return 0;
}


int distcmp(const string & inputlat1, const string & inputlon1, const string & inputctry1, const char * inputstreet1,
			const string & inputlat2, const string & inputlon2, const string & inputctry2, const char * inputstreet2 ){
	/*
//    printf("DISTCOMP:\n");
    // Extreme points of contiguous 48
    double northernmost=4938;
    double southernmost=2454;
    double easternmost=-6695;
    double westernmost=-12451;

    double dist;
    int streetmatch;

    latlon *ll1 = (latlon*)latlon1, *ll2 = (latlon*)latlon2;
    int missing = ((abs(ll1->lat)<0.0001 && abs(ll1->lon)< 0.0001) || (abs(ll2->lat)<0.0001 && abs(ll2->lon)<0.0001));
    int in_us = ll1->lat < northernmost && ll1->lat > southernmost &&
                ll1->lon < easternmost && ll1->lon > westernmost &&
                ll2->lat < northernmost && ll2->lat > southernmost &&
                ll2->lon < easternmost && ll1->lon > westernmost;
    size = size;

    dist = 3963.0 * acos(sin(ll1->lat/DEG2RAD) * sin(ll2->lat/DEG2RAD) + cos(ll1->lat/DEG2RAD) * cos(ll2->lat/DEG2RAD) *  cos(ll2->lon/DEG2RAD -ll1->lon/DEG2RAD));
    //if(dist > 1){
        //printf("\targs: %f, %f ; %f, %f\n", ll1->lat, ll1->lon, ll2->lat, ll2->lon);
        //printf("\traw: %f\n", dist);
    //}
    streetmatch = (((latlon*)latlon1)->street!=NULL) &&
                  (((latlon*)latlon2)->street!=NULL) &&
                  (((latlon*)latlon1)->street[0] != '\0')&&
                  (((latlon*)latlon2)->street[0] != '\0')&&
                  (strcmp(((latlon*)latlon1)->street, ((latlon*)latlon2)->street)==0);

    return missing +
           in_us ?
               2*(dist < 100) + (dist < 75) + (dist < 50) + 2*(dist < 10) +
                   ((dist < 1) &&  streetmatch):
               2*(dist < 100) + (dist < 75) + (dist < 50) + (dist < 10);
    */

	static const double R = 3963.0; //radius of the earth is 6378.1km = 3963 miles
	static const double DEG2RAD = 5729.58;
    static const double northernmost = 4938;
    static const double southernmost = 2454;
    static const double easternmost = -6695;
    static const double westernmost = -12451;

    const double lat1 = atof(inputlat1.c_str());
    const double lon1 = atof(inputlon1.c_str());
    const double lat2 = atof(inputlat2.c_str());
    const double lon2 = atof(inputlon2.c_str());

    const double missing_val = 0.0001;
    int missing = ( ( fabs(lat1) < missing_val && fabs(lon1) < missing_val ) ||
    				( fabs(lat2) < missing_val && fabs(lon2) < missing_val) ) ? 1 : 0;
    int in_us = ( 	lat1 < northernmost && lat1 > southernmost &&
    				lon1 < easternmost && lon1 > westernmost &&
    				lat2 < northernmost && lat2 > southernmost &&
    				lon2 < easternmost && lon2 > westernmost ) ? 1 : 0;

    const double radlat1 = lat1/DEG2RAD;
    const double radlon1 = lon1/DEG2RAD;
    const double radlat2 = lat2/DEG2RAD;
    const double radlon2 = lon2/DEG2RAD;

    const double cos_lat1 = cos(radlat1);
    const double cos_lat2 = cos(radlat2);
    const double cos_lon1 = cos(radlon1);
    const double cos_lon2 = cos(radlon2);
    const double sin_lon1 = sin(radlon1);
    const double sin_lon2 = sin(radlon2);


    // R=radius, theta = colatitude, phi = longitude
    // Spherical coordinate -> Cartesian coordinate:
    // x=R*sin(theta)*cos(phi) = R*cos(latitude)*cos(longitude)
    // y = R*sin(theta)*sin(phi) = R*cos(latitude)*sin(longitude)
    // z = R*cos(phi) = R * cos(longitude)
    // Cartesion distance = sqrt( ( x1-x2)^2 + (y1-y2)^2 + (z1 - z2)^2 );
    // Spherical distance = arccos( 1 - (Cartesian distance)^2 / (2*R^2) ) * R;

    const double x1 = cos_lat1 * cos_lon1;
    const double x2 = cos_lat2 * cos_lon2;
    const double y1 = cos_lat1 * sin_lon1;
    const double y2 = cos_lat2 * sin_lon2;
    const double z1 = cos_lon1;
    const double z2 = cos_lon2;

    const double cart_dist = sqrt( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2) );
    const double dist = acos(1 - cart_dist*cart_dist / 2 ) * R;

    int streetmatch = ( strlen(inputstreet1) != 0 && strlen( inputstreet2) != 0 &&
                  (strcmp(inputstreet1, inputstreet2) == 0 )) ? 1 : 0;

    return missing +
           in_us ?
               2*(dist < 100) + (dist < 50) + 2*(dist < 10) +
                   ((dist < 1) &&  streetmatch):
               2*(dist < 100) + (dist < 50) + (dist < 10);

}



int latloncmp(const string & inputlat1, const string & inputlon1,
				const string & inputlat2, const string & inputlon2 ){

	static const double R = 3963.0; //radius of the earth is 6378.1km = 3963 miles
	static const double pi = 3.1415926;
	//rad = degree * pi / 180
	static const double DEG2RAD = pi / 180 ;

    const double lat1 = atof(inputlat1.c_str());
    const double lon1 = atof(inputlon1.c_str());
    const double lat2 = atof(inputlat2.c_str());
    const double lon2 = atof(inputlon2.c_str());

    const double missing_val = 0.0001;
    int missing = ( ( fabs(lat1) < missing_val && fabs(lon1) < missing_val ) ||
    				( fabs(lat2) < missing_val && fabs(lon2) < missing_val) ) ? 1 : 0;

    if ( missing )
    	return 1;

    const double radlat1 = lat1 * DEG2RAD;
    const double radlon1 = lon1 * DEG2RAD;
    const double radlat2 = lat2 * DEG2RAD;
    const double radlon2 = lon2 * DEG2RAD;

    const double cos_lat1 = cos(radlat1);
    const double cos_lat2 = cos(radlat2);
    const double cos_lon1 = cos(radlon1);
    const double cos_lon2 = cos(radlon2);
    const double sin_lon1 = sin(radlon1);
    const double sin_lon2 = sin(radlon2);
    const double sin_lat1 = sin(radlat1);
    const double sin_lat2 = sin(radlat2);


    // R=radius, theta = colatitude, phi = longitude
    // Spherical coordinate -> Cartesian coordinate:
    // x=R*sin(phi)*cos(theta) = R*cos(latitude)*cos(longitude)
    // y = R*sin(phi)*sin(theta) = R*cos(latitude)*sin(longitude)
    // z = R*cos(phi) = R * sin(latitude)
    // Cartesion distance = sqrt( ( x1-x2)^2 + (y1-y2)^2 + (z1 - z2)^2 );
    // Spherical distance = arccos( 1 - (Cartesian distance)^2 / (2*R^2) ) * R;

    const double x1 = cos_lat1 * cos_lon1;
    const double x2 = cos_lat2 * cos_lon2;
    const double y1 = cos_lat1 * sin_lon1;
    const double y2 = cos_lat2 * sin_lon2;
    const double z1 = sin_lat1;
    const double z2 = sin_lat2;

    const double cart_dist_sq = (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) + (z1-z2)*(z1-z2);
    const double dist = acos(1 - cart_dist_sq / 2) * R;


    if ( dist < 1.0 )
    	return 5;
    else if ( dist < 10 )
    	return 4;
    else if ( dist < 25)
    	return 3;
    else if ( dist < 50 )
    	return 2;
    else
    	return 1;

}

int streetcmp(const string& inputstreet1, const string& inputstreet2) {
    int streetmatch = ( inputstreet1.size() != 0 && inputstreet2.size() != 0 && (inputstreet1 == inputstreet2 )) ? 1 : 0;
    return streetmatch;
}

int countrycmp(const string & country1, const string & country2 ) {
	static const string US_label ("US");
	int score = 0;
	if ( country1 == country2 ) {
		++score;
		if ( country1 == US_label)
			++score;
	}
	return score;
}

int classcmp(const string &class1, const string& class2 ){
    return ( class1 == class2 )? 1 : 0;
}

int coauthorcmp(const string &coauthor1, const string& coauthor2 ){
    return ( coauthor1 == coauthor2 )? 1 : 0;
}

// PUBMED CHANGE Taking out jwcmp - assignee not used
/*
int asgcmp(const string & asg1, const string & asg2, const map<string, std::pair<string, unsigned int> > * const asg_table_pointer){
	map<string, std::pair<string, unsigned int> >::const_iterator p1, p2;
	p1 = asg_table_pointer->find(asg1);
	p2 = asg_table_pointer->find(asg2);

	if ( p1 == asg_table_pointer->end() || p2 == asg_table_pointer->end() ) {
		std::cout << "Error: either assignee is not found in the assignee tree. "<< asg1 << " or " << asg2 << std::endl;
		throw std::runtime_error("Assignee comparison error.");
	}
	int score = 0;
	if ( p1->second.first == p2->second.first && p1->second.first.size() > 3 ) {
		score = Jaro_Wrinkler_Max;
		if ( p1->second.second < 100 || p2->second.second < 100)
			score += 1;
		//else if ( p1->second.second < 1000 || p2->second.second < 1000 )
		//	score += 1;
	}
	else {
		score = jwcmp(asg1, asg2);
	}
	return score;
}
*/

int asgcmp ( const string & s1, const string &s2) {
	if ( s1.empty() || s2.empty() )
		return 1;

	double cmpres = strcmp95_modified(s1.c_str(), s2.c_str());
	if ( cmpres > 0.9 )
		return 4;
	else if ( cmpres > 0.8 )
		return 3;
	else if ( cmpres > 0.7 )
		return 2;
	else
		return 0;
}

int asgcmp_to_test(const vector <string> & asg1, const vector <string> & asg2,
			   const map<string, std::pair<string, unsigned int> > * const asg_table_pointer){
	map<string, std::pair<string, unsigned int> >::const_iterator p1, p2;
	p1 = asg_table_pointer->find(asg1.at(0));
	p2 = asg_table_pointer->find(asg2.at(0));
	
	if ( p1 == asg_table_pointer->end() || p2 == asg_table_pointer->end() ) {
		std::cout << "Error: either assignee is not found in the assignee tree. "<< asg1.at(0) << " or " << asg2.at(0) << std::endl;
		exit(3);
	}
	int score = 0;
	if ( p1->second.first == p2->second.first && p1->second.first.size() > 3 ) {
		score = 6;
		if ( p1->second.second < 100 || p2->second.second < 100)
			score += 2;
		else if ( p1->second.second < 1000 || p2->second.second < 1000 )
			score += 1;
	}
	else {
		const double jw_threshold = 0.9;
		static const cSentence_JWComparator sjw(jw_threshold);
		vector < const string * > vec_asg1;
		vector < string >::const_iterator q1 = asg1.begin();
		for ( ++q1; q1 != asg1.end(); ++q1 )
			vec_asg1.push_back(&(*q1));
		
		vector < const string * > vec_asg2;
		vector < string >::const_iterator q2 = asg2.begin();
		for ( ++q2; q2 != asg2.end(); ++q2 )
			vec_asg2.push_back(&(*q2));
		
		score = Longest_Common_Subsequence_Incontinuous <const string *, cSentence_JWComparator>(vec_asg1, vec_asg2, sjw).size();
	}
	return score;
}

int name_compare( const string & s1, const string & s2, const unsigned int prev, const unsigned int cur) {
	if ( s1.empty() || s2.empty() )
		return 1;
	if ( s1 == s2 )
		return 4;
	int misspell_score = is_misspell(s1.c_str(), s2.c_str()) ;
	if ( misspell_score )
		return 3;

	unsigned int abbrev_score = is_abbreviation ( s1.c_str(), s2.c_str());
	if ( abbrev_score == 0 )
		return 0;
	else if ( cur != 0 && cur <= abbrev_score ) {
		if ( prev == 0 || ( prev != 0 && prev > abbrev_score ) )
			return 4;
	}

	return 2;

}

// Function created from Torvik's 2009 paper, where he updates his methodology
int torvik_name_compare(const string & s1, const string & s2, cNicknames* nnameptr){
    // Initialize some variables
    size_t count = 0;
    bool is_initials = false;
    const char * cs1 = s1.c_str(), * cs2 = s2.c_str(); // c-string versions of inputs
    // First check if either string is empty
    if(s1.size() <= 1 || s2.size() <= 1){
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
