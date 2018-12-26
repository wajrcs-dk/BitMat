-1:17:7342
-1:8:2
#####################################
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
PREFIX ub: &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#&gt;
SELECT ?X
WHERE
{
    ?X rdf:type &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#Publication&gt; .
    ?X ub:publicationAuthor &lt;http://www.Department0.University0.edu/AssistantProfessor0&gt;
}