PREFIX dbpprop: <http://dbpedia.org/property/>
PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
PREFIX dbpowl: <http://dbpedia.org/ontology/>
SELECT * WHERE
{
    ?v2 rdfs:label ?v .
    ?v6 dbpowl:city ?v2 .
    ?v6 dbpprop:iata ?v5
}