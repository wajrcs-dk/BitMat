-1:27156:0
-1:8683:0
#####################################
PREFIX dbpprop: &lt;http://dbpedia.org/property/&gt;
SELECT * WHERE
{
    ?var dbpprop:subsid ?var3 .
    ?var dbpprop:divisions ?var4
}
