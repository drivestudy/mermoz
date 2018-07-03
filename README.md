# Mermoz :airplane:
Web crawler for multi-threaded computers

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
