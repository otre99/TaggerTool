import json
import argparse
import os
from PIL import Image
from tqdm import tqdm
from pathlib import Path

COMMON_EXTENSIONS = {".jpg", ".jpeg", ".png", ".bmp", ".tiff"}


def create_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--images_folder",
        type=str,
        required=True,
        help="Path to the folder containing images",
    )
    parser.add_argument(
        "--annotations_folder",
        type=str,
        required=True,
        help="Path to the folder containing YOLO annotation files",
    )
    parser.add_argument(
        "--classes_file",
        type=str,
        required=True,
        help="Path to the file containing class names",
    )
    parser.add_argument(
        "--output_folder",
        type=str,
        required=True,
        help="Path to the output JSON annotation files",
    )
    return parser


def main(FLAGS):
    with open(FLAGS.classes_file, "r") as f:
        classes = [line.strip() for line in f.readlines()]

    yolo_annotations_folder = Path(FLAGS.annotations_folder)
    yolo_ann_files = [
        f
        for f in yolo_annotations_folder.iterdir()
        if f.is_file() and f.suffix == ".txt"
    ]

    for yolo_ann_path in tqdm(yolo_ann_files, desc="Processing YOLO annotations"):

        yolo_ann_path.stem

        img_file = yolo_file.replace(".txt", ".jpg")

        img_path = Path(FLAGS.images_folder) / img_file
        yolo_path = Path(FLAGS.annotations_folder) / yolo_file

        if not os.path.exists(img_path):
            print(
                f"Warning: Image file {img_file} not found for annotation {yolo_file}"
            )
            continue

        pil_img = Image.open(img_path)

        with open(yolo_path, "r") as f:
            lines = f.readlines()

        image_annotations = {
            "description": "",
            "label": "",
            "image_h": -1,
            "image_w": -1,
            "image_name": img_file,
            "line_strips": [],
            "bboxes": [],
            "polygons": [],
            "points": [],
            "line_strips": [],
            "lines": [],
            "circles": [],
        }

        bboxes = image_annotations["bboxes"]
        polygons = image_annotations["polygons"]
        for line in lines:
            parts = line.strip().split()
            class_id = int(parts[0])

            if len(parts) < 5:
                print(
                    f"Warning: Invalid annotation format in file {yolo_file}: {line.strip()}"
                )
                continue

            if len(parts) == 5:  # x_center, y_center, width, height
                bbox = list(map(float, parts[1:5]))  # x_center, y_center, width, height
                bboxes.append(
                    {
                        "crowded": False,
                        "description": "",
                        "label": (
                            classes[class_id] if class_id < len(classes) else "unknown"
                        ),
                        "occluded": False,
                        "truncated": False,
                        "x1": (bbox[0] - bbox[2] / 2) * pil_img.width,
                        "x2": (bbox[0] + bbox[2] / 2) * pil_img.width,
                        "y1": (bbox[1] - bbox[3] / 2) * pil_img.height,
                        "y2": (bbox[1] + bbox[3] / 2) * pil_img.height,
                    }
                )

            if len(parts) > 5:  # polygon
                coords = list(map(float, parts[1:]))
                if len(coords) % 2 != 0:
                    print(
                        f"Warning: Invalid polygon coordinates in file {yolo_file}: {line.strip()}"
                    )
                    continue
                polygon = [
                    {
                        "x": coords[i] * pil_img.width,
                        "y": coords[i + 1] * pil_img.height,
                    }
                    for i in range(0, len(coords), 2)
                ]
                polygons.append(
                    {
                        "description": "",
                        "label": (
                            classes[class_id] if class_id < len(classes) else "unknown"
                        ),
                        "x_coords": [p["x"] for p in polygon],
                        "y_coords": [p["y"] for p in polygon],
                    }
                )

        image_annotations["image_h"] = pil_img.height
        image_annotations["image_w"] = pil_img.width

        json.dump(
            image_annotations,
            open(
                os.path.join(
                    FLAGS.output_folder, yolo_file.replace(".txt", "_jpg.json")
                ),
                "w",
            ),
            indent=4,
        )


if __name__ == "__main__":
    p = create_parser()
    FLAGS, unparsed_args = p.parse_known_args()

    if len(unparsed_args):
        print("Warning: unknow arguments {}".format(unparsed_args))
    main(FLAGS=FLAGS)
