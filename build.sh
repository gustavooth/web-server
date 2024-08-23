echo "Compiling window..."

#Create build/ folder if not exist.
buildf="build"
if [ ! -d "$buildf" ]; then
    mkdir "$buildf"
fi

#Create bin/ folder if not exist.
if [ ! -d "bin" ]; then
    mkdir "bin"
fi

cd $buildf
cmake -G Ninja ..
ninja