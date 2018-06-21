mkdir bin
mkdir output
make clean
make
./bin/bitmat -l y -Q y -f ./config/my.conf -q ./config/my.query -o ./output/result.txt