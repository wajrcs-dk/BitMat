-1:21:0
-1:31383:0
-1:31384:0
#####################################
PREFIX rdfs: &lt;http://www.w3.org/2000/01/rdf-schema#&gt;
PREFIX foaf: &lt;http://xmlns.com/foaf/0.1/&gt;
PREFIX geo: &lt;http://www.w3.org/2003/01/geo/wgs84_pos#&gt;
SELECT * WHERE
{
    ?var6 &lt;http://dbpedia.org/ontology/abstract&gt; ?var1 .
    ?var6 geo:lat ?var3 .
    ?var6 geo:long ?var4
}
