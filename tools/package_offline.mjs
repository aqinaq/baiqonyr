import fs from 'node:fs';
import path from 'node:path';
import {root,copy,archive,temporary,option} from './common.mjs';
const build=path.resolve(option('--build-dir',path.join(root,'build')));
const output=path.join(root,'output/Baikonur_macOS_AppleSilicon.zip');
if(!fs.existsSync(path.join(build,'baikonur')))throw Error('Build the application first.');
temporary(dir=>{
  const prefix=path.join(dir,'Baikonur_macOS_AppleSilicon');
  for(const name of ['baikonur','assets','shaders'])copy(path.join(build,name),path.join(prefix,name));
  for(const name of ['CREDITS.md','vendor/glfw/LICENSE.md','vendor/glm/copying.txt','vendor/imgui/LICENSE.txt'])copy(path.join(root,name),path.join(prefix,name));
  fs.chmodSync(path.join(prefix,'baikonur'),0o755);
  fs.writeFileSync(path.join(prefix,'Launch.command'),'#!/bin/sh\ncd "$(dirname "$0")" || exit 1\nexec ./baikonur "$@"\n',{mode:0o755});
  fs.writeFileSync(path.join(prefix,'README.txt'),'Offline backup for Apple Silicon macOS.\nKeep this folder intact. Double-click Launch.command.\nThis executable is unsigned; source submission is separate.\nSpace launches, P pauses, R resets, H shows controls, Esc exits.\n');
  archive(dir,output);
});console.log(output);
