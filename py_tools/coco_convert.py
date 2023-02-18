import json
import argparse
import os
from collections import defaultdict


def create_parser():
    parser = argparse.ArgumentParser(
        description="Convert annotations in COCO format to TaggerTool native format")
    parser.add_argument('--coco_json_file', type=str,
                        help="COCO annotation format")
    parser.add_argument('--dst_folder', type=str,
                        help="Destination forlder")
    parser.add_argument('--only_bboxes', action='store_true',
                        help='Process only bboxes')
    parser.add_argument('--selected_labels', nargs='+',
                        help='Process only certain labels', default=[])
    return parser


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

    print("Load COCO annotations...", flush=True)
    coco_db = json.load(open(FLAGS.coco_json_file))
    id2img = {img['id']: img for img in coco_db['images']}
    id2label = {cat['id']: cat['name'] for cat in coco_db['categories']}
    imgid2anns = defaultdict(lambda: [])
    for ann in coco_db['annotations']:
        imgid2anns[ann['image_id']].append(ann)
    print("Done!")

    print("Writing annotations in TaggerTool format...", flush=True)
    n = len(id2img)
    k = max(1, n//100)
    for i, (image_id, image_info) in enumerate(id2img.items()):

        filename = image_info['file_name']
        base_name, _ = os.path.splitext(filename)

        output_ann = empty_tagger_tool_ann()
        output_ann['image_name'] = filename
        output_ann['image_w'] = image_info['width']
        output_ann['image_h'] = image_info['height']

        selected_labels = set(FLAGS.selected_labels)
        for coco_ann in imgid2anns[image_id]:

            label = id2label[coco_ann['category_id']]
            if len(selected_labels) > 0 and label not in selected_labels:
                continue

            x1, y1, w, h = coco_ann['bbox']
            output_ann['bboxes'].append(
                {
                    "label": label,
                    "occluded": False,
                    "truncated": False,
                    "crowded": bool(coco_ann["iscrowd"]),
                    "x1": x1,
                    "y1": y1,
                    "x2": x1+w,
                    "y2": y1+h
                }
            )

            if FLAGS.only_bboxes:
                continue

            segms = coco_ann['segmentation'] if 'segmentation' in coco_ann.keys(
            ) else None
            if segms:
                for seg in segms:
                    output_ann['polygons'].append(
                        {
                            'label': label,
                            'x_coords': seg[::2],
                            'y_coords': seg[1::2]
                        }
                    )

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
