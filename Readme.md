# todoview

An ncurses-based todo.txt app. View multiple lists at the same time, each with different filters and sorting rules.

## Compiling
```bash
# make clean
# make
```

## Usage
```Usage: todoview [todo.txt]```

Type the name of a list in the filters window to have that pane only show items with that list. Each line in the filters window represents one list that will be shown.

## Keyboard shortcuts

In list view:
* _+_: Add a new pane
* _-_: Remove the rightmost pane
* _left or right arrow_: Move focus left/right
* _enter_: Show the filters window for the focused pane

In filter window:
* _enter or esc_: Hide the filters window, saving any changes
* _up or down arrow_: Move between filter lines

## License

Copyright (c) 2017 Ian Glen

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
