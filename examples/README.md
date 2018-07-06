# Examples

We present here some features of the examples available:
- [URL parser](#url-parser)
- [Robots](#robots)

## URL parser
After doing `make` in the root directory, the binary will be
located within this directory.

To test it, run for instance:
```
$ ./urlparser 'https://www.example.org/news/2017?mode=dark&os=linux#title'
scheme    https
authority www.example.org
  user
  pass
  port
  domain  www.example.org
path      news/2017
  /       news
  /       2017
query     mode=dark&os=linux
  &       mode=dark
  &       os=linux
fragment  title
```

It also detects user and password, before the domain name:
```
$ ./urlparser 'sftp://john:1234@myserver.org:22'
scheme    sftp
authority john:1234@myserver.org:22
  user    john
  pass    1234
  port    22
  domain  myserver.org
path
query
fragment
```

Also it can handle URL addition if there is relative pathes, such as:
```
$ ./urlparser 'https://www.example.org/news/2017' + '../2016/june?os=linux#title'
scheme    https
authority www.example.org
  user
  pass
  port
  domain  www.example.org
path      news/2016/june
   /      news
   /      2016
   /      june
query     os=linux
    &     os=linux
fragment  title
```

The best of the best, you can compare URLs as:
```
$ ./urlparser 'https://www.example.org/news/' gt 'https://www.example.org/news/2017'
false
```
Is implemented `>, >=, <, <=` respectively coded as `gt, geq, lt, leq` (Fortran-style :floppy_disk: ).

## Robots
You can check if you are allowed to crawl some pages of website just by doing:
```
$ ./robots Website-Root User-Agent URL-To-Test
```

Then, test it:
```
$ ./robots 'https://www.facebook.com/' 'Applebot' 'https://www.facebook.com/u/'
Allowed
```
congrats you can crawl it :tada:

But sometimes, it's forbidden :warning:
```
$ ./robots 'https://www.facebook.com/' 'Applebot' 'https://www.facebook.com/ajax'
Disallowed !!!
```
