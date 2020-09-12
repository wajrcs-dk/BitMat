-1:13:-2
-1:11:0
0:8:-1
-3:12:-2
-1:1:-3
-3:9:0
0:8:-3
#####################################
PREFIX ub: &lt;http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#&gt;
SELECT *
WHERE
{
    ?st ub:teachingAssistantOf ?course .
    ?st ub:takesCourse ?course2 . ?pub1 ub:publicationAuthor ?st .
    ?prof ub:teacherOf ?course . ?st ub:advisor ?prof . 
    ?prof ub:researchInterest ?resint . ?pub2 ub:publicationAuthor ?prof
}