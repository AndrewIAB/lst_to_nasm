# lst2nasm

Tool for generating nasm %define directives from nasm lst files (generated with nasm's -l option)

## Usage

`lst2nasm {list file path} {memory offset for labels} {labels to collect}`

Note that this does not automatically detect org directives. It is also not always guarenteed to always work, as I wrote it in a day for a project.
