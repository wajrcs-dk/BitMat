PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX geo: <http://www.w3.org/2003/01/geo/wgs84_pos#>
SELECT * WHERE
{
    ?var6 <http://dbpedia.org/ontology/abstract> ?var1 .
    ?var6 geo:lat ?var3 .
    ?var6 geo:long ?var4
}
