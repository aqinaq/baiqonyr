"""Package a locally built macOS executable with its runtime assets for defense backup."""
from pathlib import Path
import argparse
import zipfile

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument('--build-dir', type=Path, default=root / 'build')
args = parser.parse_args()
build = args.build_dir.resolve()
assert (build / 'baikonur').is_file(), 'Build the application first.'
output = root / 'output/Baikonur_macOS_AppleSilicon.zip'
prefix = Path('Baikonur_macOS_AppleSilicon')
with zipfile.ZipFile(output, 'w', zipfile.ZIP_DEFLATED) as archive:
    archive.write(build / 'baikonur', prefix / 'baikonur')
    for directory in ['assets', 'shaders']:
        for file in sorted((build / directory).rglob('*')):
            if file.is_file():
                archive.write(file, prefix / file.relative_to(build))
    for name in ['CREDITS.md', 'vendor/glfw/LICENSE.md', 'vendor/glm/copying.txt',
                 'vendor/imgui/LICENSE.txt']:
        archive.write(root / name, prefix / name)
    launcher = zipfile.ZipInfo(str(prefix / 'Launch.command'))
    launcher.create_system = 3
    launcher.external_attr = 0o100755 << 16
    archive.writestr(launcher, '#!/bin/sh\ncd "$(dirname "$0")" || exit 1\nexec ./baikonur "$@"\n')
    archive.writestr(str(prefix / 'README.txt'),
        'Offline defense backup for Apple Silicon macOS only.\n'
        'Built and checked on Apple M1 / macOS 26.3.1; other systems are untested.\n'
        'Keep this folder intact. Double-click Launch.command or run ./baikonur.\n'
        'This local executable is unsigned and is not a replacement for the source submission.\n'
        'Press H for controls. Space launches, P pauses, R resets, Esc exits.\n'
        'The separate Baikonur_Demo_1080p.mp4 is the offline video backup.\n')
with zipfile.ZipFile(output) as archive:
    assert archive.testzip() is None
    assert archive.read(str(prefix / 'baikonur')) == (build / 'baikonur').read_bytes()
print(output)
