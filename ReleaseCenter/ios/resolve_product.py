#!/usr/bin/env python3
"""Resolve only owner-reviewed, immutable sources. Never accept caller-supplied repositories or refs."""
import json, os, pathlib, re, sys
catalog=json.loads(pathlib.Path("ReleaseCenter/ios/catalog.json").read_text())
key=os.environ["PRODUCT"]
build=os.environ["BUILD_NUMBER"]
if not re.fullmatch(r"[a-z0-9][a-z0-9-]{0,48}",key) or key not in catalog["products"]:
    sys.exit("Unknown iOS product")
if not re.fullmatch(r"[1-9][0-9]{0,3}",build):
    sys.exit("Invalid Apple build number")
p=catalog["products"][key]
rules={"repository":r"BSM1973/[A-Za-z0-9_.-]+","sha":r"[0-9a-f]{40}","target":r"[A-Za-z][A-Za-z0-9_]*","version":r"[0-9]+\.[0-9]+\.[0-9]+","bundle_id":r"com\.libertymusicsoftware\.[a-zA-Z0-9.-]+","environment":r"ios-[a-z0-9-]+","minimum_ios":r"[0-9]+\.[0-9]+"}
for k,pattern in rules.items():
    if not re.fullmatch(pattern,p[k]): sys.exit("Invalid catalog field: "+k)
if p["environment"]!="ios-"+key: sys.exit("Incorrect product signing environment")
outputs={"repo":p["repository"],"sha":p["sha"],"product":key,"target":p["target"],"version":p["version"],"bundle":p["bundle_id"],"environment":p["environment"],"minimum_ios":p["minimum_ios"]}
with open(os.environ["GITHUB_OUTPUT"],"a") as f:
    for k,v in outputs.items(): f.write(k+"="+v+"\n")
print("Validated product",key,p["version"],p["sha"])
