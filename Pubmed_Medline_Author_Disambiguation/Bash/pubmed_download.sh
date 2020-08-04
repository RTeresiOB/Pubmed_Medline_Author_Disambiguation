# Install wget if not already installed
brew install wget

WORKING_DIRECTORY="/Users/RobertTeresi/Documents/Github"

wget -r --no-parent ftp://ftp.ncbi.nlm.nih.gov/pubmed/baseline/ -P $WORKING_DIRECTORY

# Downloads unnecessarily nested folders. Move it to where we want it
mv $WORKING_DIRECTORY/ftp.ncbi.nlm.nih.gov/pubmed/baseline $WORKING_DIRECTORY/
