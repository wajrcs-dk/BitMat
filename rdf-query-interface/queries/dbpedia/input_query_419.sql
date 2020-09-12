PREFIX dbpprop: <http://dbpedia.org/property/>
SELECT * WHERE
{
    ?var dbpprop:subsid ?var3 .
    ?var dbpprop:divisions ?var4
}
