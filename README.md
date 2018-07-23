# Mermoz :airplane:
Web crawler for multi-threaded computers.

:loudspeaker: Dear webmasters, we crawled your website? All the needed infos
[here](#webmasters-computer) :loudspeaker:

## Build
- First clone the current repository and DO NOT REMOVE `--recursive` because
[`urlfactory`](https://www.github.com/QwantResearch/urlfactory) is needed:
```
$ git clone --recursive https://github.com/QwantResearch/mermoz.git
```

- You need to compile [`urlfactory`](https://www.github.com/QwantResearch/urlfactory)
by doing:
```
$ cd src/urlfactory
$ make
```

- Go to the root directory of `Mermoz` and check that you got all the needed
[dependencies](#dependencies).

- Finally compiling the code is really easy:
```
$ make
```

## Launch
After doing the command `make` the binary are located whihin `build/`:
```
$ cd build
$ ./mermoz --settings file --seeds file
```

The `settings` file has the following format:
```
fetchers [num fetchers]
parsers [num parsers]
user-agent [user-agent]
max-ram [GB]
```
and the `seeds` file
```
url1
[urls...]
```

## Webmasters? :computer:
Probably, you see us crawling your website, we announce ourselves as:
```
Mozilla/5.0 (compatible; Qwantify/Mermoz/0.1; +https://www.qwant.com/; +https://www.github.com/QwantResearch/mermoz)
```
with the following IPs `194.187.171.0/24`.

## Dependencies
This list is more or less like a memo:
- [`urlfactory`](https://www.github.com/QwantResearch/urlfactory) all the needed tools for
  URLs and `robots.txt`.
- [`curl`](https://github.com/curl/curl) fetch pages,
- [`gumbo`](https://github.com/google/gumbo-parser) fast and reliable HTML5 parser,
- [`boost/program_options`](https://www.boost.org/doc/libs/1_67_0/doc/html/program_options.html) basics,

## Contributing
Please first read [`CONTRIBUTING.md`](CONTRIBUTING.md) and propose what you want or you can fix or add functionalities detailed
within [`TODO.md`](TODO.md).

For any questions, comments, or collaborations, please use: **n.martin [at] qwantresearch [dot] com** or also [@stdthread](https://www.twitter.com/stdthread) on Twitter.
