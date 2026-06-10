# CSE-B_MINI-PROJECT

## 🎮 Controls

The editor is entirely keyboard-driven. Use the following keys from the main canvas:

* **`A`**: **Add** a new shape. Opens a selection menu and prompts for coordinates.
* **`D`**: **Delete** an existing shape. Opens a numbered list of active objects to remove.
* **`M`**: **Modify** a shape. Allows you to edit the coordinates/radius of a specific object.
* **`C`**: **Clear** the entire canvas and delete all active objects.
* **`Q`**: **Quit** the application.

## 🧠 Under the Hood

* **Rendering Engine:** The canvas is stored as a 2D `char` array. The `render_all()` function wipes the array clean and redraws every active object sequentially from the `objects` array every time a change is made.
* **Soft Deletion:** When a shape is deleted, its `active` flag is set to `0` rather than shifting array memory. (Note: New shapes will claim inactive slots to prevent memory leaks).
* **Popup Architecture:** Input prompts utilize sub-windows in `ncurses` with `touchwin()` and `wrefresh()` to prevent graphical ghosting when returning to the main canvas.

```markdown
# 🎨 2D ASCII Graphics Editor

A lightweight, terminal-based vector graphics editor written in C. Built entirely with the `ncurses` library, this application allows you to draw, modify, and manage geometric shapes directly from your command line using a fully interactive Text User Interface (TUI).

## ✨ Features

* **Interactive TUI:** Clean menu system with popup dialogs for coordinates and shape management.
* **Math-Backed Rendering:** Implements classic computer graphics algorithms from scratch, including Bresenham's line algorithm and the Midpoint circle algorithm.
* **Shape Management:** Add, modify, or delete shapes on the fly. 
* **Four Primitive Shapes:**
  * Lines (Bresenham's)
  * Rectangles
  * Circles (Midpoint)
  * Triangles
* **Color Support:** Utilizes terminal colors to differentiate between canvas borders (`cyan`), drawn elements (`magenta`), and TUI elements.
* **Memory Efficient:** Uses a `union` structure to store shape data, ensuring minimal memory footprint.

## 📸 Preview

```text
  ASCII 2D GRAPHICS EDITOR  [ * = fill  _ = border ]                            
 +------------------------------------------------------------------------------+
 |    __________________________                                                |
 |    _                        _                                                |
 |    _         *******        _                                                |
 |    _       **       **      _                                                |
 |    _      *           *     _                                                |
 |    _____*_______________*___                                                 |
 |         *               *                                                    |
 |           **         **                                                      |
 |             *********                                                        |
 +------------------------------------------------------------------------------+
  [A]dd   [D]elete   [M]odify   [C]lear   [Q]uit                                
  Objects: 2/64  |  Ready
