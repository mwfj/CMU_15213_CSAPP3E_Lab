# Proxy Lab Report



## Lab Test Result

### Part1 Only (40/40)

```shell
➜  ~/cmu-15-213-CSAPP3E-lab/7.Proxy_lab cd part1-only      
➜  ~/cmu-15-213-CSAPP3E-lab/7.Proxy_lab/part1-only make clean
rm -f *~ *.o proxy core *.tar *.zip *.gzip *.bzip *.gz
➜  ~/cmu-15-213-CSAPP3E-lab/7.Proxy_lab/part1-only make
gcc -g -Wall -c proxy.c
gcc -g -Wall -c csapp.c
gcc -g -Wall csapp.o proxy.o -o proxy -lpthread
➜  ~/cmu-15-213-CSAPP3E-lab/7.Proxy_lab/part1-only ./driver.sh
*** Basic ***
Starting tiny on 6550
Starting proxy on 18704
1: home.html
   Fetching ./tiny/home.html into ./.proxy using the proxy
   Fetching ./tiny/home.html into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
2: csapp.c
   Fetching ./tiny/csapp.c into ./.proxy using the proxy
   Fetching ./tiny/csapp.c into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
3: tiny.c
   Fetching ./tiny/tiny.c into ./.proxy using the proxy
   Fetching ./tiny/tiny.c into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
4: godzilla.jpg
   Fetching ./tiny/godzilla.jpg into ./.proxy using the proxy
   Fetching ./tiny/godzilla.jpg into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
5: tiny
   Fetching ./tiny/tiny into ./.proxy using the proxy
   Fetching ./tiny/tiny into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
Killing tiny and proxy
basicScore: 40/40
```



### Part1 and Part2 (55/55)

```shell
➜  ~/cmu-15-213-CSAPP3E-lab/7.Proxy_lab/part1_and_part2 make clean
rm -f *~ *.o proxy core *.tar *.zip *.gzip *.bzip *.gz
➜  ~/cmu-15-213-CSAPP3E-lab/7.Proxy_lab/part1_and_part2 make
gcc -g -Wall -c sbuf.c
gcc -g -Wall -c proxy.c
gcc -g -Wall -c csapp.c
gcc -g -Wall sbuf.o csapp.o proxy.o -o proxy -lpthread
➜  ~/cmu-15-213-CSAPP3E-lab/7.Proxy_lab/part1_and_part2 ./driver.sh
*** Basic ***
Starting tiny on 4600
Starting proxy on 28346
1: home.html
   Fetching ./tiny/home.html into ./.proxy using the proxy
   Fetching ./tiny/home.html into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
2: csapp.c
   Fetching ./tiny/csapp.c into ./.proxy using the proxy
   Fetching ./tiny/csapp.c into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
3: tiny.c
   Fetching ./tiny/tiny.c into ./.proxy using the proxy
   Fetching ./tiny/tiny.c into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
4: godzilla.jpg
   Fetching ./tiny/godzilla.jpg into ./.proxy using the proxy
   Fetching ./tiny/godzilla.jpg into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
5: tiny
   Fetching ./tiny/tiny into ./.proxy using the proxy
   Fetching ./tiny/tiny into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
Killing tiny and proxy
basicScore: 40/40

*** Concurrency ***
Starting tiny on port 9722
Starting proxy on port 14058
Starting the blocking NOP server on port 18376
Trying to fetch a file from the blocking nop-server
Fetching ./tiny/home.html into ./.noproxy directly from Tiny
Fetching ./tiny/home.html into ./.proxy using the proxy
Checking whether the proxy fetch succeeded
Success: Was able to fetch tiny/home.html from the proxy.
Killing tiny, proxy, and nop-server
concurrencyScore: 15/15
```



### Passed All Test (70/70)

```shell
➜  ~/cmu-15-213-CSAPP3E-lab/7.Proxy_lab/proxylab-handout make clean
rm -f *~ *.o proxy core *.tar *.zip *.gzip *.bzip *.gz
➜  ~/cmu-15-213-CSAPP3E-lab/7.Proxy_lab/proxylab-handout make
gcc -g -Wall -c sbuf.c
gcc -g -Wall -c reader-writer.c
gcc -g -Wall -c lrucache.c
gcc -g -Wall -c proxy.c
gcc -g -Wall -c csapp.c
gcc -g -Wall sbuf.o csapp.o reader-writer.o lrucache.o proxy.o -o proxy -lpthread
➜  ~/cmu-15-213-CSAPP3E-lab/7.Proxy_lab/proxylab-handout ./driver.sh
*** Basic ***
Starting tiny on 20753
Starting proxy on 29245
1: home.html
   Fetching ./tiny/home.html into ./.proxy using the proxy
   Fetching ./tiny/home.html into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
2: csapp.c
   Fetching ./tiny/csapp.c into ./.proxy using the proxy
   Fetching ./tiny/csapp.c into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
3: tiny.c
   Fetching ./tiny/tiny.c into ./.proxy using the proxy
   Fetching ./tiny/tiny.c into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
4: godzilla.jpg
   Fetching ./tiny/godzilla.jpg into ./.proxy using the proxy
   Fetching ./tiny/godzilla.jpg into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
5: tiny
   Fetching ./tiny/tiny into ./.proxy using the proxy
   Fetching ./tiny/tiny into ./.noproxy directly from Tiny
   Comparing the two files
   Success: Files are identical.
Killing tiny and proxy
basicScore: 40/40

*** Concurrency ***
Starting tiny on port 15206
Starting proxy on port 1702
Starting the blocking NOP server on port 29301
Trying to fetch a file from the blocking nop-server
Fetching ./tiny/home.html into ./.noproxy directly from Tiny
Fetching ./tiny/home.html into ./.proxy using the proxy
Checking whether the proxy fetch succeeded
Success: Was able to fetch tiny/home.html from the proxy.
Killing tiny, proxy, and nop-server
concurrencyScore: 15/15

*** Cache ***
Starting tiny on port 17328
Starting proxy on port 16786
Fetching ./tiny/tiny.c into ./.proxy using the proxy
Fetching ./tiny/home.html into ./.proxy using the proxy
Fetching ./tiny/csapp.c into ./.proxy using the proxy
Killing tiny
Fetching a cached copy of ./tiny/home.html into ./.noproxy
Success: Was able to fetch tiny/home.html from the cache.
Killing proxy
./driver.sh: line 399: 13704 Terminated              ./proxy ${proxy_port} &> /dev/null
cacheScore: 15/15

totalScore: 70/70
```

