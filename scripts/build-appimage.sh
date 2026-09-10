#!/usr/bin/env bash
set -euo pipefail
project_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
seed="${1:-$project_dir/QTCopyHistory-x86_64.AppImage}"
seed="$(realpath "$seed")"
work="$(mktemp -d "$project_dir/build/appimage.XXXXXX")"
trap 'rm -rf -- "$work"' EXIT
qt_prefix="${QT_PREFIX:-/home/wanglong/Qt/6.11.0/gcc_64}"
cmake -S "$project_dir" -B "$project_dir/build/release" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$qt_prefix" \
    -DCMAKE_SKIP_RPATH=ON -DBUILD_TESTING=OFF
cmake --build "$project_dir/build/release" --parallel 4
# Reuse the existing AppImage runtime, desktop icon and Qt deployment.
# Fail instead of mixing the bundled Qt with a different SDK.
(cd "$work" && "$seed" --appimage-extract > /dev/null)
appdir="$work/squashfs-root"
for lib in "$appdir"/usr/lib/libQt6*.so.*.*.*; do
    cmp "$lib" "$qt_prefix/lib/$(basename "$lib")" || {
        echo 'Bundled Qt differs from QT_PREFIX; a fresh Qt deployment is required.' >&2
        exit 1
    }
done
cp "$project_dir/build/release/QTCopyHistory" "$appdir/usr/bin/QTCopyHistory"
strip "$appdir/usr/bin/QTCopyHistory"
# Include all Wayland integration plugins from the same SDK.
for plugin in wayland-decoration-client wayland-graphics-integration-client wayland-shell-integration; do
    if [[ -d "$qt_prefix/plugins/$plugin" ]]; then
        cp -a "$qt_prefix/plugins/$plugin" "$appdir/usr/plugins/"
    fi
done
mkdir -p "$appdir/usr/share/doc/QTCopyHistory"
cp "$project_dir/README.md" "$appdir/usr/share/doc/QTCopyHistory/"
cat > "$appdir/AppRun" <<'RUN'
#!/bin/sh
HERE="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
export LD_LIBRARY_PATH="$HERE/usr/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export QT_PLUGIN_PATH="$HERE/usr/plugins"
export QT_QPA_PLATFORM_PLUGIN_PATH="$HERE/usr/plugins/platforms"
exec "$HERE/usr/bin/QTCopyHistory" "$@"
RUN
chmod +x "$appdir/AppRun"
# Runtime and filesystem are separate parts of a type-2 AppImage.
offset="$("$seed" --appimage-offset)"
[[ "$offset" =~ ^[0-9]+$ ]]
mksquashfs "$appdir" "$work/payload.squashfs" -noappend -comp xz -all-root -no-progress > /dev/null
mkdir -p "$project_dir/dist"
out="$project_dir/dist/QTCopyHistory-x86_64.AppImage"
head -c "$offset" "$seed" > "$work/output.AppImage"
cat "$work/payload.squashfs" >> "$work/output.AppImage"
chmod +x "$work/output.AppImage"
mv "$work/output.AppImage" "$out"
(cd "$project_dir/dist" && sha256sum QTCopyHistory-x86_64.AppImage > SHA256SUMS)
echo "$out"
