"""Release Center smoke tests, no private Apple data required."""
from pathlib import Path
import json
import re
root=Path(__file__).resolve().parents[1]
catalog=json.loads((root/"catalog.json").read_text(encoding="utf-8"))
wf=(root.parent/".github/workflows/liberty-release-center.yml").read_text(encoding="utf-8")
mac=(root/"macos/sign_notarize.sh").read_text(encoding="utf-8")
windows=(root/"windows/Liberty.iss").read_text(encoding="utf-8")
assert set(catalog)=={"echo-86","pedal-legends","organix"}
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
assert catalog["organix"]["repository"] == "BSM1973/Liberty-Organix"
assert catalog["organix"]["product_name"] == "Organix"
assert catalog["organix"]["target"] == "Organix"
assert catalog["organix"]["version"] == "1.3.5"
assert 'on: ' in wf or "  push:" in wf
assert "Configure Organix universal AU/VST3" in wf
assert "Configure Organix Windows x64 VST3 + integration tests" in wf
assert "RELEASE GATE — verify downloaded Windows installer EXE" in wf
assert "needs.select.outputs.slug != 'organix'" in wf  # only excludes generic CMake profile, not the Windows job
window_section=wf.split("  windows-build:",1)[1]
assert "    if: needs.select.outputs.slug != 'organix'" not in window_section
assert 'if: needs.select.outputs.slug == \'organix\'' in window_section
assert "inputs.plugin || 'organix'" in wf
print("PASS: Release Center catalog, isolation and notarization gates")
