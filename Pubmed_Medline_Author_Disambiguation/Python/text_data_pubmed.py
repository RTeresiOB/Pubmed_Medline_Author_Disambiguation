#!/usr/bin/env python3
# -*- coding: utf-8 -*-

'''
The preprocessing script for Pubmed/Medline Disambiguation.

@author: RobertKTeresi
'''

# Import libraries
import os
import sys
import re
import xml.sax
from pathlib import Path
import shutil # Copy files to another directory
from multiprocessing import Pool
import numpy as np
import pandas as pd
from datetime import datetime
import zipfile
import gzip
from pubmed_parser import medline_parser

'''
Assign Global Variables:
    
    working_dir: Optional variable to help  avoid redundancy with typing other
                    paths.
    
    nprocs: Number of processors you would like to use. Multiprocessing is
                utilized to clean thousands of (~15.Mb) datasets in parallel
                and then to append them all together.
    Data Directories:
        
        text_data_dir: Where the script finds the source xml file from
                        the medline website.
        res_dir: Where the compressed, clean csv equivalents of each xml
                    file is stored.
        combined_dir: Where the full, cleaned dataset is stored.
    
    Stoplist Paths:
        
        Paths to stoplists to take out uninformative data from title,
        affiliation, and MeSH fields. Script assumes the contents of the
        stopword files are ',' separated lists of the stopwords.
        
        If you do not want to remove
        stopwords, you can change the global variable stop_remove
        to False.
        
        title_stop_path: Path to .txt file with title stopwords.
        affil_stop_path: Path to .txt file with affiliation stopwords.
        mesh_stop_path:  Path to .txt file with MeSH stopwords.
        
    stop_remove: True/False boolean value that will tell the script whether
                    or not to remove stopwords from title, affiliation, and
                    MeSH fields.
        
'''

working_directory = ("/Users/RobertTeresi/Documents/GitHub/"
                    "Pubmed_Medline_Author_Disambiguation-orig/"
                    "Pubmed_Medline_Author_Disambiguation")

nprocs = 8

text_data_dir = ("/Volumes/LaCie/PubmedData_Update_2019/ftp.ncbi.nlm.nih.gov/"
                 "pubmed/baseline/")
res_dir = "/Volumes/LaCie/PubmedData_Update_2019/Clean_Csvs/"
combined_dir = "/Volumes/LaCie/PubmedData_Update_2019/Full_Pubmed/"

# THESE PATHS MUST BE ASSIGNED
title_stop_path = (working_directory +
                   "Stopword_lists/pubmed_small_stopwords_list.txt")
affil_stop_path = (working_directory +
                   "Stopword_lists/pubmed_affiliation_stopwords.txt")
mesh_stop_path =  (working_directory +
                   "Stopword_lists/pubmed_mesh_stopwords.txt")


def download_pubmed_data():
    """Download zipped xml from the medline website."""
    from subprocess import call
    call("./pubmed_update")
    

def process_data(filepath, filename, respath, rm_stopwords = True,
                 stop_paths = None):
    """
    Take zipped xml from medline and transform it into a clean, gzipped csv.
    
    rm_stopwords set true to remove appropriate stopwords from title,
    affiliation, and mesh fields.
    stop_paths = List or tuple of paths to title, affiliation, and mesh
                stopwords in that order. Default is none - take paths from
                main function, where the paths are defined globally
    """
    def clean_string(string, stopword_list = None,
                 rm_stopwords = True, mesh =  False):
        """Convert the string to lower and take out stopwords."""
    
        # If we don't supply a stopword list, but don't have remove stopwords set to False, we raise an error
        if stopword_list is None and rm_stopwords:
            raise Exception('Please Supply a Stopword List or set rm_stopwords'
                            'to False')
            
        # Mesh listings need a slightly different treatment
        if mesh: 
            # Remove MeSH Codes
            string = [ re.sub(r'([ ]?d\d{6}\:','',x) for x in string]
            if rm_stopwords:
                #stopword_list = [stops + '/' for stops in stopword_list] # This makes it easier to  get rid of extra semi-colons
                # Remove stop words
                string = [x for x in string if x not in stopword_list]

            # Convert to '/' separated string
            string = '/'.join(string)
            

            #string = re.sub(r'([ ]?d\d{6}\:' + r'|[ ]?d\d{6}\:'.join(stopword_list) + r')', '', string)
            # Now I only want to keep the unique ID
            #string = re.sub(r'[ ]?(d\d{6})(\:[a-z\-\'\.\, ]+)', r'\1', string)
            return(string)
        else:       
            # First, convert the string to lower
            string = string.lower()
            string = re.sub(r'\W+|\b[a-z]\b', ' ', string)
                
            # Remove stopwords if the option is set to true
            if rm_stopwords:  
                    
                string = re.sub(r'\b(' + r'| '.join(stopword_list) + r')\b\s*',
                                ' ', string)
                
            # Trim whitespaces (induced by cleaning or otherwise)
            string = string.strip()                 # From beginning and end
            string = re.sub(r'[ ]+'," ",string)     # From the middle
            return(string)
        
    def extract_email_from_affiliations(affiliation):
        """Extract an author's email from their list of affiliations."""
        try:
            emailregex = r'[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+'
            return(re.findall(emailregex, affiliation)[0])
        except:
            return('')
        
    def read_stoplist(stoplistpath, sep = ", "):
        """
        Parse our list of stopwords.
        
        stoplistpath = a path to a .txt file with stopwords
        sep = Separator of words in list (separated by ", " by default)
        """
        with open(stoplistpath, 'r') as file:
            data = file.read().split(sep)
        return(data)
    
    print(filename + " - start")
    begin = datetime.now()
    print(filename + " - load data")
    df = medline_parser.parse_medline_xml(filepath)
    df = pd.DataFrame(df)
    print(filename + " - save to " + respath)
    
    # Get rid of unneeded columns -- can maybe work this into the parser later
    df = df.drop(['publication_types', 'chemical_list',
                  'keywords', 'doi', 'references',
                  'delete','pmc','other_id','medline_ta',
                  'nlm_unique_id','issn_linking','country'],
                 axis=1)
    
    # Replace missing values (mesh, affiliation, or title) with ''
    nacols = ['mesh_terms','affiliations','title']
    df[nacols] = df[nacols].fillna('')
    
    # Sometimes authors and affiliations not read in as list
    if type(df.loc[2,'authors']) is not list:
        df['authors'] = df['authors'].apply(lambda x:x.split(";"))
        df['affiliations'] = df['affiliations'].apply(lambda x:x.split(";"))
        
    # Explode Name and Affiliation cols. Key cols now author-article, not article.
    df = df.apply(pd.Series.explode)
    
    ## Extract emails
    df['email'] = [extract_email_from_affiliations(affiliation) 
                       for affiliation in df['affiliations']]
    
    # Clean strings
    # Take out stopwords, non-alphanumeric chars, and single-character words
    if rm_stopwords:
        if stop_paths is None:
            df['title'] =  [clean_string(title,read_stoplist(title_stop_path))
                                for title in df['title']]
            df['affiliations'] = [clean_string(affiliation,
                                               read_stoplist(affil_stop_path))
                                      for affiliation in df['affiliations']]
            df['mesh_terms'] =  [clean_string(mesh_term,
                                              read_stoplist(mesh_stop_path),
                                              mesh = True)
                                     for mesh_term in df['mesh_terms']]
        elif len(stop_paths) == 3:
            try:
                df['title'] =  [clean_string(title,read_stoplist(stop_paths[0]))
                                for title in df['title']]
                df['affiliations'] = [clean_string(affiliation,
                                               read_stoplist(stop_paths[1]))
                                      for affiliation in df['affiliations']]
                df['mesh_terms'] =  [clean_string(mesh_term,
                                              read_stoplist(stop_paths[2]),
                                              mesh = True)
                                     for mesh_term in df['mesh_terms']]
            except Exception:
                raise Exception("Error while trying to read stoplists not"
                                " defined in main() function.")
    else:
        df['title'] =  [clean_string(title,rm_stopwords = False) 
                            for title in df['title']]
        df['affiliations'] =  [clean_string(affiliation,rm_stopwords = False)
                                   for affiliation in df['affiliations']]
        df['mesh_terms'] =  [clean_string(mesh_term,rm_stopwords = False) 
                                 for mesh_term in df['mesh_terms']]
        
    # Save to a compressed csv in the results directory
    df.to_csv(respath, compression = "gzip", sep=",")
    end = datetime.now() - begin
    print(filename + " - done - " + str(np.round(end.seconds / 60)) + "min")
    
def start(text_data_dir, res_dir, nprocs=8):
    '''
    entry function

    text_data_dir: folder of raw data
    text_res_dir: folder of output
    verbose: int. Information is printed every N records
    nprocs: number of cores in parallel
    '''

    p = Pool(processes=nprocs)
    for dirpath, _, filenames in os.walk(text_data_dir):
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


def combine( res_dir, combined_dir, nprocs, keep_original_files = True):
    """Append all of the smaller zipped files together in a new "full" file."""
    
    def append_files(filetuple):
        """Worker function  appends two files together."""
        if len(filetuple) != 2:
            errmsg = ("append_files function expected 2 files to append,"
                        "instead received" + len(filetuple) + ".")
            raise Exception(errmsg)
        file1 = pd.read_csv(filetuple[1], compression = "gzip", sep = ",")
        
        # Append to other file
        file1.to_csv(filetuple[0], compression= "gzip", mode = "a", sep = ",")
        os.remove(filetuple[1])
        
    def mute():
        """Mute the output of Pool."""
        sys.stdout = open(os.devnull, 'w') 
        
    files = []
    procpool = Pool(nprocs, initializer=mute)
    # Populate a vector with filenames of data
    for dirpath, dirnames, filenames in os.walk(res_dir):
        for filename in filenames:
            print(filename)
            if "gz" in filename and 'md5' not in filename and 'copy' not in filename:
                filepath = os.path.join(dirpath, filename)
                print(filepath)
                files.append(filepath)

    # Send off files two at a time to cores to be appended together.
    # Put file names in a tuple and send them through with Pool.map
    filetuples = list()
    while len(files) > 1:
        # Make an iterator of the files variable
        iterfiles = iter(files)
        for file in iterfiles:
            # If our queue is full, send to processors
            if len(filetuples) == nprocs:
                procpool.map(append_files,filetuples, chunksize=1)
                filetuples = list()

            # If there is an odd file at the end of the list, empty queue
            # Otherwise it'd be possible for repeats in the queue on next loop
            if (file == files[-1]):
                procpool.map(append_files,filetuples, chunksize=1)
                filetuples = list()
                break
            else:
                # Otherwise we add a new pair of files to be appended together
                filetuples.append((file,next(iterfiles)))

    # Change file name of combined dataset to full_pubmed.csv.gz
    os.rename(files[0],(os.path.dirname(files[0]) + "full_pubmed.csv.gz"))


def main(download_data = False, combine_data =  False, nprocs=8):
    """
    text_data_dir: folder of raw data
    text_res_dir: folder of output
    verbose: int. Information is printed every N records
    nprocs: number of cores in parallel
    """

    if download_data:
        download_pubmed_data()

    # If our results directory doesn't exist, we create it
    for temp_dir in [res_dir]:
        if not os.path.exists(temp_dir):
            os.makedirs(temp_dir)
    
    # start reads, cleans, and saves the ~1050 xml files into csv.gz files
    start(text_data_dir=text_data_dir, res_dir=res_dir, nprocs=nprocs)
    
    # Combine appends all of the csvs into one, indexed by author-article
    if combine_data:
        combine(res_dir=res_dir, combined_dir=combined_dir, nprocs=8)

if __name__ == "__main__":
        main()

## Make Coauthor Column look like 'D.ROY-0/S.LIN-0/J.LAGARENNE-0' (OK but why -0)