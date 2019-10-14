REM Make the directory batch file resides in as the working directory.
pushd %~dp0

glslangvalidator -V basic.vert -o basic.vert.spv
glslangvalidator -V basic.frag -o basic.frag.spv

popd