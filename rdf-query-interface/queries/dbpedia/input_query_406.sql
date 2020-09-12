PREFIX dct: <http://purl.org/dc/terms/>
PREFIX georss: <http://www.georss.org/georss/>
PREFIX skos: <http://www.w3.org/2004/02/skos/core#>
PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
PREFIX dbpprop: <http://dbpedia.org/property/>
PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
SELECT * WHERE
{
    ?v0 rdfs:comment ?v1 .
    ?v0 foaf:page ?v
}