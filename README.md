# Console Text Editor (DSA)

A feature-rich, console-based text editor written in C++ that simulates a full desktop text editor interface inside the terminal. This project showcases the practical application of Data Structures and Algorithms by utilizing a custom **Doubly Linked List** to handle real-time text buffering and line manipulation.

## Features

- **Text Manipulation:** Dynamic line typing, replacement, clearing, and complete line deletion.
- **File Management:** Create new files, open existing files, save progress, and maintain a history of up to 5 recent files.
- **Advanced Navigation:** Scroll through pages using Page Up/Down, jump to specific lines, and instantly snap to the top (Home) or bottom (End) of the document.
- **Find & Replace:** Search text strings globally across the document and dynamically replace them.
- **Dynamic Word Wrap:** Toggle text wrapping to prevent lines from overflowing the console boundaries.
- **ANSI Styling:** Features an interactive, colored UI complete with a title bar, top menus, and a status tracking bar.

## Data Structure Architecture

The system implements a custom `TextBuffer` using a **Doubly Linked List** (`LineNode` structure). Each line of text is explicitly tied to an individual node:

```text
[Head] <--> [Line 1 Text] <--> [Line 2 Text] <--> [Tail]

Using a doubly linked list ensures that adding new text blocks, breaking lines on user command, or deleting entire chunks runs optimally without shifting a contiguous block of elements in memory.

Getting Started
Prerequisites
Make sure you have a standard C++ compiler (like g++) installed on your system.

Compilation
Compile the file directly using your terminal:

Bash
g++ texteditor1.cpp -o TextEditor
Running the Editor
Execute the compiled file:

Bash
./TextEditor
Shortcuts Guide
Once inside the editor interface, navigate using the primary menu keys:

F — Open File Menu (New, Open, Save, Recent, Close, Exit)

E — Open Edit Menu (Type Text, Edit/Delete Line, Find & Replace)

O — Open Options Menu (Toggle Word Wrap)

V — Open View Menu (Page Up/Down, Home, End, Go to Line)

H — View Technical and Help details
