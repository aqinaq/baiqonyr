import fs from 'node:fs';
import path from 'node:path';
import os from 'node:os';
import {fileURLToPath} from 'node:url';
import {execFileSync} from 'node:child_process';
export const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '..');
export const run = (cmd, args, options={}) => execFileSync(cmd, args, {cwd:root, ...options});
export function temporary(fn) {
  const dir=fs.mkdtempSync(path.join(os.tmpdir(),'baikonur-'));
  try { return fn(dir); } finally { fs.rmSync(dir,{recursive:true,force:true}); }
}
export function copy(source,dest) {
  fs.mkdirSync(path.dirname(dest),{recursive:true});
  fs.cpSync(source,dest,{recursive:true,filter:p=>!['.git','.DS_Store','__pycache__'].includes(path.basename(p)) && !/\.py[co]?$/.test(p)});
}
export function archive(dir,dest) {
  fs.mkdirSync(path.dirname(dest),{recursive:true});
  const pending=dest+'.pending.zip';
  fs.rmSync(pending,{force:true});
  run('zip',['-q','-r',pending,'.'],{cwd:dir});
  run('unzip',['-tq',pending]);
  fs.renameSync(pending,dest);
}
export function option(name,fallback) {
  const i=process.argv.indexOf(name);
  if(i<0)return fallback;
  if(!process.argv[i+1] || process.argv[i+1].startsWith('--'))throw Error(`Missing value for ${name}`);
  return process.argv[i+1];
}
