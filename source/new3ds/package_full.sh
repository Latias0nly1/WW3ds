#!/usr/bin/env bash
set -euo pipefail
here=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
iso=${WW3DS_ISO:-"$HOME/Desktop/tww/orig/GZLE01/GZLE01.iso"}
elf=$here/ww3ds.elf
cia=$here/cia
stage=$cia/romfs-full
[[ -s "$iso" && -s "$elf" && -s "$cia/banner.png" && -s "$cia/hover.wav" && -s "$cia/icon.png" && -x "$cia/kit/makerom" ]] || {
  echo "Need the expanded GZLE01.iso, ww3ds.elf, banner, icon, hover sound, and cia/kit/makerom" >&2
  exit 1
}
python3 - "$iso" <<'PY'
import os, sys
path = sys.argv[1]
with open(path, "rb") as f:
    header = f.read(0x20)
if header[:4] != b"GZLE" or header[0x1C:0x20] != bytes.fromhex("C2339F3D") or os.path.getsize(path) != 1459978240:
    raise SystemExit("ISO is not the expanded GZLE01 revision 0 image")
PY
rm -rf "$stage"
mkdir -p "$stage"
ln "$iso" "$stage/GZLE01.iso" 2>/dev/null || cp -c "$iso" "$stage/GZLE01.iso" 2>/dev/null || cp "$iso" "$stage/GZLE01.iso"
python3 - "$cia/ww3ds.rsf" "$cia/ww3ds-full.rsf" "$stage" <<'PY'
import pathlib, sys
src, dst, root = sys.argv[1:]
text = pathlib.Path(src).read_text()
text = text.replace("UniqueId                : 0xff2d7", "UniqueId                : 0xff2d8", 1)
text = text.replace("ProductCode             : CTR-P-WW3D", "ProductCode             : CTR-P-WW3F", 1)
text = text.replace("EnableCompress          : true", "EnableCompress          : false", 1)
old = "RomFs:\n"
new = f"RomFs:\n  RootPath: {root}\n"
if old not in text:
    raise SystemExit("RSF has no RomFs section")
pathlib.Path(dst).write_text(text.replace(old, new, 1))
PY
cp -f "$elf" "$cia/ww3ds.full.elf"
"$cia/kit/bannertool" makebanner -i "$cia/banner.png" -a "$cia/hover.wav" -o "$cia/banner.bnr"
"$cia/kit/bannertool" makesmdh -s 'ww3ds' -l 'ww3ds' -p ' ' -i "$cia/icon.png" -o "$cia/icon.smdh"
"$cia/kit/makerom" -f cia -o "$cia/ww3ds-full.cia.new" -elf "$cia/ww3ds.full.elf" -rsf "$cia/ww3ds-full.rsf" \
  -icon "$cia/icon.smdh" -banner "$cia/banner.bnr" -target t
"$cia/kit/ctrtool" --exheader="$cia/ww3ds-full.exheader.bin" "$cia/ww3ds-full.cia.new" > "$cia/ww3ds-full.inspection.txt"
grep -Eq 'Title id:[[:space:]]+000400000ff2d800' "$cia/ww3ds-full.inspection.txt"
grep -Eq 'System mode \(New3DS\):.*124MB' "$cia/ww3ds-full.inspection.txt"
python3 - "$cia/ww3ds-full.cia.new" <<'PY'
import os, sys
size = os.path.getsize(sys.argv[1])
if size < 1400000000:
    raise SystemExit(f"CIA is {size} bytes; the disc was not packed")
print(f"full CIA {size} bytes")
PY
mv "$cia/ww3ds-full.cia.new" "$cia/ww3ds-full.cia"
rm -rf "$stage" "$cia/ww3ds.full.elf" "$cia/ww3ds-full.rsf"
printf 'Packaged %s (disc is inside; do not share this file)\n' "$cia/ww3ds-full.cia"
