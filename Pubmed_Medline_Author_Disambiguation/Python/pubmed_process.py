#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Tue Aug 18 18:19:09 2020

@author: RobertTeresi
"""

import os
import io
import sys
import re
from subprocess import call
from functools import partial
from csv import reader, writer
from pathlib import Path
import shutil # Copy files to another directory
#from multiprocessing import Pool
from pathos.multiprocessing import ProcessingPool as PathosPool # Pool in class
import numpy as np
import pandas as pd
from datetime import datetime
import gzip
## TO DO: Get Suffix Field out of  .xml
from pubmed_parser import medline_parser

class pubmed_processor:
    
    def __init__(self, working_directory, title_stop_path,
                 affil_stop_path, mesh_stop_path, rm_stopwords=True,
                 affiliation_correction = True,
                 select_journals=False):
        
        # Initialize all basic variables to run functions
        self.working_directory = working_directory
        
        #("/Users/RobertTeresi/Documents/GitHub/"
                   # "Pubmed_Medline_Author_Disambiguation-orig/"
                    #"Pubmed_Medline_Author_Disambiguation")
        self.rm_stopwords = rm_stopwords
        (self.affil_stop_path,
         self.mesh_stop_path,
         self.title_stop_path) = (affil_stop_path,mesh_stop_path,
                                  title_stop_path)
        self.select_journals = select_journals
        # Compile and import C++ function
        if os.getcwd()[-3:] != "C++":
            os.chdir("C++")
            call(["chmod", "755", "affilbind_compile.sh"]) # Make executable
            call("./affilbind_compile.sh") # Execute compilation

    def process_data(self, filepath, filename, respath, select_journals=None,
                     rm_stopwords=True, stop_paths=None,
                     affiliation_correction=True, abstract=False,
                     remove_commas=False):
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
                # Remove numerical MeSH Codes
                string = [ re.sub(r'([ ]?D\d{6}\:)','',x) for x in string]
                string = [x.lower() for x in string]
                if rm_stopwords:
                    
                    # Remove stop words
                    string = [x for x in string if x not in stopword_list]
    
                # Convert to '/' separated string
                string = '/'.join(string)
                
                string = re.sub(r',','',string)
                #string = re.sub(r'([ ]?d\d{6}\:' + r'|[ ]?d\d{6}\:'.join(stopword_list) + r')', '', string)
                # Now I only want to keep the unique ID
                #string = re.sub(r'[ ]?(d\d{6})(\:[a-z\-\'\.\, ]+)', r'\1', string)
                return(string)
            else:  
                if type(string) is list:
                    string = string = ('/'.join(string)).lower()
                # First, convert the string to lower
                string = string.lower()
                string = re.sub(r'\W+|\b[a-z]\b', ' ', string)
                    
                # Remove stopwords if the option is set to true
                if rm_stopwords:  
                        
                    string = re.sub(r'\b(' + r'| '.join(stopword_list) +
                                    r')\b\s*', ' ', string)
                    
                # Trim whitespaces (induced by cleaning or otherwise)
                string = string.strip()                 # From beginning and end
                string = re.sub(r'[ ]+'," ",string)     # From the middle
                return(string.split('/'))
                    
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
        
        def copy_remove(author, coauthors):
            """Remove author from list of coauthors."""
            lcopy = list(coauthors)
            lcopy.remove(author)
            return(lcopy)
        
        def add_to_affil_dict(lname, affilwordvec, d):
            for affilword in affilwordvec:
                if affilword == '':
                    continue
                elif d[lname][1].get(affilword):
                    d[lname][1][affilword] += 1
                else:
                    d[lname][1][affilword] = 1    
            d[lname][0] += 1
        
        def explode(df, lst_cols, fill_value=''):
            # make sure `lst_cols` is a list
            if lst_cols and not isinstance(lst_cols, list):
                lst_cols = [lst_cols]
            # all columns except `lst_cols`
            idx_cols = df.columns.difference(lst_cols)
        
            # calculate lengths of lists
            lens = df[lst_cols[0]].str.len()
        
            if (lens > 0).all():
                # ALL lists in cells aren't empty
                return pd.DataFrame({
                    col:np.repeat(df[col].values, df[lst_cols[0]].str.len())
                    for col in idx_cols
                }).assign(**{col:np.concatenate(df[col].values) for col in lst_cols}) \
                  .loc[:, df.columns]
            else:
                # at least one list in cells is empty
                return pd.DataFrame({
                    col:np.repeat(df[col].values, df[lst_cols[0]].str.len())
                    for col in idx_cols
                }).assign(**{col:np.concatenate(df[col].values) for col in lst_cols}) \
                  .append(df.loc[lens==0, idx_cols]).fillna(fill_value) \
                  .loc[:, df.columns]
        
        print(filename + " - start")
        begin = datetime.now()
        print(filename + " - load data")
        df = medline_parser.parse_medline_xml(filepath, reference_list=True)
        df = pd.DataFrame(df)
        print(filename + " - save to " + respath)
        
        # Get rid of unneeded columns -- can maybe work this into the parser later
        '''
        df = df.drop(['publication_types', 'chemical_list',
                      'keywords', 'doi', 'references',
                      'delete','pmc','other_id','medline_ta',
                      'nlm_unique_id','issn_linking'],
                     axis=1)
        '''
        df = df.drop(['publication_types', 'chemical_list', 'doi',
                      'delete','pmc','other_id','medline_ta',
                      'nlm_unique_id','issn_linking'],
                     axis=1)
        
        # If we have a list of journals we want to use, filter data
        if select_journals:
            # Raise an error if list of journals is not a list
            if type(select_journals) is not list:
                raise TypeError("select_journals must be either None or a list.")
            else:
                # For now assume that we are doing a year filter
                df = df.loc[(df['pubdate'].astype(int) < 2019) &
                            (df['pubdate'].astype(int) > 1973)]
                df = df[df['journal'].str.lower().isin(select_journals)]  # Filter data
            
            # If after filtering, the dataframe is empty, exit the function
            if df.empty:
                print(filename + " - Finished: No data")
                return(None)
            
        # Sometimes authors and affiliations not read in as list
        if type(df['authors'].iloc[0]) is not list:
            df['authors'] = df['authors'].apply(lambda x:x.split(";"))
            df['affiliations'] = df['affiliations'].apply(lambda x:x.split("&&"))
        
        # Now create coauthors column (Same as authors column for now)
        df['coauthors'] = df['authors']
    
        # Explode Name and Affiliation cols.
        # Key cols now author-article, not article.
        df = explode(df,['authors','affiliations'])

        # Rename authors to author (the author in this observation)
        df.rename(columns={'authors': 'author'}, inplace=True)
        
        # Now remove author from coauthor list.
        # Careful not to remove other authors with same name. Use list.remove()
        #
        # Explode apparently makes variables in slots that used to be in the same
        # row references of the same object. 
        # (i.e. the list coauthors for every row in an article are references to 
        # the same variable)
        #
        # Therefore we have to make a copy of each list and then remove the author
        # from the row of coauthors.
        #
        # Finally, convert the list to a '/' joined string
        df['coauthors'] = ['/'.join(copy_remove(author, coauthors))
                              for author, coauthors 
                              in zip(df['author'],df['coauthors'])]
        
        
        # Extract emails
        df['email'] = [extract_email_from_affiliations(affiliation) 
                           for affiliation in df['affiliations']]
        
        # Replace missing values (mesh, affiliation, or title) with ''
        nacols = ['mesh_terms','affiliations','title', 'email']
        df[nacols] = df[nacols].fillna('')
        
        # Split into firstname lastname
        df = df[df['author'] != ''] # Some entries have no author!  (Can't use)
        df = df.reset_index() # reset index
        df = df.drop('index', axis = 1) # get rid of old index column
        splitnames = df.author.str.split(expand=False)
        df['Lastname'] = [x[-1] if len(x) >= 1 else '' for x in splitnames]
        df['Firstname'] = [' '.join(x[:-1]) if len(x) >= 2 else ''
                           for x in splitnames]
        df = df[df['Lastname'] != '']
        del splitnames
        
        # Clean strings
        # Take out stopwords, non-alphanumeric chars, and single-character words
        if rm_stopwords:
            if stop_paths is None:
                df['title'] = [clean_string(title,
                                            read_stoplist(self.title_stop_path))
                               if title != '' else ''
                               for title in df['title']]
                df['affiliations'] = [clean_string(affiliation,
                                                   read_stoplist(self.affil_stop_path))
                                          if affiliation != '' else ''
                                          for affiliation in df['affiliations']]
                df['mesh_terms'] =  [clean_string(mesh_term,
                                                  read_stoplist(self.mesh_stop_path),
                                                  mesh = True)
                                         if mesh_term != '' else ''
                                         for mesh_term in df['mesh_terms']]
            elif len(stop_paths) == 3:
                try:
                    df['title'] =  [clean_string(title,
                                                 read_stoplist(stop_paths[0]))
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

        # Now separate author name into first, middle, and last
        '''
        if affiliation_correction:
            # First make dictionary structure
            # (dict<Lastnamem,pair<int, dict<Affil,count>>>)
            affildict = {lname : [0, dict()] for lname in df['Lastname']}

            # Dict is mutable + passed by reference
            # updates without having to return
            list(map(partial(add_to_affil_dict, d=affildict),
                df['Lastname'], df['affiliations']))
        '''
        # Affiliations and MeSH back into format they will be passed to diambig in
        df['affiliations'] = ['/'.join(affil) for affil in df['affiliations']]
        # Change column names to be consistent with C++ script
        
        # Middlename column will be same as first name
        df['Middlename'] = df['Firstname']
        
        # Affiliation Exists column
        df['Affiliation_Exists'] = df['affiliations']\
                    .apply(lambda x: 0 if x == '' else 1)
        
        df.rename(columns={'title':'Title',
                          'email':'Email',
                          'language':'Language',
                          'affiliations':'Affiliations',
                          'country':'Country',
                          'journal':'Journal',
                          'pmid':'PMID',
                          'mesh_terms':'MeSH',
                          'coauthors':'Coauthor',
                          'abstract': 'Abstract'}, 
                 inplace=True)
        
        # Drop author column
        df = df.drop('author', axis  = 1)
        
        # Change title column from a list to a string
        df['Title'] = [' '.join(x) for x in  df['Title']]
        df['MeSH'] = [' '.join(x) for x in  df['MeSH']]
        df['Affiliation_Exists'] = df['Affiliation_Exists'].astype(str)
        # Remove all commas from dataframe (messes up read in C++)
        if remove_commas:
            df = df.applymap(lambda x: re.sub(r',','',x))
        
        df['Affiliation_Exists'] = df['Affiliation_Exists'].astype(int)
        '''
        df = df[['PMID', 'Lastname','Firstname','Middlename', 'Title',
                 'Journal', 'pubdate','MeSH','Language',
                 'Affiliations', 'Affiliation_Exists', 'Coauthor','Email']]
        '''
        # Save to a compressed csv in the results directory
        df.to_csv(respath, compression = "gzip", sep=",", index = False)
        end = datetime.now() - begin
        print(filename + " - done - " + str(np.round(end.seconds / 60)) + "min")
        
        # If we are correcting affiliations, return the dictionary.
        # The pool function calling it will return with a list of all dictionaries.
        #return(affildict)
        
    def start(self, text_data_dir, res_dir, nprocs=8):
        '''
        entry function
    
        text_data_dir: folder of raw data
        text_res_dir: folder of output
        verbose: int. Information is printed every N records
        nprocs: number of cores in parallel
        '''
        p = PathosPool(nprocs)

        filepathsvec,filenamesvec, respaths = list(),  list(), list()
        for dirpath, _, filenames in os.walk(text_data_dir):
            for filename in filenames:
                if (("gz" in filename) and ('md5' not in filename )
                        and ('copy' not in filename)):
                    filepath = os.path.join(dirpath, filename)
                    print(filepath)
                    res_name = filename.split(".")[0] + ".csv.gz"
                    respath = os.path.join(res_dir, res_name)
                    #if os.path.exists(respath):
                       # pass
                    #else:
                    if True:
                        filepathsvec.append(filepath)
                        filenamesvec.append(filename)
                        respaths.append(respath)
                        #p.apply_async(process_data, args = (filepath,filename,
                                                           # respath, True,
                                                           # [title_stop_path,
                                                           #  affil_stop_path,
                                                           #  mesh_stop_path]))
        self.affildicts = p.amap(partial(self.process_data,
                                    stop_paths = [self.title_stop_path,
                                                  self.affil_stop_path,
                                                  self.mesh_stop_path],
                                    rm_stopwords=True,
                                    affiliation_correction = True,
                                    select_journals = self.select_journals),
                            filepathsvec, filenamesvec, respaths)
        
        p.close()
        p.join() # Having an issue joining
        print("joined")
        p.clear() # Delete the pool
        # Now send the affildicts away to C++
        #print("Sending affiliations to C++ function")
        #import affilbind
        #affilbind.affil_stopword_find(self.affildicts.get(),
         #                             (self.working_directory +
          #                            "/Results/affiliation_stopword_list.txt")
           #                           )
        #print("Complete")
