Copy all the files in this folder to the folder you have your code in ( Most likely "p4-xattr" ) 

run the command: make test

It will show sections with cmd3, cmd4,cmd5

Each of those sections first check your set function using cmdi.test. 

In this test , we use our correctly working code to get the values you had set and check whether your set operations have completed properly.
e.g.
[CORRECT(POINTS:+2):] 	 ATTR NAME: testing 	 VALUE: basictestvalue

CORRECT/INCORRECT tells you what points you scored for this particular ATTR
ATTR NAME is the attribute name for this test
VALUE is the value stored for your attribute name 


Then, we see whether your get operations work properly.

In cmd4 and cmd5: we also check whether you have correctly allocated the errors due to the flags

Towards the end, we also check whether the space occupied by your xattr is same as our xattr
