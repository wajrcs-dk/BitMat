PREFIX dbpowl: <http://dbpedia.org/ontology/>
PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
SELECT * WHERE
{
    ?var2 dbpowl:nationality ?var4 .
    ?var4 rdfs:label ?var5 . ?var2 rdfs:label ?var
}
