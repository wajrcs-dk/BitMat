PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#>
SELECT ?X, ?Y, ?Z
WHERE
{
    ?X rdf:type <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#Student> .
    ?Y rdf:type <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#Faculty> .
    ?Z rdf:type <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#Course> .
    ?X ub:advisor ?Y .
    ?Y ub:teacherOf ?Z .
    ?X ub:takesCourse ?Z
}