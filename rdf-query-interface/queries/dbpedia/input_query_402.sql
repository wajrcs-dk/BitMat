PREFIX dbpprop: <http://dbpedia.org/property/>
PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX dbpowl: <http://dbpedia.org/ontology/>
SELECT * WHERE
{
    ?v3 dbpprop:position ?v6 .
    ?v3 dbpprop:clubs ?v8 .
    ?v8 dbpowl:capacity ?v1 .
    ?v3 dbpowl:birthPlace ?v5
}