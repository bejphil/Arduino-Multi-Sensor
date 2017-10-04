#!/bin/bash

# Build Docs for Arduino file contained in base folder

# Make a copy of all *.ino files and rename to *.cpp files
shopt -s nullglob
for x in *.ino; do
	BASENAME=${x%.*}
	cp x BASENAME+".cpp"
done

cd ./doxygen

# Run doxygen, making a local copy of docs
# Then copy html output to ./docs folder to populate GitHub pages
doxygen Doxyfile
rm -r ../docs/*
cp -r ./html/* ../docs

cd ..

# Remove all copies of the *.ino files
shopt -s nullglob
for x in *.cpp; do
	rm $x
done
