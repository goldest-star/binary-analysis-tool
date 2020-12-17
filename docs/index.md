# Overview

## Summary

Hobbits is a software platform for analyzing, processing, and visualizing bits.
The Hobbits GUI is the central tool of the platform, and will be the primary
focus of this document. The Hobbits Runner CLI enables the execution of plugins
and batches without a GUI.

### Why?

Hobbits was developed to accelerate manual data analysis tasks that were
starting to burden Mahlet's hardware and software development. The de facto
solution involved the use of multiple tools with slow turnaround and improvised
integration (e.g. custom Python scripts and xxd.) Hobbits provided a fully
integrated analysis environment without sacrificing flexibility. As a result, it
has quickly accumulated success stories across the different development teams.

There are tools that are similar to Hobbits, but none can claim to be as
extensible, portable, and flexible. GUI and plugin binaries can be easily
dropped, swapped, and executed on a variety of platforms without dependencies
or restrictions. Hobbits even supports pure Python plugins via a built-in
CPython interpreter and custom hobbits library bindings. These are all priceless
advantages when developing analysis tradecraft across a variety of communities,
networks, or access levels.

### What Now?

The general architecture of Hobbits has proven to be highly effective and agile,
and the budding Python capabilities are promising. With a stable interface in
place, the development priorities will shift to core stability and performance,
adding and improving plugins, and exploring the possibilities of distributed and
client-server architectures.

## Hobbits GUI

The primary concern of Hobbits GUI is making machine-level data presentable to
humans so that they can analyze/fix/generate it easily. This usually involves
some combination of:

 - **Importing** bits, e.g. from files, packets, or a radio interface
 - **Analyzing** bits, e.g. framing, highlighting, or classifying
 - **Displaying** bits so that a human can understand them
 - **Processing** bits, e.g. enriching, decoding, or demuxing them
 - **Exporting** bits, e.g. saving a file, or forwarding to follow-on processes

Hobbits simplifies the task of developing and repeating workflows comprised of
these steps. Steps are developed independently as plugins, and Hobbits
orchestrates their execution.

## Capabilities Via Plugins

The importing, analyzing, displaying, processing, and exporting capabilities of
Hobbits are provided dynamically with plugins. (The plugin system is described
in depth [here](#plugin-system).)

While the current collection of capabilities provides a uniquely fast, easy, and
portable experience, the true power of the platform lies in its ability add new
plugins that further customize and optimize desired workflows. Thoughtful
architectural planning and useful development tools allow one to create plugins
with C++ or Python in a matter of minutes.

The ease of extensibility provided by the plugin system will enable core feature
development to proceed rapidly, and even opens up the possibility of externally
developed plugin collections.

### Displays

Displays are critical because the information bandwidth of the average human
optical physiology is very high. Variety and configurability are necessary for
showing the most important charactaristics of different types of data.

 - Bit Raster: shows frames of bits as raster lines in which each pixel
   represents a bit
 - Byte Raster: same as the Bit Raster, but each pixel represents a byte
 - Symbol Raster: same as the Bit Raster, but each pixel is a configurable
   bit-length "symbol" with a correlating custom color
 - Binary: shows frames of bits as lines of binary digits
 - Hex: shows frames of bits as lines of hexadecimal digits
 - ASCII: shows frames of bits as lines of decoded ASCII text
 - Digraph Plot: shows byte adjacency patterns
 - Dot Plot: shows byte repetition patterns
 - Hilbert Plot: shows byte sections in visibly delineated chunks
 - Frequency Plot: shows a histogram of the most common bytes
 - Spectrogram: shows a power spectral density waterfall plot for sampled data

### Analyzers

Analyzers digest and decorate the data in a way that facilitates follow-on
processing and/or human evaluation. Analyzers are non-destructive.

 - Find: finds and navigates to bit, hex, octal, and ASCII patterns in the data
 - Width Framer: frames the data to given bit-width. Includes an
   auto-correlation function and width-picker graph.
 - Highlight: enables highlighting of bit ranges
 - Kaitai Struct: highlights the data sections based on a known/specified binary
   format
 - Metadata: shows and allows editing of metadata fields

### Operators

Operators take some number of data inputs, and produce some number of data
outputs. Most operators are 1-in, 1-out, but the flexibility of the interface
enables several critical operations (e.g. muxes, demuxes.) The output of
operators will go into new containers so that the original data can still be
easily referenced.

 - Take Skip: performs common bitwise pre-processing operations using a
   simple-yet-powerful "take, skip" syntax
 - Bit Error: injects bit errors into the data
 - Symbol Remapper: maps n-bit "symbols" to other n-bit "symbols"
 - Header Framer: frames the data on a given constant header sequence
 - Extractor: extracts bits within and/or around a selected bit range highlight
 - Python Runner: runs a custom Python script using the hobbits Python API

### Importer/Exporters

Importer/Exporters are the primary way to get bitstreams in and out of hobbits.

 - File Data: import bits from or export bits to a file on the filesystem
 - HTTP Data: import bits from or export bits to an HTTP URL
 - Hex String: import bits from or export bits to an ASCII string of hex
   characters that represent the bits
 - TCP Data: import bits by listening for TCP data on a specified port, or
   export bits to a TCP server at a specified address
 - LFSR: generates LFSR bits based on given parameters (import only)
 - Packet Capture: captures packets using libpcap (import only)
 - Display Print: exports a display as an image file (export only)

## Batches

When a user develops a sequence of operations that they want to perform
frequently, they can save it as a "Batch" and perform it later with a single
step. Batches can also be used to process bits on the command line with the
hobbits-runner CLI.

## Technical Architecture

The rest of this document covers the technical architecture of the Hobbits 
platform.

## Challenges

The architecture of Hobbits is best understood through the lens of the
challenges it faces in satisfying its user goals.

### Required capabilities are unpredictable

The problem space that can be approached with a "bitwise analysis,
processing, and visualization" tool is vast, even when scoped to a specific
domain like high-speed networks. Hobbits must be prepared for unexpected
features.

### Required capabilities are numerous

A large problem space also leads to a large number of capabilities being
integrated, and each newly integrated capability beckons adjacent capabilities
to be integrated, and so on. Hobbits must be prepared for a lot of features. 

### Required capabilities are performance-intensive

When working with large amounts of data at the bit level, it is critical to use
processing cycles and memory efficiently. An effective interface and core
library needs to ensure that simple capabilities have efficient abstractions
available, while high-performance capabilities are unencumbered by abstraction.
Hobbits must provide lean interfaces and fast utilities.

### Implemented capabilities need to work together

Decoupling capabilities from one another in a plugin system makes it harder for
them to coordinate with each other and create the smoothest experience possible.
Hobbits must provide ways of orchestrating multiple plugins cleanly, and the
plugin interface must allow safe, opportunistic coordination between plugins.

## Solutions

### C++/Qt

Using C++ and the Qt framework as the foundation for Hobbits makes it easier to
meet performance requirements, and trivializes the process of turning existing
C/C++ programs into plugins. This is useful because many performance-critical
data analysis and processing capabilities are written in C/C++.

Qt enables rapid GUI design as well as a variety of well-documented convenience
tools, such as a plugin loading system.

### Python

Allowing plugins to leverage or be completely written in Python enables users
and developers to prioritize flexibility and development speed when C++ is too
cumbersome. It also lets plugins take advantage of the wealth of existing Python
packages.

### Plugin System

As mentioned above, all analysis, processing, and visualization capabilities of
Hobbits are provided dynamically with plugins. This makes it easy to add new
and unexpected capabilities without increasing the complexity of adding *more*
new and unexpected capabilities. This kind of development scaling can be
achieved without plugins, but plugins *guarantee* it. Furthemore, plugins enable
the program to be extended without any updates to the core program.

The plugin interfaces grant a lot of implementation flexibility while
maintaining a narrow, inflexible coupling to the core program. The flexibility
enables the addition of unpredictable capabilities, while the clear coupling
enables useful integration between plugins.

Another important consideration of the plugin system design was implementation
speed and ease. Adding new plugins needs to be fast and painless. The simplicity
of the essential interface components helps with this, but the core library
utilities and the plugin templates are also very important.

Finally, a plugin system allows Mahlet (or any other organization) to develop
plugins that can be used internally, or licensed and sold separately from the
core Hobbits open-source project.

### Core Library

The core library's main purpose is to provide useful tools and abstractions for
the plugin implementations. This includes ways of accessing and operating on
packed bits, ways of drawing common display primitives, and other conveniences.

The core library also contains useful capabilities for managing the integration
of different plugins in a sensible way. The Hobbits GUI relies heavily on these
default integration managers, and as a result is fairly compact. Other
applications would be able to similarly leverage the core library, so custom
Hobbits command line utilities and processing servers could feasibly be created
(or added to existing programs) in a matter of hours rather than days/weeks.

### Templates

Even with rigorous simplification of the plugin interfaces and useful
abstractions of the Qt plugin loader, there is still a small amount of
boilerplate code required when making a plugin. This boilerplate code can be
annoying to experienced developers, and it can be intimidating to developers
that are unfamiliar with the program.

Templates, in the form of Qt Creator wizards, are available to set up all of the
boilerplate code required to make a functioning plugin. This dramatically speeds
up the development of new plugins, especially simple plugins, and plugins that
just wrap existing C/C++ processing capabilities.