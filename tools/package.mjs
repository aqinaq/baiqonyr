import fs from 'node:fs';
import path from 'node:path';
import {createHash} from 'node:crypto';
import {root,copy,archive,temporary,run} from './common.mjs';
const out=path.join(root,'output'),source=path.join(out,'Baikonur_Source.zip');
const roots=['CMakeLists.txt','README.md','CREDITS.md','.gitignore','.clang-format','run.command','.github','src','assets','shaders','tests','vendor','tools','docs'];
temporary(dir=>{
  const prefix=path.join(dir,'Baikonur_Source');
  for(const name of roots)copy(path.join(root,name),path.join(prefix,name));
  fs.mkdirSync(path.join(prefix,'output'));fs.writeFileSync(path.join(prefix,'output/.gitkeep'),'');
  archive(dir,source);
  const entries=run('unzip',['-Z1',source],{encoding:'utf8'}).trim().split('\n');
  for(const name of entries.filter(n=>!n.endsWith('/')&&n!=='Baikonur_Source/output/.gitkeep')){
    const actual=run('unzip',['-p',source,name],{maxBuffer:64*1024*1024});
    if(!actual.equals(fs.readFileSync(path.join(root,name.slice('Baikonur_Source/'.length)))))throw Error(`Stale archive member: ${name}`);
  }
});
if(!process.argv.includes('--source-only')){
  const files=[source,...['Baikonur_Report.docx','pdf/Baikonur_Report.pdf','Baikonur_Presentation.pptx','Baikonur_Demo_1080p.mp4'].map(n=>path.join(out,n))];
  const backup=path.join(out,'Baikonur_macOS_AppleSilicon.zip');if(fs.existsSync(backup))files.push(backup);
  fs.writeFileSync(path.join(out,'SHA256SUMS.txt'),files.map(f=>createHash('sha256').update(fs.readFileSync(f)).digest('hex')+'  '+path.basename(f)).join('\n')+'\n');
  temporary(dir=>{
    for(const file of [...files,path.join(out,'START_HERE.txt'),path.join(out,'SHA256SUMS.txt'),path.join(root,'docs/REQUIREMENTS_CHECKLIST.md')])copy(file,path.join(dir,path.basename(file)));
    archive(dir,path.join(out,'Baikonur_Submission_Package.zip'));
  });
}
console.log('Verified source archive'+(process.argv.includes('--source-only')?'': ' and submission package')+'.');
