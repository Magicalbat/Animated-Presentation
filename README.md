# Animated-Presentation
A C application for creating animated presentations

- [Introduction](#introduction)
- [File Format](#file-format)
- [plugin_basic](#plugin_basic)
- [plugin_text](#plugin_text)
- [Build Instructions](#build-instructions) 
- [Custom Plugins](#custom-plugins)

Introduction
------------
Animated Presentation is a presentaiton software written from scratch in C. It can run on Linux, Windows, and there is a web version. To get started with Animated Presentation, install the latest release for your platform of choice. You can also download the example presentation if you would like.  

The presentations are made a in custom, human-readable, file format. To run the program, you need to specify which presentation file to load. In the web version, the file to load is specified in a paragraph tag in the HTML. For Windows and Linux, the file to load will be the first command line argument. If there is no argument, it will try to load `main.pres` by default. On Windows, you can also right click a presentation file, click open with, choose another applicaiton, and open it with Animated-Presentation.exe.

File Format
-----------
There is a custom file format for presentation files: `.pres`. It is similar to JSON. Here is an example:
```
plugins = [
    "plugin_basic",
    "plugin_text"
],
settings = [
    reference_dim = vec2{ 1600, 900 },
    background_color = vec4{ .5, 0.6, 0.7, 1 },
    title = "Test Presentation"
],
slides = [
    global {
        font {
            source = "res/Hack.ttf",
            sizes = [64, 96],
            default = true
        }
    },
    slide {
        text {
            x = 300, y = 100,
            text = "Hello World\nHello Again",
            center_y = true, center_x = true
            col = vec4{ 0, 1, 1, 1 }
        }
    }
]
```
As you can see, there are three main parts to a `.pres` file: plugins, settings, and slides.

### Datatypes

Here are the core data types:
- Float 64 (double)
    - `123.456`
- Strings
    - `"Hello World\nHello Again"`
- Booleans
    - `true` or `false`
- 2D Vector
    - `vec2{1, 2}`
- 3D Vector
    - `vec3{3, 4, 5}`
- 4D Vector
    - `vec4{6, 7, 8, 0}`

You can also have an array of each of these. **All elements in an array have to be the same type.** The type of the array is implicitly defined by the first element. 

`[ "One", "Two", "Three " ]`

### Plugins
This is the most straight forward section. Plugins are just a list of strings that contain the name of each plugin. This does not include the path or the file extension. The actual plugin files are dynamic libraries and are stored in a `plugins` folder that is in the executable directory.

In the example, I am using `plugin_basic` and `plugin_text`. These are two plugins that I wrote, and they come with the application. There is more information about these two below.

### Settings

This is also relatively simple. There are three fields that you can set in settings.

- `reference_dim`
    - The application will scale these dimensions to fit the current window.
    - All objects can treat the screen as having these dimensions
- `background_color`
    - This is just the background color
    - RGBA
- `title`
    - This will be the title of the window or tab that the application is running in

### Slides

This is where the majority of the presentation is defined. Broadly, this is a list of slide, but there is one exception. You can have one special `global` slide. In the example, I am using the global slide to load in a font.

A slide is a list of object and an object is a set of key-value pairs separated by commas. The key-value pairs are defined by the plugins.

```
test_obj {
    key1 = 123.456,
    key2 = [ "Hello", "World" ],
    key3 = false
}
```

plugin_basic
------------

This plugin has four objects: rectangles, images, convex polygons, and bezier curves.

- `rectangle`
    - `x`: `f64`
    - `y`: `f64`
    - `w`: `f64`
        - Width
    - `h`: `f64`
        - Height
    - `fill`: `bool`
        - Whether or not to fill the rectangle
    - `fill_col`: `vec4`
        - RGBA
    - `col`: `vec4`
        - RGBA
    - `outline`: `bool`
        - Whether or not to have an outline on the rectangle
    - `outline_col`: `vec4`
        - RGBA
    - `outline_width`: `f64`
        - How wide the outline should be
        - NOTE: the total dimensions of the rectangle will not change; the outlne will grow inward
- `image`
    - `x`: `f64`
    - `y`: `f64`
    - `scale`: `vec2`
        - Instead of width and height, images have a scale factor that is so that you can easily maintain the original image's aspect ratio
    - `source`: `string8`
        - File path of the image
    - `col`: `vec4`
        - RGBA
- `polygon`
    - `x`: `f64`
    - `y`: `f64`
    - `points`: `vec2 []`
        - List of points for the polygon
        - NOTE: As of now, polygons have to be convex
    - `col`: `vec4`
        - RGBA
- `bezier`
    - `p0`: `vec2`
    - `p1`: `vec2`
    - `p2`: `vec2`
    - `p3`: `vec2`
        - The points of the bezier
    - `width`: `f64`
    - `col`: `vec4`
        - RGBA
    - `gradient`: `bool`
    - `start_col`: `vec4`
    - `end_col`: `vec4`
        - Colors for the gradient

plugin_text
-----------

This plugins only has two objects: font and text.
This plugin uses `stb_truetype.h` and `stb_rect_pack.h` for font rendering.

- `font`
    - `source`: `string8`
        - Path to font file
    - `sizes`: `f64 []`
        - List of possible font sizes
    - `default`: `bool`
        - Whether or not this is the default font
        - The default size is the first size in the list
    - NOTE: I would recommend to putting font in the `global` slide
- `text`
    - `text`: `string8`
    - `font`: `string8`
        - The font to use, not including the full path and file extension
        - Ex: For a font at `res/MyFont.ttf`, you would write `"MyFont"` for this field
    - `x`: `f64`
    - `y`: `f64`
    - `center_x`: `bool`
    - `center_y`: `bool`
    - `col`: `vec4`
        - RGBA

Build Instructions
------------------
This project uses [premake5](https://premake.github.io/) for build configuration.
```
git clone https://github.com/Magicalbat/Animated-Presentation.git
cd Animated-Presentation
```
**NOTE: By default, clang will be used on both linux and windows. To avoid this, you can use the `--no-clang` option when running premake.**
### Windows
You can change the version if you do not have [Visual Studio 2022](https://visualstudio.microsoft.com/vs/) installed.
```
premake5 vs2022
```
Open the Visual Studio Solution to build the project.
### Linux
You made need to install some packages. The project needs to link with X11, GL, and GLX.
```
premake5 gmake2
make
```
### Emscripten
This does not work well on Windows. If you can only use Windows, you will have to modify the makefile that premake5 generates.
```
premake5 gmake2 --wasm
emmake make
```
Custom Plugins
--------------

**TODO**

## Inspired by
- [Mr. 4th Programming](https://www.youtube.com/c/Mr4thProgramming)
- [olc Pixel Game Engine](https://github.com/OneLoneCoder/olcPixelGameEngine)
- [Manim](https://github.com/3b1b/manim)
