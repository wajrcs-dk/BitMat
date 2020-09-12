-1:18046:0
-1:20182:0
-1:3504:0
#####################################
PREFIX dbpprop: &lt;http://dbpedia.org/property/&gt;
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
SELECT * WHERE
{
    ?var6 dbpprop:name ?var0 .
    ?var6 dbpprop:pages ?var1 .
    ?var6 dbpprop:author ?var3
}
