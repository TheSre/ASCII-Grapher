default: 
	gcc gsgc.c -lm -Wall -g -o gsgc.out
clean:
	rm *.out
test:
	(echo +,3,x; echo 3; echo 3) | ./gsgc.out
	(echo *,145,x; echo 5; echo 5) | ./gsgc.out
	(echo -,*,12,x,55; echo 5; echo 5) | ./gsgc.out
	(echo -,+,*,3,^,x,2,/,*,3,x,5,2; echo 25; echo 25) | ./gsgc.out


