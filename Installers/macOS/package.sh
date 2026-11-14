#!/usr/bin/env bash
# ECHO 86 macOS distribution; does NOT modify audio/plugin sources.
set -euo pipefail

: "$RUNNER_TEMP"
: "$GITHUB_WORKSPACE"
stage="$RUNNER_TEMP/ECHO86-macOS"
payload="$RUNNER_TEMP/echo86-pkg-root"
package="$GITHUB_WORKSPACE/ECHO86-macOS-Installer.pkg"
version="0.1.0"

test -d "$stage/Components/ECHO 86.component" || { echo "::error::AU missing"; exit 1; }
test -d "$stage/VST3/ECHO 86.vst3" || { echo "::error::VST3 missing"; exit 1; }
test -d "$stage/Standalone/ECHO 86.app" || { echo "::error::Standalone missing"; exit 1; }

mkdir -p "$payload/Library/Audio/Plug-Ins/Components"
mkdir -p "$payload/Library/Audio/Plug-Ins/VST3"
mkdir -p "$payload/Applications"

# ditto preserves bundle permissions, symlinks and extended metadata.
ditto "$stage/Components/ECHO 86.component" "$payload/Library/Audio/Plug-Ins/Components/ECHO 86.component"
ditto "$stage/VST3/ECHO 86.vst3" "$payload/Library/Audio/Plug-Ins/VST3/ECHO 86.vst3"
ditto "$stage/Standalone/ECHO 86.app" "$payload/Applications/ECHO 86.app"

for path in "$payload/Library/Audio/Plug-Ins/Components/ECHO 86.component" \
            "$payload/Library/Audio/Plug-Ins/VST3/ECHO 86.vst3" \
            "$payload/Applications/ECHO 86.app"; do
  codesign --verify --deep --strict --verbose=2 "$path"
done

if [[ "$ECHO86_DEVELOPER_ID_READY" == "true" ]]; then
  : "$ECHO86_INSTALLER_IDENTITY"
  pkgbuild --root "$payload" \
    --identifier com.libertymusicsoftware.echo86.installer \
    --version "$version" --install-location / \
    --sign "$ECHO86_INSTALLER_IDENTITY" \
    "$package"
  pkgutil --check-signature "$package"
else
  echo "::warning::Apple Developer ID secrets not configured. Creating UNSIGNED TEST .pkg."
  pkgbuild --root "$payload" \
    --identifier com.libertymusicsoftware.echo86.installer \
    --version "$version" --install-location / \
    "$package"
fi

test -s "$package" || { echo "::error::Installer .pkg missing"; exit 1; }
if [[ "$ECHO86_DEVELOPER_ID_READY" == "true" ]]; then
  pkgutil --check-signature "$package"
fi

# Optional Apple notarization (never mark an ad-hoc signed package as notarized).
if [[ "$ECHO86_DEVELOPER_ID_READY" == "true" ]]; then
  count=0
  if [[ -n "$APPLE_NOTARY_API_KEY_P8_BASE64" ]]; then count=$((count+1)); fi
  if [[ -n "$APPLE_NOTARY_KEY_ID" ]]; then count=$((count+1)); fi
  if [[ -n "$APPLE_NOTARY_ISSUER_ID" ]]; then count=$((count+1)); fi
  if [[ "$count" -ne 0 && "$count" -ne 3 ]]; then
    echo "::error::Notarization needs all 3 APPLE_NOTARY_* secrets (or none)."
    exit 1
  fi
  if [[ "$count" -eq 3 ]]; then
    key="$RUNNER_TEMP/AuthKey_ECHO86.p8"
    printf %s "$APPLE_NOTARY_API_KEY_P8_BASE64" | base64 -D > "$key"
    chmod 600 "$key"
    echo "Submitting ECHO 86 installer to Apple notarization service..."
    xcrun notarytool submit "$package" \
      --key "$key" \
      --key-id "$APPLE_NOTARY_KEY_ID" \
      --issuer "$APPLE_NOTARY_ISSUER_ID" \
      --wait --output-format json > "$RUNNER_TEMP/echo86-notarization.json"
    /usr/bin/python3 -c 'import json,sys; x=json.load(open(sys.argv[1])); print("Notary status:",x.get("status")); sys.exit(0 if x.get("status")=="Accepted" else 1)' \
       "$RUNNER_TEMP/echo86-notarization.json"
    xcrun stapler staple "$package"
    xcrun stapler validate "$package"
    spctl --assess --type install --verbose=2 "$package"
    echo "ECHO86_NOTARIZED=true" >> "$GITHUB_ENV"
    echo "Apple Developer ID signed + Apple notarized + stapled .pkg validated."
  else
    echo "::warning::Developer ID installer is signed, but Apple notarization secrets are not configured."
  fi
fi
