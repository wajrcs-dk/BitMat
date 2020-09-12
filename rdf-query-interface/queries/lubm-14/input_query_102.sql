PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#>
SELECT ?X, ?Y, ?Z
WHERE
{
    ?X rdf:type <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#GraduateStudent> .
    ?Y rdf:type <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#University> .
    ?Z rdf:type <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#Department> .
    ?X ub:memberOf ?Z .
    ?Z ub:subOrganizationOf ?Y .
    ?X ub:undergraduateDegreeFrom ?Y
}