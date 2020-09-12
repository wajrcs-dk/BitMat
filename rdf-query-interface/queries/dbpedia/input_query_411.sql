PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX dbpprop: <http://dbpedia.org/property/>
SELECT * WHERE
{
    ?var3 dbpprop:series ?var8 .
    ?var3 foaf:name ?var4 .
    ?var3 rdfs:comment ?var5
}
