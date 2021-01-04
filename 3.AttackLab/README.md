###  Table Of Content

- [Part I: Code Injection Attacks](#part-i-code-injection-attacks)
	- [1.1 Level 1](#11-level-1)
	- [1.2 Level 2](#12-level-2)
	- [1.3 Level 3](#13-level-3)
	
- [Part II: Return-Oriented Programming](#part-ii-return-oriented-programming)
	- [2.1 Level 2](#21-level-2)
	- [2.2 Level 3](#22-level-3)


# 	Attack Lab Report

In this lab, it used the vulnerability of function `Gets()` to do the correspond attacks, where it never check the input string length to trigger the stack overflow attack.

```c
1 unsigned getbuf() 
2 { 
3 	char buf[BUFFER_SIZE]; 
4 	Gets(buf); 
5 	return 1; 
6 }
```

Specifically, this lab uses two types of attack : `Code Injection` and `Return-Oriented Programming`

Here is the summary of lab phases: 

| Phase  | Program   | Level  |	Method	| Functions|
| :----: |:--------:| :----: | :------: | :-------:|
| 1      	| CTARGET |   1     |      CI       |  Touch1  |
| 2      	| CTARGET |   2     |      CI       |  Touch2  |
| 3      	| CTARGET |   3     |      CI       |  Touch3  |
| 4      	| RTARGET |   2     |     ROP     |  Touch2  |
| 5      	| RTARGET |   3     |     ROP     |  Touch3  |

**where CI = Code Injection; ROP = Return-Oriented Programming.**

##	Part I: Code Injection Attacks

### 1.1 Level 1


### 1.2 Level 2


### 1.3 Level 3


##	Part II: Return-Oriented Programming

###  2.1 Level 2

###  2.2 Level 3
