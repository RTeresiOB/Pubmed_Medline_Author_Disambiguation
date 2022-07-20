#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Mon Oct 19 14:02:59 2020

@author: RobertTeresi
"""

import pandas as pd

matches = pd.read_csv("/Users/RobertTeresi/Documents/GitHub/Pubmed_Medline_Author_Disambiguation-orig/Pubmed_Medline_Author_Disambiguation/final.txt",
                      sep="###",header=None, names=["group","idk","record"])

# Clean list of record ids by row
def parse_cluster(clusterstring):
        clusterstring = clusterstring[:-1] # All rows have extra comma
        clusterstring = clusterstring.split(",")
        return(list(map(int, clusterstring)))


matches["record"] = matches["record"].apply(lambda x: parse_cluster(x))

matches = matches.explode("record")
# Read in our original data
data = pd.read_csv("/Volumes/LaCie/PubmedData_Update_2019/Full_Pubmed/tenth_stratified_sample.csv")

data = data.merge(matches, left_on='RecordID', right_on='record',
          suffixes=('_left', '_right'))

data = data.sort_values('group')

data_head = data.loc[:100000]

data_head['fullname'] = data_head['Firstname'] + data_head['Lastname']

data_head.groupby('fullname')
data_head['n_groups'] = data_head.groupby('fullname')['group'].nunique()
more_than_one_group = data_head.loc[data_head['n_groups'] > 1]

group_counts = data_head.groupby('fullname')['group'].nunique()
len(group_counts)
sum(group_counts)

data_head = data_head.loc[:10000]
