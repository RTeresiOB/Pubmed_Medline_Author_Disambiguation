import numpy as np
#import psutil
from datetime import datetime
import os
import zipfile
import xml.sax
import multiprocessing
import sys
import pandas as pd
import pubmed_parser
import gzip
import re
from pathlib import Path


## Downloads zipped xml from the medline website
def download_pubmed_data():
    from subprocess import call
    call("./pubmed_update")
    
## Reads stoplist
def read_stoplist(stoplistpath):  # a path to a .txt file with stopwords separated by ", "
    with open(stoplistpath, 'r') as file:
        data = file.read().split(', ')
        return(data)

## Extract an author's email address from their list of affiliations
def extract_email_from_affiliations(affiliation):
    try:
        return(re.findall("[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+", affiliation)[0])
    except:
        return('')

## This function will convert the string to lower and take out stopwords
def clean_string(string, stopword_list = None, rm_stopwords = True, mesh =  False):
    
    ## If we don't supply a stopword list, but don't have remove stopwords set to False, we raise an error
    if stopword_list is None and rm_stopwords:
        raise Exception('Please Supply a Stopword List or set rm_stopwords to False')
        
    ## First, convert the string to lower
    string = string.lower()
    
    if mesh: ## The mesh listings need a slightly different treatment
        if rm_stopwords:
            
            stopword_list = [stops + ';' for stops in stopword_list] # This makes it easier to  get rid of extra semi-colons
            ## Remove stopwords
            string = re.sub(r'([ ]?d\d{6}\:' + r'|[ ]?d\d{6}\:'.join(stopword_list) + r')', '', string)
        
        
        ## Now I only want to keep the unique ID
        string = re.sub(r'[ ]?(d\d{6})(\:[a-z\-\'\.\, ]+)', r'\1', string)
        return(string)
    else:       
        string = re.sub(r'\W+|\b[a-z]\b', ' ', string)
            
            ## Now I only want to keep
        ## Remove stopwords if the option is set to true
        if rm_stopwords:  
                
            string = re.sub(r'\b(' + r'| '.join(stopword_list) + r')\b\s*', ' ', string)
            
        ## Trim whitespaces (induced by cleaning or otherwise)
        string = string.strip()                 # From beginning and end
        string = re.sub(r'[ ]+'," ",string)     # From the middle
        return(string)
    
    
## Worker Function - takes compressed xml from medline and transforms it into a corresponding clean, compressed csv
def process_data(filepath, filename, respath, rm_stopwords = True): #  rm_stopwords set true to remove appropriate stopwords from title, affiliatio, and mesh fields
    print(filename + " - start")
    begin = datetime.now()
    print(filename + " - load data")
    df = pubmed_parser.parse_medline_xml(filepath)
    df = pd.DataFrame(df)
    print(filename + " - save to " + respath)
    
    ## Get rid of some unneeded columns -- can maybe work this into the parser later
    df = df.drop(['publication_types', 'chemical_list','keywords', 'doi', 'references',\
             'delete','pmc','other_id','medline_ta','nlm_unique_id','issn_linking','country'],axis=1)
    
    ## Replace missing values (mesh, affiliation, or title) with ''
    df[['mesh_terms','affiliations','title']] = df[['mesh_terms','affiliations','title']].fillna('')
    
    ## Apparently only some of the xmls read in the authors and affiliations as list
    if type(df.loc[2,'authors']) is not list:
        df['authors'] = df['authors'].apply(lambda x:x.split(";"))
        df['affiliations'] = df['affiliations'].apply(lambda x:x.split(";"))
        
    ## "Explodes" Name and Affiliation columns - makes data by author-article, not article
    df = df.apply(pd.Series.explode)
    
    ## Extract emails
    df['email'] = [extract_email_from_affiliations(affiliation) for affiliation in df['affiliations']]
    
    ## Clean strings - take out stopwords, non-alphanumeric characters, and single  character words
    if rm_stopwords:
        df['title'] =  [clean_string(title,read_stoplist(title_stoplist_path))\
          for title in df['title']]
        df['affiliations'] =  [clean_string(affiliation,read_stoplist(affiliations_stoplist_path))\
          for affiliation in df['affiliations']]
        df['mesh_terms'] =  [clean_string(mesh_term,read_stoplist(mesh_stoplist_path), mesh = True)\
          for mesh_term in df['mesh_terms']]
    else:
        df['title'] =  [clean_string(title,rm_stopwords = False) for title in df['title']]
        df['affiliations'] =  [clean_string(affiliation,rm_stopwords = False) for affiliation in df['affiliations']]
        df['mesh_terms'] =  [clean_string(mesh_term,rm_stopwords = False) for mesh_term in df['mesh_terms']]
    ## Save to a compressed csv in the results directory
    df.to_csv(respath, compression = "gzip", sep=",")
    end = datetime.now() - begin
    print(filename + " - done - " + str(np.round(end.seconds / 60)) + "min")
    
def start(text_data_dir, res_dir, nprocs=26):
    '''
    entry function

    text_data_dir: folder of raw data
    text_res_dir: folder of output
    verbose: int. Information is printed every N records
    nprocs: number of cores in parallel
    '''

    p = multiprocessing.Pool(processes=nprocs)
    for dirpath, dirnames, filenames in os.walk(text_data_dir):
        for filename in filenames[:4]:
            if "gz" in filename and 'md5' not in filename and 'copy' not in filename:
                filepath = os.path.join(dirpath, filename)
                print(filepath)
                res_name = filename.split(".")[0] + ".csv.gz"
                respath = os.path.join(res_dir, res_name)
                if not os.path.exists(respath):
                    pass
                else:
                    p.apply_async(process_data, args = (filepath,filename,respath))
    p.close()
    p.join()

## The following appends all of the smaller zipped files together in a new "full" file.
    #### TO DO: This runs on a single core. We can parallelize this if we make it a recursive function
    # https://stackoverflow.com/questions/42246888/python-recursive-algorithm-multiprocess
def combine(res_dir, combined_dir):
    files = []
    for dirpath, dirnames, filenames in os.walk(res_dir):
        for filename in filenames:
            print(filename)
            if "gz" in filename and 'md5' not in filename and 'copy' not in filename:
                filepath = os.path.join(dirpath, filename)
                print(filepath)
                files.append(filepath)
    n = 0
    respath = os.path.join(combined_dir, "full_pubmed.csv.gz")
    for f in files:
        if n ==0:
            combined_df  = pd.read_csv(f,compression='gzip', sep = ",")
            
            combined_df.to_csv(respath, compression = "gzip",sep = ",")
            del  combined_df
        else:
            combined_df = pd.read_csv(f,compression='gzip', sep = ",", lineterminator = '\n')
            combined_df.to_csv(respath,compression='gzip', sep = ",", mode='a', header=False)  
        n = n + 1   
        print(n)


def main(download_data = False, combine_data =  False):
    '''
    text_data_dir: folder of raw data
    text_res_dir: folder of output
    verbose: int. Information is printed every N records
    nprocs: number of cores in parallel
    '''
    
    if download_data:
        download_pubmed_data()
    ## Not sure if this is the best place,but we can curl in parallel (https://gist.github.com/CMCDragonkai/5914e02df62137e47f32)
    ## Set the number of processors here
    nprocs = 8
    
    ## Set the data directories
        ## Text data dir is where the script finds the source xml file from the medline website
    text_data_dir = "/Volumes/LaCie/PubmedData_Update_2019/ftp.ncbi.nlm.nih.gov/pubmed/baseline/"
        ## Res dir is where the compressed, clean csv equivalents of each xml file is stored 
    res_dir = "/Volumes/LaCie/PubmedData_Update_2019/Clean_Csvs/"
        ## Combined dir is where 
    combined_dir = "/Volumes/LaCie/PubmedData_Update_2019/Full_Pubmed/"
    
    ## Give the paths where stoplists are kept -- must be .txt files of words separated by commas and a space
    
        #### PATHS MUST BE ASSIGNED
    title_stoplist_path = "/Users/RobertTeresi/Documents/pubmed_small_stopwords_list.txt"
    affiliations_stoplist_path = "/Users/RobertTeresi/Documents/pubmed_affiliation_stopwords.txt"
    mesh_stoplist_path =  "/Users/RobertTeresi/Documents/pubmed_mesh_stopwords.txt"

    for temp_dir in [res_dir]:
        if not os.path.exists(temp_dir):
            os.makedirs(temp_dir)
    
    ## Start reads, cleans, and saves the ~1050 xml files into csv.gz files
    start(text_data_dir=text_data_dir, res_dir=res_dir, nprocs=nprocs)
    
    if combine_data:
        ## Combine appends all of the csvs into one, complete pubmed database, indexed by author-article
        combine(res_dir=res_dir, combined_dir = combined_dir)

if __name__ == "__main__":
        main()

## Make Coauthor Column look like 'D.ROY-0/S.LIN-0/J.LAGARENNE-0', maybe?