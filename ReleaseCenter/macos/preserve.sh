#!/usr/bin/env bash
# Run in the unprivileged build job, NEVER with Apple credentials.
set -euo pipefail
: "${LIBERTY_PRODUCT_NAME:?}"
: "${LIBERTY_TARGET:?}"
root="$GITHUB_WORKSPACE/build/${LIBERTY_TARGET}_artefacts/Release"
out="$GITHUB_WORKSPACE/exported-mac"
mkdir -p "$out"
for spec in "AU:component:au.zip" "VST3:vst3:vst3.zip"; do
  IFS=: read -r format ext archive <<< "$spec"
  bundle="$root/$format/${LIBERTY_PRODUCT_NAME}.$ext"
  test -d "$bundle" || { echo "::error::Missing $bundle"; exit 1; }
  file="$(find "$bundle/Contents/MacOS" -type f -maxdepth 1 -print -quit)"
  test -n "$file" || { echo "::error::Missing Mach-O inside $bundle"; exit 1; }
  archs="$(/usr/bin/lipo -archs "$file")"
  echo "$bundle : $archs"
  for arch in arm64 x86_64; do
    if [[ " $archs " != *" $arch "* ]]; then
      echo "::error::$arch missing from $bundle"; exit 1
    fi
  done
  # ditto ZIP retains permissions, symlinks and bundle structure before signing.
  /usr/bin/ditto -c -k --keepParent "$bundle" "$out/$archive"
  /usr/bin/unzip -tq "$out/$archive"
done
echo "Unsigned universal AU+VST3 build artifacts verified."
