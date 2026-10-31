// Compose WordprocessingML directly, preserving the supplied template's other parts.
import fs from 'node:fs';
import path from 'node:path';
import {root,run,temporary,archive,option} from './common.mjs';
const template=path.resolve(option('--template',path.join(root,'docs/templates/report.docx')));
const output=path.resolve(option('--output',path.join(root,'output/Baikonur_Report.docx')));
const date=option('--submission-date','To be confirmed');
const blocks=JSON.parse(fs.readFileSync(path.join(root,'docs/report-content.json'),'utf8'));
const metrics=JSON.parse(fs.readFileSync(path.join(root,'docs/evidence/metrics.json'),'utf8'));
const refs=JSON.parse(fs.readFileSync(path.join(root,'docs/references.json'),'utf8'));
const escape=s=>String(s).replaceAll('&','&amp;').replaceAll('<','&lt;').replaceAll('>','&gt;').replaceAll('"','&quot;');
const textRun=(s,bold=false)=>`<w:r><w:rPr><w:rFonts w:ascii="Times New Roman" w:hAnsi="Times New Roman"/><w:sz w:val="24"/>${bold?'<w:b/>':''}</w:rPr>${String(s).split('\n').map(t=>`<w:t xml:space="preserve">${escape(t)}</w:t>`).join('<w:br/>')}</w:r>`;
const para=(text='',bold=false,center=false,keep=false)=>`<w:p><w:pPr><w:spacing w:after="160" w:line="278" w:lineRule="auto"/>${center?'<w:jc w:val="center"/>':''}${keep?'<w:keepNext/>':''}</w:pPr>${textRun(text,bold)}</w:p>`;
function table(headers,rows,widths=headers.map(()=>6.5/headers.length)){
 const widthsTwip=widths.map(w=>Math.round(w*1440));
 return '<w:tbl><w:tblPr><w:tblLayout w:type="fixed"/><w:tblBorders>'+['top','left','bottom','right','insideH','insideV'].map(s=>`<w:${s} w:val="single" w:sz="4" w:color="888888"/>`).join('')+'</w:tblBorders></w:tblPr><w:tblGrid>'+widthsTwip.map(w=>`<w:gridCol w:w="${w}"/>`).join('')+'</w:tblGrid>'+[headers,...rows].map((row,i)=>'<w:tr><w:trPr><w:cantSplit/>'+(i===0?'<w:tblHeader/>':'')+'</w:trPr>'+row.map((v,j)=>`<w:tc><w:tcPr><w:tcW w:w="${widthsTwip[j]}" w:type="dxa"/></w:tcPr>${para(v,i===0)}</w:tc>`).join('')+'</w:tr>').join('')+'</w:tbl>'+para();
}
function equation(text){
 const nodes=[];const run=s=>`<m:r><m:t>${escape(s)}</m:t></m:r>`;
 for(const token of text.split(/([A-Za-zΣ]+_[A-Za-z]+|\^[A-Za-z0-9]+)/)){
  if(token.includes('_')){const [a,b]=token.split('_');nodes.push(`<m:sSub><m:e>${run(a)}</m:e><m:sub>${run(b)}</m:sub></m:sSub>`);}
  else if(token.startsWith('^'))nodes.push(`<m:sSup><m:e>${nodes.pop()??''}</m:e><m:sup>${run(token.slice(1))}</m:sup></m:sSup>`);
  else if(token)nodes.push(run(token));
 }
 return `<w:p><w:pPr><w:jc w:val="center"/></w:pPr><m:oMath>${nodes.join('')}</m:oMath></w:p>`;
}
// Split only direct body children, retaining template XML verbatim.
function children(xml){
 const result=[];let depth=0,start=0;
 for(const m of xml.matchAll(/<[^>]+>/g)){
  const tag=m[0];if(tag.startsWith('<?')||tag.startsWith('<!'))continue;
  if(tag.startsWith('</')){depth--;if(depth===0)result.push(xml.slice(start,m.index+tag.length));}
  else {if(depth===0)start=m.index;if(!tag.endsWith('/>'))depth++;else if(depth===0)result.push(tag);}
 }
 return result;
}
temporary(dir=>{
 run('unzip',['-q',template,'-d',dir]);
 const document=path.join(dir,'word/document.xml'),relfile=path.join(dir,'word/_rels/document.xml.rels');
 let xml=fs.readFileSync(document,'utf8'),rels=fs.readFileSync(relfile,'utf8');
 const body=xml.match(/<w:body>([\s\S]*?)<\/w:body>/);if(!body)throw Error('Template has no document body');
 const nodes=children(body[1]);let cover=nodes.slice(0,13).filter(n=>!n.startsWith('<w:sectPr')).join('');
 for(const [placeholder,value] of [['[Insert Project Title]','Baikonur Rocket Launch'],['[Insert Names and group]','Akbope Bakytkeldy, SE2426\nMoldir Omar, SE2426\nAyaulym Talgat, SE2427\nTalant Altaikhan, SE2425'],['[Insert Date]',date]]){
  if(!cover.includes(placeholder))throw Error(`Template missing ${placeholder}`);
  cover=cover.replace(placeholder,escape(value).replaceAll('\n','</w:t><w:br/><w:t>'));
 }
 const section=nodes.find(n=>n.startsWith('<w:sectPr'))??'';let content=cover,imageIndex=0;
 for(const block of blocks){
  const a=block.args??[];
  switch(block.type){
   case 'p':content+=para(...a);break;
   case 'new':content+='<w:p><w:r><w:br w:type="page"/></w:r></w:p>'+para(a[0],true,false,true);break;
   case 'sub':content+=para(a[0],true,false,true);break;
   case 'eq':content+=equation(a[0]);break;
   case 'table':content+=table(...a);break;
   case 'metrics':content+=table(['Scene','Mean FPS','p95 ms','Max particles'],metrics.runs.map(r=>[r.scene,r.mean_fps.toFixed(1),r.p95_ms.toFixed(2),r.max_particles]),[2.4,1.3,1.2,1.6]);break;
   case 'references':for(const r of refs.slice(block.start,block.end??undefined))content+=para(`[${r.id}] ${r.authors}, "${r.title}," ${r.venue}, vol. ${r.volume}, no. ${r.issue}, ${String(r.pages).includes('Article')?'':'pp. '}${r.pages}, ${r.year}. https://doi.org/${r.doi}`);break;
   case 'fig':{
    const [file,caption,width=6.35]=a,data=fs.readFileSync(path.join(root,'docs/evidence',file));
    if(data.subarray(1,4).toString()!=='PNG')throw Error('Report figures must be PNG');
    const cx=Math.round(width*914400),cy=Math.round(cx*data.readUInt32BE(20)/data.readUInt32BE(16));
    const id=`rIdReportFigure${++imageIndex}`,name=`report-figure-${imageIndex}.png`;
    fs.mkdirSync(path.join(dir,'word/media'),{recursive:true});fs.writeFileSync(path.join(dir,'word/media',name),data);
    rels=rels.replace('</Relationships>',`<Relationship Id="${id}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/image" Target="media/${name}"/></Relationships>`);
    content+=`<w:p><w:pPr><w:jc w:val="center"/><w:keepNext/></w:pPr><w:r><w:drawing><wp:inline xmlns:wp="http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"><wp:extent cx="${cx}" cy="${cy}"/><wp:docPr id="${1000+imageIndex}" name="${name}"/><a:graphic xmlns:a="http://schemas.openxmlformats.org/drawingml/2006/main"><a:graphicData uri="http://schemas.openxmlformats.org/drawingml/2006/picture"><pic:pic xmlns:pic="http://schemas.openxmlformats.org/drawingml/2006/picture"><pic:nvPicPr><pic:cNvPr id="0" name="${name}"/><pic:cNvPicPr/></pic:nvPicPr><pic:blipFill><a:blip r:embed="${id}"/><a:stretch><a:fillRect/></a:stretch></pic:blipFill><pic:spPr><a:xfrm><a:off x="0" y="0"/><a:ext cx="${cx}" cy="${cy}"/></a:xfrm><a:prstGeom prst="rect"><a:avLst/></a:prstGeom></pic:spPr></pic:pic></a:graphicData></a:graphic></wp:inline></w:drawing></w:r></w:p>`+para(caption,false,true);break;
   }
   default:throw Error(`Unknown report block ${block.type}`);
  }
 }
 xml=xml.replace(body[0],`<w:body>${content}${section}</w:body>`);
 fs.writeFileSync(document,xml);fs.writeFileSync(relfile,rels);
 const types=path.join(dir,'[Content_Types].xml');let typesXml=fs.readFileSync(types,'utf8');
 if(!typesXml.includes('Extension="png"'))typesXml=typesXml.replace('</Types>','<Default Extension="png" ContentType="image/png"/></Types>');
 fs.writeFileSync(types,typesXml);
 archive(dir,output);
});console.log(output);
