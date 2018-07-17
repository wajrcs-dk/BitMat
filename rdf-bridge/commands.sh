mkdir ../bitmat-data-store/
rm ../bitmat-data-store/*
rm ../log/*

cat ../data-store/bitmat-sample-1 ../data-store/bitmat-sample-2 > ../data-store/full-bitmat-sample
sed -i 's/://g' ../data-store/full-bitmat-sample
sed -i 's/ /:/g' ../data-store/full-bitmat-sample

sort -u ../data-store/full-bitmat-sample | awk -F: '{print $1}' > ../data-store/full-bitmat-sample-sub
sed -i 's/\/\//:\/\//g' ../data-store/full-bitmat-sample-sub

sort -u ../data-store/full-bitmat-sample | awk -F: '{print $2}' > ../data-store/full-bitmat-sample-pre
sed -i 's/\/\//:\/\//g' ../data-store/full-bitmat-sample-pre

sort -u ../data-store/full-bitmat-sample | awk -F: '{print $3}' > ../data-store/full-bitmat-sample-obj
sed -i 's/\/\//:\/\//g' ../data-store/full-bitmat-sample-obj

sed -i 's/:/ /g' ../data-store/full-bitmat-sample
sed -i 's/\/\//:\/\//g' ../data-store/full-bitmat-sample

sort ../data-store/full-bitmat-sample-sub | uniq > ../data-store/full-bitmat-sample-sub-all
rm ../data-store/full-bitmat-sample-sub

sort ../data-store/full-bitmat-sample-pre | uniq > ../data-store/full-bitmat-sample-pre-all
rm ../data-store/full-bitmat-sample-pre

sort ../data-store/full-bitmat-sample-obj | uniq > ../data-store/full-bitmat-sample-obj-all
rm ../data-store/full-bitmat-sample-obj

sort ../data-store/full-bitmat-sample-sub-all ../data-store/full-bitmat-sample-obj-all | uniq -d > ../data-store/full-bitmat-sample-common

wc -l < ../data-store/full-bitmat-sample-common > ../data-store/full-bitmat-sample-common-count

sort ../data-store/full-bitmat-sample-sub-all ../data-store/full-bitmat-sample-common | uniq -u > ../data-store/full-bitmat-sample-sub-left

sort ../data-store/full-bitmat-sample-obj-all ../data-store/full-bitmat-sample-common | uniq -u > ../data-store/full-bitmat-sample-obj-left

cat ../data-store/full-bitmat-sample-common ../data-store/full-bitmat-sample-sub-left > ../data-store/full-bitmat-sample-sub-all

cat ../data-store/full-bitmat-sample-common ../data-store/full-bitmat-sample-obj-left > ../data-store/full-bitmat-sample-obj-all

rm ../data-store/full-bitmat-sample-sub-left

rm ../data-store/full-bitmat-sample-obj-left

python rdf-bridge/bootstrap.py "data-store/rdf-sample-1 data-store/rdf-sample-2" /usr/local/go/bin/go config/rdf-sample.conf all 100 rdf-sample

python rdf-bridge/bootstrap.py /raid0/datasets/lubm/lubmsample /home/alamgir/go/bin/go config/rdf-sample.conf all 25000 rdfs-1-7