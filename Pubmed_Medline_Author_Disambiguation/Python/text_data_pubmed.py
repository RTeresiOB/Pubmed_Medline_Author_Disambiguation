#!/usr/bin/env python3
# -*- coding: utf-8 -*-

'''
The preprocessing script for Pubmed/Medline Disambiguation.

@author: RobertKTeresi
'''

# Import libraries
import os
import io
import sys
from  subprocess import call
import re
from csv import reader, writer
from pathlib import Path
import shutil # Copy files to another directory
from multiprocessing import Pool
import numpy as np
import pandas as pd
from datetime import datetime
import gzip
from pubmed_parser import medline_parser
from pubmed_process import pubmed_processor

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
                   "/Stopword_lists/pubmed_small_stopwords_list.txt")
affil_stop_path = (working_directory +
                   "/Stopword_lists/pubmed_affiliation_stopwords.txt")
mesh_stop_path =  (working_directory +
                   "/Stopword_lists/pubmed_mesh_stopwords.txt")


#journals = list(pd.read_excel('/Users/RobertTeresi/Downloads/Race and General '
 #                             'Psych Journals.xlsx',
  #                            sheet_name='Journal_List').Journals.str.lower())

def download_pubmed_data():
    """Download zipped xml from the medline website."""
    from subprocess import call
    call("./pubmed_update")

def combine( res_dir, combined_dir, nprocs, keep_original_files=True, add_row_number=True):
    """Append all of the smaller zipped files together in a new "full" file."""
    
    # Pool can only use top-level functions. Make append_files a global var
    global append_files
    def append_files(filetuple):
        """Worker function  appends two files together."""
        if len(filetuple) != 2:
            errmsg = ("append_files function expected 2 files to append,"
                        "instead received" + len(filetuple) + ".")
            print(errmsg)
            raise Exception()
        print("111")
        file1 = pd.read_csv(filetuple[1], compression = "gzip",
                            sep = ",", keep_default_na=False)
        # Append to other file
        file1.to_csv(filetuple[0], compression= "gzip", mode = "a", sep = ",",
                     header=False, index=False)
        # Remove other file from hard drive
        os.remove(filetuple[1])
            
    def mute():
        """Mute the output of Pool."""
        sys.stdout = open(os.devnull, 'w') 
    
    def add_column_in_csv(input_file, output_file, transform_row):
        """
        Append a column in existing csv using csv.reader / csv.writer classes.
        
        Used to add row numbers (Unique_Record_ID) to full data file without
         reading entire dataset into memory.
         
        Function taken from:
         'https://thispointer.com/python-add-a-column-to-an-existing-csv-file/'
        
        Adapted to work with .gzip files using the answers found on:
         'https://stackoverflow.com/questions/9252812
        """
        # Open the input_file in read mode and output_file in write mode
        with gzip.open(input_file, 'rt') as read_obj, \
             gzip.open(output_file, 'wt') as write_obj:
            # Create a csv.reader object from the input file object
            csv_reader = reader(read_obj)
            # Create a csv.writer object from the output file object
            csv_writer = writer(write_obj)
            # Read each row of the input csv file as list
            for row in csv_reader:
                # Pass the list / row in the transform function to add column text for this row
                transform_row(row, csv_reader.line_num) # transform_row called by parent function
                # Write the updated row / list to the output file
                csv_writer.writerow(row)
        
    files = []
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
        if len(files) % 2:
            iterfiles = iter(files[:-1])
        else:
            iterfiles = iter(files)
        filetuples = [(x,next(iterfiles)) for x in iterfiles]

        del files[1::2] 
        
        procpool = Pool(nprocs)  #, initializer=mute)
        
        procpool.map(append_files,filetuples, chunksize=1)
        procpool.close()
        procpool.join()
        procpool = Pool(nprocs)  #, initializer=mute)

    # Change file name of combined dataset to full_pubmed.csv.gz
    full_data_dir =  os.path.dirname(files[0])
    os.rename(files[0],full_data_dir + "/full_pubmed_no_id.csv.gz")
    # Add Unique_Record_ID Column to full data
    if add_row_number:
        add_column_in_csv(full_data_dir + "/full_pubmed_no_id.csv.gz",
                          full_data_dir + "/full_pubmed.csv.gz",
                      lambda row, line_num: row.append("RecordID".encode("utf-8"))
                      if line_num == 1 
                      else row.append(line_num - 2))  # Start with 0

    # Now get rid of data without the id
    os.remove(full_data_dir + "/full_pubmed_no_id.csv.gz")

def main(download_data=False, combine_data=True, nprocs=12,
         affilbind_compiled=True, select_journals=False):
    """
    text_data_dir: folder of raw data
    text_res_dir: folder of output
    verbose: int. Information is printed every N records
    nprocs: number of cores in parallel
    """
    global processor
    if download_data:
        download_pubmed_data()

    # If our results directory doesn't exist, we create it
    for temp_dir in [res_dir]:
        if not os.path.exists(temp_dir):
            os.makedirs(temp_dir)
    for temp_dir in [combined_dir]:
        if not os.path.exists(temp_dir):
            os.makedirs(temp_dir)
    
    if not select_journals:
        journals=False

    processor = pubmed_processor(working_directory = working_directory,
                                 title_stop_path = title_stop_path,
                                 affil_stop_path = affil_stop_path,
                                 mesh_stop_path = mesh_stop_path)
    
    # start reads, cleans, and saves the ~1050 xml files into csv.gz files
    processor.start(text_data_dir=text_data_dir,
                    res_dir=res_dir,
                    nprocs=nprocs)
    
    # Combine appends all of the csvs into one, indexed by author-article
    if combine_data:
        combine(res_dir=res_dir, combined_dir=combined_dir, nprocs=8)

    # df.to_csv(filename, date_format='%Y%m%d')
if __name__ == "__main__":
       # main(select_journals=journals)
       main()
