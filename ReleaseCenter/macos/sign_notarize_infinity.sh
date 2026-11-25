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

# Standalone ARM64 only. No AU/VST3 installation paths.
input="$GITHUB_WORKSPACE/incoming"
root="$RUNNER_TEMP/infinity-pkg"
unpack="$RUNNER_TEMP/infinity-unpack"
mkdir -p "$root/Applications" "$unpack"
/usr/bin/ditto -x -k "$input/Liberty-Infinity-Black-Gold-Mac-Test-arm64.zip" "$unpack"
source="$unpack/Liberty Infinity MPC Test.app"
test -d "$source"
test "$(/usr/libexec/PlistBuddy -c 'Print CFBundleShortVersionString' "$source/Contents/Info.plist")" = "$LIBERTY_VERSION"
test "$(/usr/bin/lipo -archs "$source/Contents/MacOS/Infinity")" = arm64
/usr/bin/codesign --force --options runtime --timestamp --sign "$app_identity" "$source"
/usr/bin/codesign --verify --deep --strict --verbose=2 "$source"
signature="$(/usr/bin/codesign --display --verbose=2 "$source" 2>&1)"
grep -Fq 'Authority=Developer ID Application:' <<< "$signature"
/usr/bin/ditto "$source" "$root/Applications/Liberty Infinity MPC Test.app"
/usr/bin/codesign --verify --deep --strict "$root/Applications/Liberty Infinity MPC Test.app"
out="$GITHUB_WORKSPACE/release-output"
mkdir -p "$out"
package="$out/LIBERTY-Infinity-macOS-ARM64-v$LIBERTY_VERSION-RC-SIGNED-NOTARIZED.pkg"
/usr/bin/pkgbuild --root "$root" --identifier com.libertymusicsoftware.infinitympctest.installer --version "$LIBERTY_VERSION" --install-location / --sign "$installer_identity" "$package"
/usr/sbin/pkgutil --check-signature "$package"
payload="$(/usr/sbin/pkgutil --payload-files "$package")"
grep -Fq "Applications/Liberty Infinity MPC Test.app/Contents/MacOS/Infinity" <<< "$payload"
response="$RUNNER_TEMP/infinity-notarization.json"
/usr/bin/xcrun notarytool submit "$package" --apple-id "$APPLE_NOTARY_APPLE_ID" --password "$APPLE_NOTARY_APP_PASSWORD" --team-id "$APPLE_NOTARY_TEAM_ID" --wait --output-format json > "$response"
status="$(/usr/bin/python3 -c 'import json,sys;print(json.load(open(sys.argv[1])).get("status","Unknown"))' "$response")"
echo "Apple notary status: $status"
if [[ "$status" != Accepted ]]; then
  echo "::error::Notarization rejected or unavailable. No installer published."
  exit 1
fi
/usr/bin/xcrun stapler staple "$package"
/usr/bin/xcrun stapler validate "$package"
/usr/sbin/spctl --assess --type install --verbose=2 "$package"
/usr/sbin/pkgutil --check-signature "$package"
test -s "$package"
(cd "$out"; /usr/bin/shasum -a 256 "$(basename "$package")" > SHA256SUMS.txt)
echo "PASS signed, notarized, stapled RC package; hardware/listening QC remains pending."
