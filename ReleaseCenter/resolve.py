#!/usr/bin/env python3
"""Resolve a trusted Liberty plugin from a fixed catalog.

Only the local operator can add a repository to the catalog; Actions inputs
cannot introduce an arbitrary repository or shell syntax into signing jobs.
"""
import json
import os
import re
from pathlib import Path

root = Path(__file__).resolve().parents[1]
catalog = json.loads((root / "catalog.json").read_text(encoding="utf-8"))
slug = os.environ.get("LIBERTY_PLUGIN", "pedal-legends").strip() or "pedal-legends"
override = os.environ.get("LIBERTY_VERSION", "").strip()
if slug not in catalog:
    raise SystemExit(f"Unknown registered plugin: {slug!r}")
item = catalog[slug]
version = override or item["version"]
if not re.fullmatch(r"[0-9]+\.[0-9]+\.[0-9]+(?:[-.][A-Za-z0-9]+)*", version):
    raise SystemExit("Invalid version: use numeric semver, e.g. 0.6.1")
if not re.fullmatch(r"BSM1973/[A-Za-z0-9._-]+", item["repository"]):
    raise SystemExit("Repository must belong to the curated BSM1973 allowlist")
if not re.fullmatch(r"[A-Za-z0-9_-]+", slug):
    raise SystemExit("Unsafe slug")
for key in ("target", "product_name", "cmake_source"):
    if "\n" in item[key] or "\r" in item[key]:
        raise SystemExit(f"Invalid new line in {key}")
values = {
    "slug": slug,
    "repository": item["repository"],
    "product_name": item["product_name"],
    "source_dir": item["cmake_source"],
    "target": item["target"],
    "version": version,
    "juce_checkout": str(item["juce_checkout"]).lower(),
    "standalone": str(item["standalone"]).lower()
}
output = Path(os.environ["GITHUB_OUTPUT"])
with output.open("a", encoding="utf-8") as out:
    for key, value in values.items():
        out.write(f"{key}={value}\n")
print("Release selection:", slug, "version", version, "repository", item["repository"])
