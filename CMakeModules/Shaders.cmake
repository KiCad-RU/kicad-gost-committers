# CMake script file to process a GLSL source file, so it can be included
# in C array and compiled in to an application.

# number of input files
list( LENGTH inputFiles shadersNumber ) 

# write header
file( WRITE ${outputFile} "// Do not edit this file, it is autogenerated by CMake.

#ifndef SHADER_SRC_H
#define SHADER_SRC_H

const unsigned int shaders_number = ${shadersNumber};
const char *shaders_src[] = {\n" )

foreach( inputFile ${inputFiles} )
	# put the input file name into the output file
	file( APPEND ${outputFile} "\n// ${inputFile}" )
	
	# process the input file
	file( READ ${inputFile} contents )
	
	# remove /* */ comments
	string( REGEX REPLACE "/\\*.*\\*/" "" contents "${contents}" )
	# remove // comments
	string( REGEX REPLACE "//[^\n]*" "" contents "${contents}" )
	# remove whitespaces at the beginning of each line
	string( REGEX REPLACE "\n([\t ])*" "\n" contents "${contents}" )
	# remove unnecessary spaces
	string( REGEX REPLACE " *([\\*/+&\\|,=<>\(\)]) *" "\\1" contents "${contents}" )
	# remove empty lines & wrap every line in "" and add '\n' at the end of each line	
	string( REGEX REPLACE "\n+" "\\\\n\"\n\"" contents "${contents}" )
	# remove unnecessary " & \n from the beginning and the end of contents
	string( REGEX REPLACE "^\\\\n\"" "" contents "${contents}" )
	string( REGEX REPLACE "\"$" "," contents "${contents}" )

	file( APPEND ${outputFile} "${contents}" )

endforeach( inputFile ${inputFiles} )

# write footer
file( APPEND ${outputFile} "};
#endif /* SHADER_SRC_H */" )

