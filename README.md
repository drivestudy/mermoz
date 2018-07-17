# Mermoz :airplane:
Web crawler for multi-threaded computers

## Start it
Compile the code by doing `make` and then launch it:
```
$ cd build
$ ./mermoz --parsers N --fetchers M --seed URL --user-agent UA
```

## Webmasters? :computer: 
Probably, you see us crawling your website, we announce ourselves as:
```
Mozilla/5.0 (compatible; Qwantify/Mermoz/0.2; +https://www.qwant.com/; +https://www.github.com/QwantResearch/mermoz)
```
with the following IPs `194.187.171.0/24`.

## Examples

The detailed readme of examples is located within [`examples/README.md`](examples/README.md).

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
```

Also it can handle URL addition if there is relative paths, such as:
```
$ ./urlparser 'https://www.example.org/news/2017' + '../2016/june?os=linux#title'
```

The best of the best, you can compare URLs as:
```
$ ./urlparser 'https://www.example.org/news/' gt 'https://www.example.org/news/2017'
```
Is implemented `>, >=, <, <=` respectively coded as `gt, geq, lt, leq` (Fortran-style :floppy_disk: ).

## Dependencies
This list is more or less like a memo:
- `boost/program_options` basics,
- `curl` fetch pages,
- `gumbo` fast and reliable HTML5 parser,

## Contributing
Please first read [`CONTRIBUTING.md`](CONTRIBUTING.md) and propose what you want or you can fix or add functionalities detailed
within [`TODO.md`](TODO.md).

For any questions, comments, or collaborations, please use: **n.martin [at] qwantresearch [dot] com** or also [@stdthread](https://www.twitter.com/stdthread) on Twitter.
