#!/usr/bin/env bash
set -euo pipefail
here=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
cd "$here/cia"
exec 8>.package.lock
flock 8
for tool in makerom bannertool ctrtool; do
  [[ -x kit/$tool ]] || { echo "Missing cia/kit/$tool" >&2; exit 1; }
done
python3 - "$here/ww3ds.elf" "$PWD/ww3ds.snapshot.elf" <<'PY'
from pathlib import Path
import os,sys,hashlib,json,time
source,target=map(Path,sys.argv[1:]);temp=target.with_suffix('.tmp')
for attempt in range(5):
    with source.open('rb') as f:
        before=os.fstat(f.fileno());data=f.read();after=os.fstat(f.fileno())
    current=source.stat()
    def signature(s):return s.st_ino,s.st_size,s.st_mtime_ns
    if signature(before)==signature(after)==signature(current) and len(data)==before.st_size and data[:4]==b'\x7fELF':
        temp.write_bytes(data);temp.replace(target)
        Path('snapshot.json').write_text(json.dumps({'source':source.name,'bytes':len(data),'source_mtime_ns':before.st_mtime_ns,'sha256':hashlib.sha256(data).hexdigest()},indent=2)+'\n')
        break
    time.sleep(.2)
else:raise SystemExit('ELF changed during snapshot; rerun packaging once the current link finishes')
PY
audio="$here/cia/hover.wav"
[[ -s banner.png && -s "$audio" ]] || { echo "Missing presentation assets" >&2; exit 1; }
kit/bannertool makebanner -i banner.png -a "$audio" -o banner.bnr
kit/bannertool makesmdh -s 'ww3ds' -l 'ww3ds' -p ' ' -i icon.png -o icon.smdh
kit/makerom -f cia -o ww3ds.cia.new -elf ww3ds.snapshot.elf -rsf ww3ds.rsf -icon icon.smdh -banner banner.bnr -target t
kit/ctrtool --exheader=ww3ds.exheader.bin ww3ds.cia.new > inspection.txt 2>&1
if ! grep -E 'System mode \(New3DS\):.*124MB' inspection.txt >/dev/null; then
  echo 'Packaged exheader did not report New 3DS 124MB mode; see cia/inspection.txt' >&2
  exit 1
fi
mv -- ww3ds.cia.new ww3ds.cia
sha256sum ww3ds.cia > ww3ds.cia.sha256
printf 'Packaged %s/cia/ww3ds.cia (ISO and material bundles remain external)\n' "$here"
