#!/bin/bash
make all
gcc -c final_shell.c
gcc -c queue.c 
gcc -c keyval_table.c 
gcc queue.o final_shell.o keyval_table.o -o myshell

# Checking redirect
ls -la > file
ls >> file
ls –l nofile 2> logfile

# Checking echo
echo 123 456
echo $?
erfr
echo $?

# Checking cd
mkdir d
cd d
touch file
ls
pwd

# Checking !!
ls
!!

# Checking CTRL^C
CTRL^C

# Checking prompt
prompt = hi

# Checking pipeline
ls | grep my | grep ell | grep c > file

# Checking variables
$l = ron
$m = myshell
echo $l 
echo $m 
$l = aviya
echo $l

# Checking READ
read
ron
echo $REPLY

read var
aviya
echo $var


# Check if the input number is greater than 10
if [ $num -gt 10 ]; then
    echo "The number is greater than 10."
else
    echo "The number is not greater than 10."
fi

if [ true ]; then
    echo "Allways true"
fi

# pipe
cat > colors.txt
blue 
black
red
red
red
red
blue
Control-D
cat colors.txt
cat colors.txt | cat | cat | cat
sort colors.txt | uniq -c | sort -r | head -3

ls | grep myshell

# check quit
quit

# clean
make clean
rm -f *.o myshell *.so