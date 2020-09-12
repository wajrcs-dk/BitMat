PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX dbpprop: <http://dbpedia.org/property/>
PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
SELECT * WHERE
{
    ?var3 dbpprop:numEmployees ?var .
    ?var3 foaf:homepage ?var7
}
