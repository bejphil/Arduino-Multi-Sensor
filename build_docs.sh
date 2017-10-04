#!/bin/bash

# Build Docs for Arduino file contained in base folder

shopt -s nullglob
for x in *.ino; do
	BASENAME=${s%.*}
	cp x BASENAME+".cpp"
done

doxygen ./docs/Doxyfile
