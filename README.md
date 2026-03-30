# CVTaggerTool

![plot](./assets/Screenshot01.jpg)

https://github.com/user-attachments/assets/3b6ee81a-c87a-433c-8c02-6a32dd5ddc33

CVTaggerTool is a simple, lightweight **computer vision annotation tool** built with Qt 6.  
It supports the most common annotation types used in CV datasets:

- Bounding Boxes  
- Polygons  
- Points  
- Lines, Line Strips  
- Circles  

The goal of this project is to provide an **easy-to-use, dependency-light alternative** to existing heavy annotation tools. Because it is a pure Qt app, it integrates well into Linux/Window/Mac  without requiring complex runtime environments.



## Usage

- **Open a project**: set the *image* and *annotation* folders, then click the **Open Project** button.  
- **Add annotations**: use the corresponding icons in the toolbar.  
- **Handy shortcuts**:
  - **Next/previous image** → `Left/Right arrows` 
  - **Lock / Unlock item** → `Shift + Right Click`
  - **Open edit dialog** → `Left Click` on an unlocked item
  - **Zoom in/out** → `Mouse Scroll`
  - **Pan image** → `Ctrl + Right Mouse Drag`
  - **Cycle selection** (when items overlap) → `Ctrl + Shift + Right Click`
  



## Features

- Create and edit multiple annotation types  
- Lock/unlock items to prevent accidental edits  
- Export annotations to:
  - **Native JSON format** (one file per image)
  - **COCO JSON** dataset format
  - **YOLO (Ultralytics)** text format  
- Undo/redo with `QUndoStack`  
- Cross-platform (Linux, Windows, macOS [not tested])
- Ready to use executables here for [Window 11](link here) and Linux (link here)

## Tagger Tool Internal JSON format 

The native annotation export stores **one JSON file per image** inside the annotation folder.  
For an image named `frame_001.png`, the annotation file name is:

```text
frame_001_png.json
```

The root object contains image metadata plus one array per supported annotation type:

- `image_name`: original image file name
- `image_w`, `image_h`: image width and height in pixels
- `label`: optional image-level label
- `description`: optional image-level description
- `lines`: array of line annotations
- `bboxes`: array of bounding boxes
- `circles`: array of circles
- `points`: array of points
- `polygons`: array of closed polygons
- `line_strips`: array of open polyline annotations

All annotation objects include:

- `label`: class/tag name
- `description`: free-text description

Per-shape fields:

- `lines`: `x1`, `y1`, `x2`, `y2`
- `bboxes`: `x1`, `y1`, `x2`, `y2`, `occluded`, `truncated`, `crowded`
- `circles`: `x`, `y`, `radius`
- `points`: `x`, `y`
- `polygons` and `line_strips`: `x_coords`, `y_coords` as matching coordinate arrays

Example:

```json
{
  "image_name": "frame_001.png",
  "image_w": 1920,
  "image_h": 1080,
  "label": "",
  "description": "",
  "lines": [
    {
      "label": "lane_marking",
      "description": "",
      "x1": 120.0,
      "y1": 300.0,
      "x2": 640.0,
      "y2": 820.0
    }
  ],
  "bboxes": [
    {
      "label": "car",
      "description": "sedan",
      "x1": 420.0,
      "y1": 280.0,
      "x2": 710.0,
      "y2": 640.0,
      "occluded": false,
      "truncated": false,
      "crowded": false
    }
  ],
  "circles": [
    {
      "label": "wheel",
      "description": "",
      "x": 560.0,
      "y": 700.0,
      "radius": 42.0
    }
  ],
  "points": [
    {
      "label": "keypoint",
      "description": "",
      "x": 960.0,
      "y": 540.0
    }
  ],
  "polygons": [
    {
      "label": "road_sign",
      "description": "",
      "x_coords": [100.0, 160.0, 140.0],
      "y_coords": [100.0, 110.0, 180.0]
    }
  ],
  "line_strips": [
    {
      "label": "boundary",
      "description": "",
      "x_coords": [50.0, 70.0, 120.0],
      "y_coords": [400.0, 420.0, 460.0]
    }
  ]
}
```


## License

This project is released under the [MIT License](https://opensource.org/licenses/MIT).  
See the [LICENSE](LICENSE) file for the full text.

## Requirements

- **Qt 6.5+** (tested with 6.9)  
- CMake 3.16+  
- A C++17 (or later) compiler (GCC, Clang, MSVC all supported)



## Building

```bash

git clone https://github.com/otre99/TaggerTool
```
Open `TaggerTool/src/CMakeLists.txt` with QtCreator and compile it.

This produces the `TaggerTool` binary inside the build folder.

## Usage

```bash
./TaggerTool
```


## Contributing

Contributions are welcome!
Please open issues or pull requests on the GitHub repository.


## Acknowledgements
 - Built with Qt
 - COCO export based on the official dataset specification
 - YOLO export compatible with Ultralytics
