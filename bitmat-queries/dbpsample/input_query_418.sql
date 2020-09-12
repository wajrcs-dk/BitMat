-1:596:0
-1:31400:0
-1:18237:0
#####################################
PREFIX foaf: &lt;http://xmlns.com/foaf/0.1/&gt;
PREFIX rdfs: &lt;http://www.w3.org/2000/01/rdf-schema#&gt;
SELECT * WHERE
{
    ?var6 &lt;http://dbpedia.org/ontology/iataLocationIdentifier&gt; ?var5 .
    ?var6 foaf:homepage ?var6_home .
    ?var6 &lt;http://dbpedia.org/property/nativename&gt; ?var6_name
}
