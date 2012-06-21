BFFD - Binary File Format Debugger
==================================

August 11, 2010

Usage
=====

bffd <bfft_file> <binary_file>

Analyzes the binary_file according to the file format template specified by the bfft_file. Once analysis is complete a command-line interface is presented to the user where they can perform a host of operations on the analysis to get detailed information about the bytes in the file.

BFFT Format
===========

The BFFT (binary file format template) format is based on an EBNF grammar that is as follows, along with specific notes where appropriate:

Grammar
-------

translation_unit   = { struct_set }
struct_set         = [ comment | struct ] { struct_set }
comment            = trail_comment | lead_comment
struct             = "struct" identifier '{' { statement_set } '}'
statement_set      = scope_or_statement { statement_set }
scope_or_statement = conditional_scope | statement
conditional_scope  = if_scope { comment } { else_scope }
if_scope           = "if" expression scope_content
else_scope         = "else" scope_content
scope_content      = '{' { statement_set } '}' | scope_or_statement
statement          = comment | [ invariant | constant | notify | skip | field ] ';'
invariant          = "invariant" identifier expression
constant           = "const" identifier expression
notify             = "notify" argument_list
skip               = "skip" identifier expression
field              = [ struct_field | atom_field ] identifier { field_size } { offset } { callback }
struct_field       = "struct" identifier
atom_field         = [ "float" | "unsigned" | "signed" ] expression [ "big" | "little" | expression ]
field_size         = '[' expression ']' | "while" expression
offset             = '@' expression
callback           = "tell" | expression

struct
------

There is always at least one struct declaration in a BFFT file. One of the structs named in the file must be 'main', and is the first struct used to analyze a binary file. The structures can be defined in any order in the BFFT, but no two may have the same name.

field
-----

Every structure is composed of one or more fields which represent an interpretation of the actual bytes of the file. There are two fundamental types of fields: atoms and structures. Atoms represent contiguous bytes in the binary file. Structures are a collection of fields and the fundamental building block for nesting. Fields (both atoms and structs) can be singletons or arrays depending on how they are declared. The simplest BFFT, then, is:

    struct main
    {
    }

The above structure will do nothing. Fields inserted into a structure (if there are any) will be analyzed based on the current "read position", which advances by the number of bytes interpreted by a field when one is specified.

No two fields may have the same name inside a structure. Additionally no field may be named 'main' or 'this'. Violating either of these conditions will result in undefined behavior.

condition
---------

Conditional statements apply either to the next statement or the next collection of statments (when wrapped in braces). Conditional blocks can be grouped into the traditional if/else if/else block set. For example to switch analysis of a PNG chunk based on the type of chunk found, one might use the following if/elseif/else block:

    if (str(@type) == 'IHDR')
        struct ihdr_t ihdr;
    else if (str(@type) == 'PLTE')
        struct plte_t plte;
    else if (str(@type) == 'tEXt')
        struct text_t text;
    else if (str(@type) == 'iTXt')
        struct itxt_t itxt;
    else
    {
        notify "Unknown chunk type: ", str(@type);

        skip unknown length;
    }

atom_field
----------

Both the base type (float/signed/unsigned) and endian decorators (big/little/expression) are required for all atoms, though omitting them is planned for a future version. (Yes, I know endianness makes no sense for an atom <= 1 byte in size. The notation is still required.)

The expression in the middle of the atom field declaration must evaluate to an integer. It represents the number of bits this field interprets in the file. All values must be byte-aligned. Support for values smaller than a byte is planned for a future version.

The float base type is only supported in 32- and 64-bit atoms. Float atoms specified at other sizes will result in an error.

The endian 'expression' option must evaluate to a boolean: true for big endian and false for little endian. This can be useful for portions of a file format whose endianness depends on the value of another field (e.g., exif metadata in a JPEG.) (The big and little keywords are syntactic sugar for true and false, respectively.)

field_size
----------

Fields are allowed to represent an array of bytes in the file. Array sizes can be static or dynamic. Static sizes use the "[expression]" declaration format. For example to read in 5 bytes from the file the field declaration may look like:

    unsigned 8 big five_bytes[5];

The size expression is evaluated at the time the field is to be read. As another example one might read the length of a field and then want to read the field according to the length. In such case the field declarations may look like:

    unsigned 16 big my_length;
    unsigned 8  big my_field[my_length];

Dynamic sizes use the "while expression" declaration format, and a different expression is required depending on what kind of field the size is specified for. For structures the dynamic expression must evaluate to a boolean; at the time the expression evaluates to false the array is terminated. For example to read JPEG segment headers until the start of stream marker is read the field declaration may look like:

    struct segment_t segment while card(@segment) == 0 ||
                                   segment[card(@segment) - 1].app_marker2 != 218; // 218 == \xDA

For atoms the dynamic expression must evaluate to a dictionary. The dictionary must have the following keys:

  - size: number of bits to consider for the stop value (the stop value window)
  - value: the stop value
  - inclusive: boolean denoting if the stop value should be included in the array. This parameter is optional and is false by default.

Using that dictionary the atom array will be filled with data from the file until the stop value is found. For example to read a null-terminated 8-bit string from a file the field would look like:

    unsigned 8 big ntbs while { size: 8, value: 0, inclusive: true };

As another example, to read JPEG image stream data until the end of image marker (\xFFD9) is found, the field declaration may look like:

    unsigned 8 big image_stream while { size: 16, value: 65497 };

Because 'inclusive' is omitted from the while expression it defaults to false, and the next bytes to be read will be the stop value (the JPEG EOI marker). Note that the analyzer will read in 8 bit increments (as per the field's specification) though the stop value's window is 16 bits. (It is not correct in this case to read 16 bytes at a time, as the EOI marker is not guaranteed to fall on an even byte boundary.) The stop value size must be byte-aligned; support for values smaller than a byte is planned for a future version.

Note that struct-"while" is several orders of magnitude slower than the atom-"while" because the boolean expression must be evaluated at every iteration of the loop, whereas the atom-"while" is tuned for rapid input.

offset
------

Some fields are specified at an offset from the current reading position in the file. The offset expression must evaluate to an integer, and is the absolute offset of the data to be read. For example some JPEG Exif data is too big to fit in the 4 bytes available in the Exif field, so an offset into the file is used to locate the extended data. All data fetched with an offset specification is referred to as "remote data". (Likewise data fetched without an offset is referred to as "local data".) As such a possible declaration of the Exif field with (potentially) remote data may look like:

    if (total_data_size > 4)
        unsigned 8 big remote_data[total_data_size] @ startof(@tiff_header) + data_or_offset;

Reading remote data does not affect the read position once the reading is complete. For example given the following fields:

    unsigned 8 big offset;
    unsigned 8 big remote_data @ offset
    unsigned 8 big some_value;

The bytes 'offset' and 'some_value' are adjacent to one another in the binary file. In the analysis however, the 'remote_data' will be between them as the remote data is brought into its interpreted location in the file during analysis. Likewise a parent's starting and ending offset (startof and endof) is only affected by its local data, though this may change in a later version.

constant
--------

Constant fields are expressions used to increase the readability of a BFFT. They do not consume any bytes in the binary file and have no "size". For example one can use a constant expression to provide an endianness value to subsequent fields once an endian identifier is read from a file:

    unsigned 16 big tiff_header;   // 0x4949 (little) or 0x4d4d (big)
    const BE_k tiff_header == 19789; // 0x4d4d
    unsigned 16 BE_k tiff_tag_mark; // 42

Here the constant field "BE_k" evaluates to true or false depending on the tiff header's value. From there it affects the endianness of tiff_tag_mark by being used as its endian expression.

invariant
---------

Invariant declarations are expressions used to increase the reliability of a binary file's interpretation. They do not consume any bytes in the binary file and have no "size". At the point an invariant expression is declared it will be evaluated within the structure and must return a boolean. If the boolean result is false, a notification of the invariant's failure is posted and analysis of the binary file is halted. For example when reading JPEG marker segments the first byte must always be 0xFF; to assert this an invariant field may look like:

    struct jpeg_segment_t
    {
        unsigned 8 big marker1; // must be 0xFF
    
        invariant ok_marker1 marker1 == 255; // 0xFF
    
        //...
    }

In the above description the analyzer will halt analysis if a segment's first byte fails to be \xff, implying an incomplete BFFT for the format or a corrupt binary file. When an invariant fails the user will be able to debug what has been analyzed up to the point of failure via the command line.

skip
----

Skip declarations are a way of passing over binary file data and still have it count towards the size of a structure. There can be several reasons for this, including (bit not limited to) unimplemented substructures, remote data passover, or ignoring uninteresting data. One thing to keep in mind is the skip declaration takes byte sizes instead of bit sizes as the amount of data to skip. For example one might skip over all PNG chunk data that is uninteresting for the purposes of the current analysis:

    if (str(@type) == 'tEXt')
        struct text_t text;
    else if (length != 0)
        skip unexamined length; // not text; skip chunk data

Note that skip declarations are very fast because they ignore the data on-disk, however since skips show up as a field in a structure you can search for them by offset and get details about them (however limited).

notify
------

Notification declarations are a way of communicating with the user through the command line as to the status of the parse. The argument list for the notify declaration is a comma-separated list of values, each of which will be evaluated in-context and concatenated to stdout. For example to send out notifications when a new PNG chunk is found, one might express the notification as:

    notify "found ", str(@type), " @ ", startof(@length);

callback
--------

The BFFD library (separate from the command-line implementation) supports the ability for a client to specify a callback to be notified when requested of nodes that have been discovered during analysis. The callback will be notified whenever it is specified and the callback expression evaluates to true for a node. The keyword "tell" can be used in place of true for readability. Example:

    unsigned 32 little width tell;

BFFT/BFFD PATH DEDUCTION
========================

Path deduction in a BFFT or by the BFFD command line leverages the notion that a forest of data is being built up about the file. As such there are two fixed-name nodes, 'main' which always refers to the topmost node the tree and 'this' which always refers to the current structure being parsed.

To specify an absolute path, then, one should always start the path from 'main'. To specify a relative path from the current node to any child nodes, one should always start the path from 'this'.

Paths that begin with neither 'main' nor 'this' go through a lookup process. Starting at the current node a child field with the specified name is searched for. If it is found it is returned, and the path is further deduced from that node. If it is not found the same search is conducted on parent node. This lookup process repeats until it fails at 'main', at which point an error is emitted to the user.

Using this deduction method it is possible to reach up the tree and across to siblings that have already been analyzed to leverage the information they posses. As an example we look at a portion of the BMPv1 BFFT from the end of this file:
    
    struct pixel_row_t
    {
        struct pixel_t pixel_set[dib_header.width];
        unsigned 8 big padding[dib_header.width % 4];
    }
    
    struct main
    {
        struct header_t    header;
        struct dib_t       dib_header;
        struct pixel_row_t pixel_row_set[dib_header.height];
    }

Note how the pixel_row_set_t's pixel_set declaration sets its fixed size to 'dib_header.width'. Because that path begins with neither 'main' nor 'this', path deduction is invoked. The first parent of pixel_row_t in this description is main, which does have a dib_header field. From there the 'width' subfield of dib_header is traversed and its value used for the size of the pixel_set array.

Note that any path specification is only valid for nodes that have already been analyzed by the time the path is evaluated. It is not possible, then, to "look ahead" into an unanalyzed portion of the binary file to use a value. The proper way to fetch such a value would be to use a remote data specification to fetch it prior to requiring its use.

BFFT EXPRESSION COMMANDS
========================

There are several commands one can use while specifying an expression. Note that many of these commands take "addresses" of fields in the analysis instead of the fields themselves. In the event a BFFT is ill-formed and a field is used instead of an address you will see an error like:

    bad_cast: adobe::implementation::forest_iterator<node_t>* -> name_t:version_1:adobe

sizeof(@field)
--------------

Returns the size of the field specified in bytes.

sizeof(@field1, @field2)
------------------------

Returns the number of bytes used from the start of field1 to the end of field2.

startof(@field)
----------------

Returns the offset to the first byte of the field.

endof(@field)
----------------

Returns the offset to the last byte of the field. (For you STL users out there this is *not* one-past-the-end!)

byte(offset)
------------

Returns the value of the byte found at the specified offset as an unsigned integer.

card(@field)
------------

Returns the cardinality of an array root

str(@field)
------------

Returns a character string representation of @field

path(@field)
------------

Returns the absolute path to @field as a string. If @field is omitted, @this is implied.

indexof(@field)
---------------

Returns the array element index of the specified field. If the field is not an element in an array an error is emitted. Typically this is used as indexof(@this) to get the array element index of the current structure being analyzed. This can be useful for cross-hierarchy access when two arrays are related and are the same size. For example:

    struct main
    {
        unsigned 8 big length;
        struct foo_t foo[length];
        struct bar_t bar[length];
    }

Above, if an element in bar wanted to access data in its corresponding element in foo, it could fetch its own index with indexof(@this) and pass that to an expression index into foo.

BFFT LIMITATIONS
================

All bit-size values must be byte-aligned. Sub-byte size support is planned for a future version.

All numbers in the BFFT file must be in decimal notation. Hexadecimal notation is planned for a future version.

BFFD COMMAND LINE
=================

After the binary file is analyzed a command line is presented to the user that allows them to explore the results of the analysis. We'll use a 50x50 BMPv1 file as a running example throughout this section to highlight the features of the command line interface. A BFFT for BMPv1 can be found in the section following.

The first command line presented to the user looks like this:

    $main$

The value between the '$' is the current "path". main is always the first structure used to interpret the binary file and as such will always be at the beginning of any path. As the user moves in an out of substructures and arrays the path will tell the user where they are in the binary file interpretation.

Each command has several strings usable for invocation, each of which will be listed below in parentheses after the verbose version. Commentary about the commands is also provided as appropriate.

BFFD COMMANDS
=============

quit (q)
--------

Terminates the BFFD

help (?)
--------

Prints BFFD help

print_struct (ps) (ls) (ll)
---------------------------

Displays a synopsis of the structure at the current path. For example:

    $main$ ll
    (main) main
    {
      (header_t) header
      (dib_t) dib_header
      (pixel_row_t) pixel_row_set[50]
    }

print_branch (pb)
-----------------

Displays a complete synopsis of the current structure at the current path. Executing print_branch at $main$ will output the entire contents of the analysis. For leaf structures this command is equivalent to print_struct.

print_string <path> (str)
-------------------------

Displays the field specified by the path as a string. Values that have no graphical representation (i.e., std::isgraph(c) == false) are output as their ASCII value in hex prepended with an "\x". For example:

    $main.dib_header$ str bpp
    \x18\x0

An equivalent command is:

    $main$ str dib_header.bpp
    \x18\x0

(The example isn't very useful as there are no strings in a BMPv1 file, but you get the idea.)

step_in <path> (si) (cd)
------------------------

Sets the path to the structure defined by the path. This can be done both relative to the current path and absolutely from $main$. For example:

    $main$ cd header
    $main.header$ 

or

    $main$ cd pixel_row_set[5].pixel_set[5]
    $main.pixel_row_set[5].pixel_set[5]$ 

You can also specify an absolute path from $main$:

    $main.pixel_row_set[5].pixel_set[5]$ cd main.header
    $main.header$ 

You can also step out to the parent structure with '..'. For example:

    $main.header$ cd ..
    $main$

For arrays it is possible to step into the array without stepping into an element of that array. It is known as the "array root":

    $main$ cd pixel_row_set
    $main.pixel_row_set$

You can also refer to the current structure with the keyword this:

    $main.pixel_row_set$ cd this[4]
    $main.pixel_row_set[4]$ 

step_out (so)
-------------

Sets the path to be the parent structure of the current path. Note that in the case of an array element, the parent structure is the array root and not the structure that contains the array.

top (t)
-------

Sets the path to $main$

detail_field <path> (df)
------------------------

Prints out detailed information about the path specified. For example:

    $main$ df this
         path: main
         type: struct
       struct: main
        bytes: [ 0 .. 7625 ]
         size: 7626 bytes (7.44727 KB)

Note that the information displayed differs on the field type:

    $main.header$ df file_size
         path: main.header.file_size
       format: 32-bit unsigned little
       offset: 2
          raw: 0xca 0x1d 0x00 0x00 
        value: 7626 (0x1dca)

or

    $main$ df pixel_row_set[1].pixel_set
         path: main.pixel_row_set[1].pixel_set
         type: array
       struct: pixel_t
     elements: 50
        bytes: [ 178 .. 327 ]
         size: 150 bytes

detail_offset <offset> (do)
---------------------------

Searches the binary file analysis for the atom that interprets the byte at the provided offset. Currently only local data is included in the search (not remote data) though its inclusion is planned for a future release. For example:

$main$ do 1234
     path: main.pixel_row_set[7].pixel_set[48].blue
   format: 8-bit unsigned 
   offset: 1234
      raw: 0xff 
    value: 255 (0xff)

Sometimes the result will be in the middle of an atom, in which case the whole atom will be detailed. For example:

    $main$ do 16
         path: main.dib_header.header_size
       format: 32-bit unsigned little
       offset: 14
          raw: 0x0c 0x00 0x00 0x00 
        value: 12 (0xc)

evaluate_expression <expression> (eval) (ee)
--------------------------------------------

Allows for the evaluation of an expression whose result is immediately output. For example the following prints the starting offset of the main.dib_header.bpp field:

    $main.dib_header$ ee startof(@bpp)
    24

dump_field <field1> <field2> (duf)
----------------------------------

Dumps the on-disk bytes interpreted by the field (in the case one field is supplied) or range of fields (in the case two fields are supplied). Dump format is in five columns: the first four columns are the hexidecimal representation of
4 bytes each. The fifth column is the ASCII representation of the first four columns. If a byte fails std::isgraph a '.' is subsituted as the glyph in the fifth column.

dump_offset <start_offset> <end_offset> (duo)
----------------------------------

Same behavior as dump_field but takes a starting and ending offset into the file instead of field(s). The byte at the end offset is included in the dump.

BMP BFFT EXAMPLE
================

The following is a specification for version 1 of the BMP image file format in BFFT. Note how expressions inside some of the structures leverage path deduction heuristics in their specifications.

struct header_t
{
    unsigned 8  big    magic_number1;
    unsigned 8  big    magic_number2;
    unsigned 32 little file_size;
    unsigned 16 little creator1;
    unsigned 16 little creator2;
    unsigned 32 little bmp_offset;
}

struct dib_t
{
    unsigned 32 little header_size;
    unsigned 16 little width;
    unsigned 16 little height;
    unsigned 16 little color_plane_count;
    unsigned 16 little bpp;
}

struct pixel_t
{
    unsigned 8 big blue;
    unsigned 8 big green;
    unsigned 8 big red;
}

struct pixel_row_t
{
    struct pixel_t pixel_set[dib_header.width];
    unsigned 8 big padding[dib_header.width % 4];
}

struct main
{
    struct header_t    header;
    struct dib_t       dib_header;
    struct pixel_row_t pixel_row_set[dib_header.height];
}
