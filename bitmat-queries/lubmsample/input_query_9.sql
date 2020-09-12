-1:17:1931137
-2:17:0
-3:17:1931133
-1:1:-2
-2:12:-3
-1:11:-3
#####################################
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
PREFIX ub: &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#&gt;
SELECT ?X, ?Y, ?Z
WHERE
{
    ?X rdf:type &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#GraduateStudent&gt; .
    ?Y rdf:type ?V .
    ?Z rdf:type &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#Course&gt; .
    ?X ub:advisor ?Y .
    ?Y ub:teacherOf ?Z .
    ?X ub:takesCourse ?Z
}