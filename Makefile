
# machine.c: machine.org
#	emacs --batch --file machine.org --eval "(org-babel-tangle)"

machine: machine.c
	gcc -o machine machine.c

machine.html: machine.org
	emacs --batch --visit=machine.org --funcall org-export-as-html-batch
