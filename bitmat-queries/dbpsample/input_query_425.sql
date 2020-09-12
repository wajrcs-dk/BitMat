-1:31373:0
-1:31396:0
-1:31400:0
#####################################
PREFIX rdfs: &lt;http://www.w3.org/2000/01/rdf-schema#&gt;
PREFIX foaf: &lt;http://xmlns.com/foaf/0.1/&gt;
PREFIX geo: &lt;http://www.w3.org/2003/01/geo/wgs84_pos#&gt;
SELECT * WHERE
{
    ?var6 rdfs:label ?var .
    ?var6 foaf:depiction ?var8 .
    ?var6 foaf:homepage ?var10
}
