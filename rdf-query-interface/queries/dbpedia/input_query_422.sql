PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX dct: <http://purl.org/dc/terms/>
SELECT * WHERE
{
    ?var4 dct:subject ?var .
    ?var4 foaf:name ?var6
}
