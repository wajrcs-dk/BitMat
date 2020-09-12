PREFIX ub: <http://www.lehigh.edu/~zhp2/2004/0401/univ-bench.owl#>
SELECT *
WHERE
{
    ?st ub:teachingAssistantOf ?course .
    ?st ub:takesCourse ?course2 . ?pub1 ub:publicationAuthor ?st .
    ?prof ub:teacherOf ?course . ?st ub:advisor ?prof . 
    ?prof ub:researchInterest ?resint . ?pub2 ub:publicationAuthor ?prof
}