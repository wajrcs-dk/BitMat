PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
PREFIX dbpowl: <http://dbpedia.org/ontology/>
SELECT * WHERE
{
    ?v5 dbpowl:thumbnail ?v4 .
    ?v5 rdfs:label ?v .
    ?v5 foaf:page ?v8
}