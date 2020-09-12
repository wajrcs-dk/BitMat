PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX geo: <http://www.w3.org/2003/01/geo/wgs84_pos#>
SELECT * WHERE
{
    ?var6 rdfs:label ?var .
    ?var6 foaf:depiction ?var8 .
    ?var6 foaf:homepage ?var10
}
