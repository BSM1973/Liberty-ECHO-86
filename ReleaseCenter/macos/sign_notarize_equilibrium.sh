#!/usr/bin/env bash
# Isolated signing job: only trusted ReleaseCenter scripts run with secrets.
# No source checkout or project compilation is executed in this job.
set -euo pipefail
: "${LIBERTY_PRODUCT_NAME:?}"
: "${LIBERTY_SLUG:?}"
: "${LIBERTY_VERSION:?}"
for key in APPLE_DEVELOPER_ID_APPLICATION_P12_BASE64 \
  APPLE_DEVELOPER_ID_INSTALLER_P12_BASE64 APPLE_P12_PASSWORD \
  APPLE_NOTARY_APPLE_ID APPLE_NOTARY_APP_PASSWORD APPLE_NOTARY_TEAM_ID; do
  if [[ -z "${!key:-}" ]]; then
    echo "::error::Signing secret $key is not present in the ECHO 86 repository."
    exit 1
  fi
done
kc="$RUNNER_TEMP/liberty-release-center.keychain-db"
pass="$(/usr/bin/uuidgen)"
app="$RUNNER_TEMP/liberty-app.p12"
installer="$RUNNER_TEMP/liberty-installer.p12"
trap 'rm -f "$app" "$installer"' EXIT
/usr/bin/security create-keychain -p "$pass" "$kc"
/usr/bin/security set-keychain-settings -lut 21600 "$kc"
/usr/bin/security unlock-keychain -p "$pass" "$kc"
/usr/bin/security list-keychains -d user -s "$kc"
/usr/bin/security default-keychain -s "$kc"
printf %s "$APPLE_DEVELOPER_ID_APPLICATION_P12_BASE64" | /usr/bin/base64 -D > "$app"
printf %s "$APPLE_DEVELOPER_ID_INSTALLER_P12_BASE64" | /usr/bin/base64 -D > "$installer"
/usr/bin/security import "$app" -k "$kc" -P "$APPLE_P12_PASSWORD" -T /usr/bin/codesign
/usr/bin/security import "$installer" -k "$kc" -P "$APPLE_P12_PASSWORD" -T /usr/bin/pkgbuild -T /usr/bin/productbuild
/usr/bin/security set-key-partition-list -S apple-tool:,apple: -s -k "$pass" "$kc"
app_identity="$(/usr/bin/security find-identity -v -p codesigning "$kc" | /usr/bin/awk -F '"' '/Developer ID Application/{print $2; exit}')"
installer_identity="$(/usr/bin/security find-identity -v "$kc" | /usr/bin/awk -F '"' '/Developer ID Installer/{print $2; exit}')"
if [[ -z "$app_identity" || -z "$installer_identity" ]]; then
  echo "::error::Developer ID Application / Installer identity unavailable."
  exit 1
fi

input="$GITHUB_WORKSPACE/incoming"
root="$RUNNER_TEMP/liberty-pkg"
mkdir -p "$root/Library/Audio/Plug-Ins/Components" "$root/Library/Audio/Plug-Ins/VST3" "$root/Applications"
declare -a paths=()
for spec in "au.zip:component:Components" "vst3.zip:vst3:VST3" "app.zip:app:Applications"; do
  IFS=: read -r zip ext folder <<< "$spec"
  dir="$RUNNER_TEMP/unpack-$ext"
  mkdir -p "$dir"
  /usr/bin/ditto -x -k "$input/$zip" "$dir"
  source="$dir/${LIBERTY_PRODUCT_NAME}.$ext"
  test -d "$source" || { echo "::error::Missing signed source $source"; exit 1; }
  /usr/bin/codesign --force --deep --options runtime --timestamp \
    --sign "$app_identity" "$source"
  /usr/bin/codesign --verify --deep --strict --verbose=2 "$source"
  signature="$(/usr/bin/codesign --display --verbose=2 "$source" 2>&1)"
  grep -Fq 'Authority=Developer ID Application:' <<< "$signature" || {
    echo "::error::Invalid Developer ID Application signature on $source"; exit 1;
  }
  if [[ "$ext" == "component" ]]; then
    destination="$root/Library/Audio/Plug-Ins/Components/$(basename "$source")"
  elif [[ "$ext" == "app" ]]; then
    destination="$root/Applications/$(basename "$source")"
  else
    destination="$root/Library/Audio/Plug-Ins/VST3/$(basename "$source")"
  fi
  /usr/bin/ditto "$source" "$destination"
  /usr/bin/codesign --verify --deep --strict --verbose=2 "$destination"
  paths+=("$destination")
done
out="$GITHUB_WORKSPACE/release-output"
mkdir -p "$out"
package="$out/LIBERTY-${LIBERTY_SLUG}-macOS-Universal-v${LIBERTY_VERSION}-SIGNED-NOTARIZED.pkg"
/usr/bin/pkgbuild --root "$root" \
  --identifier "com.libertymusicsoftware.${LIBERTY_SLUG}.installer" \
  --version "$LIBERTY_VERSION" --install-location / \
  --sign "$installer_identity" "$package"
/usr/sbin/pkgutil --check-signature "$package"
payload="$(/usr/sbin/pkgutil --payload-files "$package")"
grep -Fq "Library/Audio/Plug-Ins/Components/${LIBERTY_PRODUCT_NAME}.component" <<< "$payload" || exit 1
grep -Fq "Library/Audio/Plug-Ins/VST3/${LIBERTY_PRODUCT_NAME}.vst3" <<< "$payload" || exit 1
grep -Fq "Applications/${LIBERTY_PRODUCT_NAME}.app" <<< "$payload" || exit 1

response="$RUNNER_TEMP/liberty-notarization.json"
/usr/bin/xcrun notarytool submit "$package" \
  --apple-id "$APPLE_NOTARY_APPLE_ID" \
  --password "$APPLE_NOTARY_APP_PASSWORD" \
  --team-id "$APPLE_NOTARY_TEAM_ID" \
  --wait --output-format json > "$response"
status="$(/usr/bin/python3 -c 'import json,sys;print(json.load(open(sys.argv[1])).get("status","Unknown"))' "$response")"
echo "Apple notary status: $status"
if [[ "$status" != "Accepted" ]]; then
  echo "::error::Apple did not accept notarization. No installer is published."
  exit 1
fi
/usr/bin/xcrun stapler staple "$package"
/usr/bin/xcrun stapler validate "$package"
/usr/sbin/spctl --assess --type install --verbose=2 "$package"
/usr/sbin/pkgutil --check-signature "$package"
test -s "$package"
echo "SIGNED, NOTARIZED and STAPLED installer verified: $(basename "$package")"
