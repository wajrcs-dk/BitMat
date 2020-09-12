PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
SELECT * WHERE
{
    ?var2 rdfs:label ?var .
    ?var6 <http://dbpedia.org/ontology/city> ?var2
}
