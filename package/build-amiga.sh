#!/bin/sh
# The release archive, for Cradle's stove_build (target package): Aleph One
# built with amiga/Makefile.amigaos on the stove's GFOpenGPU developer kit
# (SDL 2, SDL2_image, SDL2_ttf, SDL2_net), with zlib 1.3.1 fetched at its
# pinned commit and Boost 1.83.0's headers from its release archive (checked
# against its SHA-256); the Makefile's package target makes the drawer, and
# amiga/tools/make_package.py makes alephone-VERSION-68040.lha with its
# .sha256 and .readme: the AlephOne drawer (the program and its GlowIcons,
# ReadMe, Data-ReadMe, the GPL, MML/, Plugins/Default_Theme/ and an empty
# Scenario/), its icon, and beside them the Amiga Installer script
# Install-AlephOne and its icon (package/installer/, made by amicradle
# scripts/port_installer.py from package/installer.json). No Marathon data
# is packaged. OUT also gets the drawer as it unpacks, as AlephOne.tar (for
# a lab), and package.json. VERSION is package/VERSION. Nothing is
# published from here.
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu

HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$HERE/.." && pwd)
OUT=${OUT:-"$ROOT/build-amiga-package"}
VERSION=$(cat "$HERE/VERSION")
mkdir -p "$OUT"
OUT=$(CDPATH= cd -- "$OUT" && pwd)
BUILD=$(mktemp -d "${TMPDIR:-/tmp}/alephone-package.XXXXXX")
trap 'rm -rf "$BUILD"' EXIT INT TERM
cd "$ROOT"

# A stove's compiler is in its prefix/; the GFOpenGPU kit is in the prefix's
# m68k-amigaos/ (include/SDL2, lib/libSDL2.a).
if [ ! -x "$STOVE/bin/m68k-amigaos-gcc" ] && [ -x "$STOVE/prefix/bin/m68k-amigaos-gcc" ]; then
	STOVE="$STOVE/prefix"
fi
SDL2KIT="$STOVE/m68k-amigaos"
for l in SDL2 SDL2_image SDL2_ttf SDL2_net; do
	[ -f "$SDL2KIT/lib/lib$l.a" ] || { echo "the stove has no GFOpenGPU kit (lib$l.a)"; exit 1; }
done
if [ -z "${NDK:-}" ]; then
	git clone -q --depth 1 https://github.com/DalsinAI/gfopensdk "$BUILD/gfopensdk"
	NDK="$BUILD/gfopensdk/OpenNDK/Include_H"
fi
git init -q "$BUILD/zlib"
git -C "$BUILD/zlib" fetch -q --depth 1 https://github.com/madler/zlib 51b7f2abdade71cd9bb0e7a373ef2610ec6f9daf	# v1.3.1
git -C "$BUILD/zlib" checkout -q FETCH_HEAD

# Boost's headers: the release archive keeps each library's in
# libs/NAME/include/boost; one boost/ of them all is what -I$(BOOST) wants.
# Kept in .deps/ between runs (the archive is 94 MB).
BOOST="$ROOT/.deps/boost_1_83_0"
if [ ! -f "$BOOST/boost/version.hpp" ]; then
	mkdir -p "$ROOT/.deps"
	TXZ="$ROOT/.deps/boost-1.83.0.tar.xz"
	[ -f "$TXZ" ] || curl -sSLfo "$TXZ" https://github.com/boostorg/boost/releases/download/boost-1.83.0/boost-1.83.0.tar.xz
	echo "c5a0688e1f0c05f354bbd0b32244d36085d9ffc9f932e8a18983a9908096f614  $TXZ" | sha256sum -c - >/dev/null \
		|| { echo "boost-1.83.0.tar.xz: wrong SHA-256"; rm -f "$TXZ"; exit 1; }
	tar -xJf "$TXZ" -C "$BUILD"
	rm -rf "$BOOST" && mkdir -p "$BOOST"
	for d in "$BUILD"/boost-1.83.0/libs/*/include "$BUILD"/boost-1.83.0/libs/*/*/include; do
		[ -d "$d/boost" ] && cp -R "$d/boost" "$BOOST/"
	done
	rm -rf "$BUILD/boost-1.83.0"
	[ -f "$BOOST/boost/version.hpp" ] || { echo "no boost/version.hpp in the merged headers"; exit 1; }
fi

make -f amiga/Makefile.amigaos STOVE="$STOVE" NDK="$NDK" SDL2KIT="$SDL2KIT" ZLIB="$BUILD/zlib" BOOST="$BOOST" -j4 \
	> "$OUT/build-output.txt" 2>&1 || { tail -n 60 "$OUT/build-output.txt"; exit 1; }
cp build/amigaos/AlephOne/AlephOne "$BUILD/AlephOne"
# The rest of the drawer as the package target made it.
(cd build/amigaos/AlephOne && tar --format=ustar --exclude=./AlephOne -cf "$BUILD/drawer.tar" .)

python3 - "$BUILD" "$STOVE/bin/m68k-amigaos-gcc" "AlephOne" <<'PY'
import hashlib, json, subprocess, sys
from pathlib import Path
build, cc, exe = Path(sys.argv[1]), sys.argv[2], sys.argv[3]
git = lambda *a: subprocess.run(["git", *a], capture_output=True, text=True).stdout.strip()
receipt = {
    "executable_sha256": hashlib.sha256((build / exe).read_bytes()).hexdigest(),
    "source_commit": git("rev-parse", "HEAD"),
    "compiler_release": subprocess.run([cc, "--version"], capture_output=True,
                                       text=True).stdout.splitlines()[0],
    "source_date_epoch": int(git("log", "-1", "--format=%ct") or 0),
    "boost": "1.83.0",
}
(build / "receipt.json").write_text(json.dumps(receipt, indent=2) + "\n")
PY
cp "$BUILD/receipt.json" "$OUT/receipt.json"
python3 "$ROOT/amiga/tools/make_package.py" --build "$BUILD" --version "$VERSION" --out "$OUT"
