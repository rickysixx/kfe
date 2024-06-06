# kfe - Kaleidoscope front-end compiler

Progetto per il corso di Linguaggi e compilatori (a.a. 2022-2023).

Il progetto consiste in un frontend per il linguaggio di programmazione fittizio [Kaleidoscope](https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/LangImpl01.html).

## Dipendenze

Il progetto è stato testato con le seguenti versioni delle dipendenze, ma potrebbe funzionare anche con versioni più vecchie/nuove:
- bison 3.8.2
- flex 2.6.4
- g++ 14.1.1
- llvm 17.0.6

## Usage

Clonare il repository e compilarlo con `make all`.

Per testare uno degli esempi in `kaleidoscope-examples`:
```bash
# creazione file oggetto
bin/kfe -o kaleidoscope-examples/array/array{,.k}

# compilazione
g++ -o kaleidoscope-examples/array/array kaleidoscope-examples/array/{main.cc,array.o}

# esecuzione
kaleidoscope-examples/array/array
```
