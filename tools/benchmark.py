"""Repeat the required 720p scenes; run after closing other GPU-heavy applications."""
import csv,json,subprocess,statistics
from pathlib import Path
root=Path(__file__).resolve().parents[1];out=root/'docs/evidence';out.mkdir(exist_ok=True)
runs=[]
for name,args in [('Ready',[]),('Ignition and ascent',['--launch','--start-time','-3']),('Separation and stars',['--launch','--start-time','24'])]:
 stem=name.lower().replace(' ','_');csvpath=out/(stem+'.csv')
 command=[str(root/'build/baikonur'),'--duration','12','--width','1280','--height','720','--no-retina','--no-vsync','--csv',str(csvpath)]+args
 result=subprocess.run(command,cwd=root,text=True,capture_output=True,timeout=40)
 (out/(stem+'.log')).write_text('Command: '+' '.join(command)+'\n'+result.stdout+result.stderr)
 if result.returncode:raise RuntimeError(result.stdout+result.stderr)
 rows=list(csv.DictReader(csvpath.open()));times=sorted(float(r['frame_ms']) for r in rows)
 runs.append({'scene':name,'mean_fps':1000*len(times)/sum(times),'p95_ms':times[int(len(times)*.95)],'min_fps':1000/max(times),'max_particles':max(int(r['particles']) for r in rows),'samples':len(times),'command':command})
 print(runs[-1],flush=True)
(out/'metrics.json').write_text(json.dumps({'hardware':'Apple M1, 8 GiB unified RAM, macOS 26.3.1','resolution':'1280x720','vsync':False,'warmup_seconds':2,'duration_seconds':12,'runs':runs},indent=2))
