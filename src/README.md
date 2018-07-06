# Sources
Within this directory you will find [`mermoz.cpp`](mermoz.cpp) which contains the code for the full
executable.

Then the sources are organized as follow:
- [`urlserver`](urlserver/) contains all the code needed by the `UrlServer` thread to manage the
  explorations of websites,
- [`spider`](spider/) is piece of code in charge of fetching and parsing the pages,
- [`common`](common/) contains all the tools needed in every parts of the program.
