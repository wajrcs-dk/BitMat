-1:17:1931137
-2:17:1931134
-1:6:-2
-2:10:758399
-1:3:0
#####################################
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
PREFIX ub: &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#&gt;
SELECT ?X, ?Y, ?Z
WHERE
{
    ?X rdf:type &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#GraduateStudent&gt; .
    ?Y rdf:type &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#Department&gt; .
    ?X ub:memberOf ?Y .
    ?Y ub:subOrganizationOf &lt;http://www.University0.edu&gt; .
    ?X ub:emailAddress ?Z
}