"""Package only project source and final deliverables; exclude build/cache/history."""
from pathlib import Path
import hashlib
import zipfile

root = Path(__file__).resolve().parents[1]
out = root / 'output'
source = out / 'Baikonur_Source.zip'
roots = ['CMakeLists.txt', 'README.md', 'CREDITS.md', '.gitignore', '.clang-format',
         'run.command', 'src', 'assets', 'shaders', 'tests', 'vendor', 'tools', 'docs']
with zipfile.ZipFile(source, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as z:
    for name in roots:
        target = root / name
        files = sorted(target.rglob('*')) if target.is_dir() else [target]
        for file in files:
            if file.is_file() and not any(p in {'.git', '__pycache__', '.DS_Store'} for p in file.parts):
                z.write(file, Path('Baikonur_Source') / file.relative_to(root))
    z.writestr('Baikonur_Source/output/.gitkeep', '')
with zipfile.ZipFile(source) as z:
    assert z.testzip() is None
    assert 'Baikonur_Source/vendor/glfw/deps/glad/gl.h' in z.namelist()
    assert 'Baikonur_Source/assets/fonts/Roboto-Medium.ttf' in z.namelist()
    assert 'Baikonur_Source/assets/fonts/LICENSE-Roboto.txt' in z.namelist()
    assert not any('/build/' in n or '/tmp/' in n or '/.git/' in n for n in z.namelist())

files = [source, out / 'Baikonur_Report.docx', out / 'pdf/Baikonur_Report.pdf',
         out / 'Baikonur_Presentation.pptx', out / 'Baikonur_Demo_1080p.mp4']
manifest = '\n'.join(hashlib.sha256(f.read_bytes()).hexdigest() + '  ' + f.name for f in files) + '\n'
(out / 'SHA256SUMS.txt').write_text(manifest)
with zipfile.ZipFile(out / 'Baikonur_Submission_Package.zip', 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as z:
    for file in files + [out / 'START_HERE.txt', out / 'SHA256SUMS.txt']:
        z.write(file, file.name)
    z.write(root / 'docs/REQUIREMENTS_CHECKLIST.md', 'REQUIREMENTS_CHECKLIST.md')
with zipfile.ZipFile(out / 'Baikonur_Submission_Package.zip') as z:
    assert z.testzip() is None
print('Verified source ZIP and complete deliverables ZIP.')
