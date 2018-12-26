-1:17:7338
-1:16:1
-1:7:0
-1:3:0
-1:14:0
#####################################
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
PREFIX ub: &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#&gt;
SELECT ?X, ?Y1, ?Y2, ?Y3
WHERE
{
    ?X rdf:type &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#FullProfessor&gt; .
    ?X ub:worksFor &lt;http://www.Department0.University0.edu&gt; .
    ?X ub:name ?Y1 .
    ?X ub:emailAddress ?Y2 .
    ?X ub:telephone ?Y3
}