# How to build

## Dependencies

1. clang / clang-cl (18.1.8)
2. meow_hash

So, because I accidentaly forget to delete .git folder from meow_hash while cloning it, now all cloning process is a joke...

Now you have only 2 ways:

1. Clone with `--recursive` flag. But I assume you already cloned the repo? So:
2. Write this `git submodule init` and `git submodule update` and wait for it to clone meow_hash repo.

## Build

Do this to build:

```
clang -o build.exe build.c && build.exe
```

## Run

...

```
bin\chiff.exe
```
