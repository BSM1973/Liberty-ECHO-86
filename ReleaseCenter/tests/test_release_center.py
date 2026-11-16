"""Release Center smoke tests, no private Apple data required."""
from pathlib import Path
import json
import re
root=Path(__file__).resolve().parents[1]
catalog=json.loads((root/"catalog.json").read_text(encoding="utf-8"))
wf=(root.parent/".github/workflows/liberty-release-center.yml").read_text(encoding="utf-8")
mac=(root/"macos/sign_notarize.sh").read_text(encoding="utf-8")
windows=(root/"windows/Liberty.iss").read_text(encoding="utf-8")
assert set(catalog)=={"echo-86","pedal-legends"}
for slug,info in catalog.items():
    assert info["repository"].startswith("BSM1973/")
    assert re.fullmatch(r"[0-9]+\.[0-9]+\.[0-9]+",info["version"])
    assert slug in wf
assert "actions/download-artifact@v4" in wf
assert "needs: [select, mac-build]" in wf
assert "notarytool submit" in mac and "stapler staple" in mac
assert 'if [[ "$status" != "Accepted" ]]' in mac
assert "APPLE_NOTARY_APP_PASSWORD" in wf and "APPLE_P12_PASSWORD" in wf
assert "{commoncf64}\\VST3" in windows
assert "UNSIGNED" in wf and "SIGNED-NOTARIZED" in wf
print("PASS: Release Center catalog, isolation and notarization gates")
