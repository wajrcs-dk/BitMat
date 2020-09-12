-1:31373:0
-2:242:-1
-2:13532:0
#####################################
PREFIX dbpprop: &lt;http://dbpedia.org/property/&gt;
PREFIX foaf: &lt;http://xmlns.com/foaf/0.1/&gt;
PREFIX rdfs: &lt;http://www.w3.org/2000/01/rdf-schema#&gt;
PREFIX dbpowl: &lt;http://dbpedia.org/ontology/&gt;
SELECT * WHERE
{
    ?v2 rdfs:label ?v .
    ?v6 dbpowl:city ?v2 .
    ?v6 dbpprop:iata ?v5
}