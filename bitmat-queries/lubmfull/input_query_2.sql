-1:17:80991094
-2:17:80991101
-3:17:80991091
-1:6:-3
-3:10:-2
-1:15:-2
#####################################
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
PREFIX ub: &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#&gt;
SELECT ?X, ?Y, ?Z
WHERE
{
    ?X rdf:type &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#GraduateStudent&gt; .
    ?Y rdf:type &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#University&gt; .
    ?Z rdf:type &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#Department&gt; .
    ?X ub:memberOf ?Z .
    ?Z ub:subOrganizationOf ?Y .
    ?X ub:undergraduateDegreeFrom ?Y
}