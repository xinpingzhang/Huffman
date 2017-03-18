./huffc books/iliad.txt iliad.txt.he
./huffd iliad.txt.he iliad.txt
cmp --silent books/iliad.txt iliad.txt && echo '### SUCCESS: Files Are Identical! ###' || echo '### WARNING: Files Are Different! ###'

