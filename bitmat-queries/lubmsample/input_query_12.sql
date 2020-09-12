-1:17:0
-2:17:1931134
-1:16:-2
-2:10:758399
#####################################
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
PREFIX ub: &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#&gt;
SELECT ?X, ?Y
WHERE
{
    ?X rdf:type ?Z .
    ?Y rdf:type &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#Department&gt; .
    ?X ub:worksFor ?Y .
    ?Y ub:subOrganizationOf &lt;http://www.University0.edu&gt;
}