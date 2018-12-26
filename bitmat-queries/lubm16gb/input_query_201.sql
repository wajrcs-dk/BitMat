-1:13:-2
-3:12:-2
-1:1:-3
#####################################
PREFIX rdf: &lt;http://www.w3.org/1999/02/22-rdf-syntax-ns#&gt;
PREFIX ub: &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#&gt;
SELECT ?X
WHERE
{
    ?X ub:teachingAssistantOf ?Y .
    ?Z ub:teacherOf ?Y .
    ?X ub:advisor ?Z
}