# MoniTAal

Monitor for Timed buchi automata Aalborg

## Dependencies

Install cmake and build-essentials:
```sudo apt install build-essentials cmake```

Boost:
- headers for base build
- program options for binary (needs to be built)
- unit test framework for tests (needs to be built)

See [getting started](https://www.boost.org/doc/libs/release/more/getting_started/index.html) for info on building.

## Build using cmake

```console
mkdir build && cd build
cmake .. <cmake options>
make
```

### Cmake Options:

- Build binary: `-DMONITAAL_BUILD_BIN=ON`
- Build tests: `-DMONITAAL_BUILD_TEST=ON`

## Usage

MoniTAal can monitor whether all infinite extensions of a finite timed word satisfy an MITL property. In order to use MoniTAal you have to provide two timed buchi automata, one that accepts the language of the property, and one that accepts the negation. 
Then, in an online fashion, MoniTAal can monitor a given timed word.

Models for MoniTAal can be drawn in [UPPAAL](https://uppaal.org/) with a few modifications:
- synchronizations are labels on the edges.
- Locations must have a name.
- Locations with a name ending with '_a' is an accepting location.

### Timed Word Format

MoniTAal can parse timed words in text on either interval or concrete form. A concrete timed word is where time points are singular points. Interval timed word is where the time points are intervals.

- Concrete timed word: `@0 a @10 b`
- Interval timed word: `@[0,10] a @[10, 10] b`

Each observation/event can be separated by whitespace, but has to begin with '@'.

### Binary

With the binary you can monitor a timed word from a file, and/or provide a timed word step-by-step in the interactive mode.

#### Options:
|  option                                         |     Description      |                  
|------------------------------------------------ |  --------------------|
|`-h --help`                                      | Dispay the help message Example: monitaal-bin --pos <name> <path> --neg <name> <path>|
|`-p --pos <name of template> <path to xml file>` | Property automaton. (Required)|
|`-n --neg <name of template> <path to xml file>` | Negated property automaton. (Required) |
|`-t --type (concrete \| interval)`               | Input timing type (concrete or interval) is concrete by default.|
|`-i --input <path>`                              | Start by monitoring events contained in file.|
|`-v --verbose`                                   | Prints the states during the interactive procedure.|
|`-o --print-dot`                                 | Starts by printing the dot graphs of the given automata.|
|`-d --div <list of labels>`                      | Take time divergence into account, provided the FULL alphabet of the property|

### Example

Monitoring the property: $G_{[0, \infty]}a \rightarrow_{0,30} b$.

Provide the property automata for the property and its negation:
```console
user@device:~/.../MoniTAal/build$ ./src/monitaal-bin/MoniTAal-bin -p a_leadsto_b test/models/a-b30.xml -n not_a_leadsto_b test/models/a-b30.xml
Parsed "test/models/a-b30.xml" without errors
Parsed "test/models/a-b30.xml" without errors
Interactive monitor (respond with "q" to quit)
Next event:
```

While in interactive mode, you can provide a timed word step-by-step. The procedure automatically stops when we have a verdict for all possible infinite extensions of the word. 
```console
Next event: @0 a
Next event: @20 b
Next event: @25 a
Next event: @56 b
Monitoring ended, verdict is: NEGATIVE
Monitored 4 events
user@device:~/.../MoniTAal/build$
```
