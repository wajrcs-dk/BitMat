-1:21514:0
-1:6133:-2
-2:191:0
-1:142:0
#####################################
PREFIX dbpprop: &lt;http://dbpedia.org/property/&gt;
PREFIX foaf: &lt;http://xmlns.com/foaf/0.1/&gt;
PREFIX dbpowl: &lt;http://dbpedia.org/ontology/&gt;
SELECT * WHERE
{
    ?v3 dbpprop:position ?v6 .
    ?v3 dbpprop:clubs ?v8 .
    ?v8 dbpowl:capacity ?v1 .
    ?v3 dbpowl:birthPlace ?v5
}