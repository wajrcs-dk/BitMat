PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
PREFIX dbpowl: <http://dbpedia.org/ontology/>
SELECT * WHERE
{
    ?var5 dbpowl:thumbnail ?var4 .
    ?var5 rdfs:label ?var .
    ?var5 foaf:page ?var8
}
