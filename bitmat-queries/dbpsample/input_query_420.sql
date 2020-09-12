-1:8683:0
-1:27156:0
#####################################
PREFIX dbpprop: &lt;http://dbpedia.org/property/&gt;
SELECT * WHERE
{
    ?var dbpprop:divisions ?var4 .
    ?var dbpprop:subsid ?var3
}
