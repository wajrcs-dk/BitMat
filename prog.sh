mkdir bin
mkdir output
make clean
make
./bin/bitmat -l y -Q y -f ./config/my.conf -q ./config/my.query -o ./output/result.txt
./bin/bitmat -l n -Q y -f config/lubm15gb.conf -p 2 -v 0 -q config/input_query_1.sql -o output/rdf-query-interface.txt