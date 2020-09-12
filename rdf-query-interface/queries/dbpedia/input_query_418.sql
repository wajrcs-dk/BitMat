PREFIX foaf: <http://xmlns.com/foaf/0.1/>
PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#>
SELECT * WHERE
{
    ?var6 <http://dbpedia.org/ontology/iataLocationIdentifier> ?var5 .
    ?var6 foaf:homepage ?var6_home .
    ?var6 <http://dbpedia.org/property/nativename> ?var6_name
}
