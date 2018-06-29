# Mermoz :airplane:
Web crawler for multi-threaded computers

## Examples

### URL Parser

One of the tool developed within the crawler is the URL parser.
You can compile the CLI example:
```
$ make urlparser
```
The executable will be located within `examples/`.

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

## Dependencies
This list is more or less like a memo:
- `boost/program_options` basics,
- `librdkafka++` external services communications,
- `curl` fetch pages,
- `gumbo` fast and reliable HTML5 parser,
- `redis` distributed in-memory DB.

## Contributing
Please first read [`CONTRIBUTING.md`](CONTRIBUTING.md) and propose what you want or you can fix or add functionalities detailed
within [`TODO.md`](TODO.md).

For any questions, comments, or collaborations, please use: **n.martin [at] qwantresearch [dot] com**.
