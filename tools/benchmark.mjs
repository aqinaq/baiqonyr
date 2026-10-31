import fs from 'node:fs';
import path from 'node:path';
import os from 'node:os';
import {spawnSync} from 'node:child_process';
import {root,option} from './common.mjs';
export function summarize(csv){
  const [header,...lines]=csv.trim().split(/\r?\n/).map(s=>s.split(','));
  const frame=header.indexOf('frame_ms'),particle=header.indexOf('particles');
  if(frame<0||particle<0||!lines.length)throw Error('CSV needs frame_ms, particles and samples');
  const times=lines.map(r=>Number(r[frame])).sort((a,b)=>a-b), particles=lines.map(r=>Number(r[particle]));
  if(times.some(n=>!Number.isFinite(n)||n<=0)||particles.some(n=>!Number.isFinite(n)))throw Error('Invalid CSV sample');
  return {mean_fps:1000*times.length/times.reduce((a,b)=>a+b,0),p95_ms:times[Math.floor(times.length*.95)],min_fps:1000/times.at(-1),max_particles:Math.max(...particles),samples:times.length};
}
if(process.argv[1]&&path.resolve(process.argv[1])===path.join(root,'tools/benchmark.mjs')){
  const out=path.resolve(option('--output-dir',path.join(root,'docs/evidence')));
  fs.mkdirSync(out,{recursive:true});const runs=[];
  for(const [name,args] of [['Ready',[]],['Ignition and ascent',['--launch','--start-time','-3']],['Separation and stars',['--launch','--start-time','24']]]){
    const stem=name.toLowerCase().replaceAll(' ','_'),csvpath=path.join(out,stem+'.csv');
    const command=[path.resolve(option('--executable',path.join(root,'build/baikonur'))),'--duration','12','--width','1280','--height','720','--no-retina','--no-vsync','--csv',csvpath,...args];
    const result=spawnSync(command[0],command.slice(1),{cwd:root,encoding:'utf8',timeout:40000});
    fs.writeFileSync(path.join(out,stem+'.log'),'Command: '+command.join(' ')+'\n'+(result.stdout??'')+(result.stderr??''));
    if(result.error||result.status!==0)throw result.error??Error(`Benchmark failed: ${result.stderr}`);
    runs.push({scene:name,...summarize(fs.readFileSync(csvpath,'utf8')),command});console.log(runs.at(-1));
  }
  fs.writeFileSync(path.join(out,'metrics.json'),JSON.stringify({hardware:`${os.cpus()[0]?.model}, ${(os.totalmem()/2**30).toFixed(1)} GiB RAM, ${os.platform()} ${os.release()}`,resolution:'1280x720',vsync:false,warmup_seconds:2,duration_seconds:12,runs},null,2)+'\n');
}
