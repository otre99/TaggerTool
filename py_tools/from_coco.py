import json
import argparse
import os
from collections import defaultdict

def main(FLAGS):
    with open(FLAGS.coco_json, 'r') as f:
        coco_data = json.load(f)

    images = {img['id']: img for img in coco_data['images']}
    annotations = defaultdict(list)
    for ann in coco_data['annotations']:
        annotations[ann['image_id']].append(ann)

    output_data = []
    for img_id, img_info in images.items():
        img_entry = {
            'file_name': img_info['file_name'],
            'height': img_info['height'],
            'width': img_info['width'],
            'annotations': []
        }
        for ann in annotations[img_id]:
            ann_entry = {
                'bbox': ann['bbox'],
                'category_id': ann['category_id'],
                'iscrowd': ann.get('iscrowd', 0)
            }
            img_entry['annotations'].append(ann_entry)
        output_data.append(img_entry)

    with open(FLAGS.output_json, 'w') as f:
        json.dump(output_data, f, indent=4)


def create_parser():
    parser = argparse.ArgumentParser()
    parser.add_argument('--coco_json', type=str, required=True, help='Path to the input COCO JSON file')
    parser.add_argument('--output_folder', type=str, required=True, help='Folder to save the output JSON annotations files')
    return parser


if __name__ == '__main__':
    p = create_parser()
    FLAGS, unparsed_args = p.parse_known_args()
    if len(unparsed_args):
        print("Warning: unknow arguments {}".format(unparsed_args))
    main(FLAGS=FLAGS)
