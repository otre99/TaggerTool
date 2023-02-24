import json
import argparse
import os
import xml.etree.ElementTree as ET


def create_parser():
    parser = argparse.ArgumentParser(
        description="Convert annotations in PASCAL format to TaggerTool native format")
    parser.add_argument('--pascal_ann_folder', type=str,
                        help="Pascal annotation folder")
    parser.add_argument('--dst_folder', type=str,
                        help="Destination forlder")
    # parser.add_argument('--only_bboxes', action='store_true',
    #                     help='Process only bboxes')
    parser.add_argument('--selected_labels', nargs='+',
                        help='Process only certain labels', default=[])
    return parser


def read_pascal_xml_content(xml_file: str, valid_labels: set = set()):
    tree = ET.parse(xml_file)
    root = tree.getroot()

    filename = root.find('filename').text
    width = int(root.find('size/width').text)
    height = int(root.find('size/height').text)

    bboxes = []
    for boxes in root.iter('object'):
        ymin, xmin, ymax, xmax = None, None, None, None

        label = boxes.find("name").text

        if valid_labels != set() and label not in valid_labels:
            continue

        xmin = float(boxes.find("bndbox/xmin").text)
        ymin = float(boxes.find("bndbox/ymin").text)
        xmax = float(boxes.find("bndbox/xmax").text)
        ymax = float(boxes.find("bndbox/ymax").text)
        difficult = int(boxes.find("difficult").text)

        truncated = boxes.find("truncated")
        truncated = 0 if truncated is None else int(truncated.text)

        bboxes.append(
            {
                "label": label,
                "x1": xmin,
                "y1": ymin,
                "x2": xmax,
                "y2": ymax,
                "difficult": difficult,
                "truncated": truncated
            }
        )
    _, ext = os.path.splitext(filename)
    image_format = ext[1:].lower()

    return {
        "bboxes": bboxes,
        "filename": filename,
        "width": width,
        "height": height,
        "format": image_format
    }

def load_pascal_annotations(ann_foler: str):
    flist = os.listdir(ann_foler)
    annotations = []
    for fname in flist:
        annotations.append(
            read_pascal_xml_content(xml_file=os.path.join(ann_foler, fname))
        )
    return annotations


def empty_tagger_tool_ann():
    return {
        "bboxes": [],
        "description": "",
        "image_h": -1,
        "image_name": "",
        "image_w": -1,
        "label": "",
        "line_strips": [],
        "lines": [],
        "points": [],
        "polygons": [],
        "tags": []
    }


def main(FLAGS):
    dst_folder = FLAGS.dst_folder
    os.system(f"mkdir -p {dst_folder}")

    print("Load PASCAL annotations...", flush=True)
    pascal_db = load_pascal_annotations(FLAGS.pascal_ann_folder)
    print("Done!")

    print("Writing annotations in TaggerTool format...", flush=True)
    n = len(pascal_db)
    k = max(1, n//100)
    for i, pascal_ann in enumerate(pascal_db):

        filename = pascal_ann['filename']
        base_name, _ = os.path.splitext(filename)

        output_ann = empty_tagger_tool_ann()
        output_ann['image_name'] = filename
        output_ann['image_w'] = pascal_ann['width']
        output_ann['image_h'] = pascal_ann['height']

        selected_labels = set(FLAGS.selected_labels)
        for pascal_box in pascal_ann['bboxes']:

            label = pascal_box['label']
            if len(selected_labels) > 0 and label not in selected_labels:
                continue

            x1, y1 = pascal_box['x1'], pascal_box['y1']
            x2, y2 = pascal_box['x2'], pascal_box['y2']
            output_ann['bboxes'].append(
                {
                    "label": label,
                    "occluded": False,
                    "truncated": pascal_box['truncated'],
                    "crowded": False, #pascal_box['difficult'],
                    "x1": x1,
                    "y1": y1,
                    "x2": x2,
                    "y2": y2
                }
            )

            #if FLAGS.only_bboxes:
            #    continue
            # TODO(otre99)

        with open(os.path.join(dst_folder, base_name+'.json'), 'wt') as ofile:
            json.dump(obj=output_ann, fp=ofile)

        if i % k == 0:
            print(f"  Progress: {(i+1)*100/n:5.2f}%", end='\r')
    print("                                \rDone!")


if __name__ == '__main__':
    p = create_parser()
    FLAGS, unparsed_args = p.parse_known_args()
    if len(unparsed_args):
        print("Warning: unknow arguments {}".format(unparsed_args))
    main(FLAGS=FLAGS)
