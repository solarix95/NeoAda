# Publishing the Book

## GitHub

Commit the book into the NeoAda repository (for example under `/docs`) and link it prominently from the main README. GitHub renders Markdown directly.

## MkDocs / GitHub Pages

A starter `mkdocs.yml` is included. Adjust paths depending on where the package is placed in the repository. A useful site title is:

**Programming in NeoAda — Beginner-Friendly NeoAda Programming Guide**

Give each chapter its own HTML page so topics such as NeoAda exceptions, C++ embedding, strings, lists, and high-integrity design each have an indexable URL.

## Documentation testing

Add CI that extracts runnable NeoAda code blocks and executes them with the interpreter. Mark conceptual examples so they can be skipped.

## Versioning

Tag book releases with NeoAda releases or state the minimum compatible NeoAda version at the top of the book.
