(() => {
  'use strict';
  const params = new URLSearchParams(location.search);
  const wanted = params.get('file') || '';
  const match = (window.COURSE_DATA || []).flatMap(l => l.files.map(file => ({lecture:l,file}))).find(({file}) => file.url === wanted);
  const body = document.getElementById('viewer-body');
  const escapeHTML = input => String(input).replace(/[&<>"']/g, c => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
  let raw = '';
  if (!match){
    document.getElementById('viewer-title').textContent = 'Resource not found';
    body.innerHTML = '<p class="preview-error">This resource is not in the public course library.</p>';
    document.getElementById('viewer-raw').hidden = true;
    document.getElementById('viewer-download').hidden = true;
    return;
  }
  const {file,lecture} = match;
  document.title = `${file.name} · SE618 Lecture ${lecture.num}`;
  document.getElementById('viewer-kind').textContent = file.kind;
  document.getElementById('viewer-title').textContent = file.name;
  document.getElementById('viewer-desc').textContent = file.description || `Resource for Lecture ${lecture.num}: ${lecture.title}`;
  document.getElementById('viewer-filename').textContent = file.name;
  document.getElementById('viewer-breadcrumb').innerHTML = `<a href="index.html">Course library</a><span>/</span><a href="index.html#lecture-${String(lecture.num).padStart(2,'0')}">Lecture ${lecture.num}</a><span>/</span><span>${escapeHTML(file.name)}</span>`;
  document.getElementById('viewer-raw').href = encodeURI(file.url);
  document.getElementById('viewer-download').href = encodeURI(file.url);
  document.getElementById('viewer-back').href = `index.html#lecture-${String(lecture.num).padStart(2,'0')}`;

  const inline = src => {
    let s = escapeHTML(src);
    const codes = [];
    s = s.replace(/`([^`]+)`/g, (_, v) => {codes.push(`<code>${v}</code>`);return `\u0001${codes.length-1}\u0002`;});
    s=s.replace(/\*\*([^*]+)\*\*/g,'<strong>$1</strong>').replace(/\*([^*]+)\*/g,'<em>$1</em>');
    // Allow only web hyperlinks and same-page anchors, never javascript: schemes.
    s=s.replace(/\[([^\]]+)\]\(([^)]+)\)/g, (_m,label,target) => /^(https?:\/\/|#)/i.test(target) ? `<a href="${escapeHTML(target)}" target="_blank" rel="noopener noreferrer">${label}</a>` : label);
    return s.replace(/\u0001(\d+)\u0002/g,(_,n)=>codes[Number(n)]);
  };
  function renderMarkdown(source){
    const lines=source.replace(/\r\n/g,'\n').split('\n');const out=[];
    let inCode=false,code=[],list=null,table=[];
    const closeList=()=>{if(list){out.push(`</${list}>`);list=null;}};
    const closeTable=()=>{if(!table.length)return;const rows=table.filter(l=>!/^\s*\|?\s*:?-+:?\s*\|/.test(l));out.push('<div class="table-scroll"><table>'+rows.map((row,j)=>'<tr>'+row.trim().replace(/^\||\|$/g,'').split('|').map(cell=>`<${j===0?'th':'td'}>${inline(cell.trim())}</${j===0?'th':'td'}>`).join('')+'</tr>').join('')+'</table></div>');table=[];};
    const flush=()=>{closeList();closeTable();};
    for(const line of lines){
      if(/^\s*```/.test(line)){if(inCode){out.push(`<pre><code>${escapeHTML(code.join('\n'))}</code></pre>`);code=[];inCode=false;}else{flush();inCode=true;}continue;}
      if(inCode){code.push(line);continue;}
      if(/^\s*\|.*\|\s*$/.test(line)){closeList();table.push(line);continue;}else closeTable();
      if(!line.trim()){closeList();continue;}
      const heading=line.match(/^(#{1,3})\s+(.+)$/);
      if(heading){closeList();const level=heading[1].length;out.push(`<h${level}>${inline(heading[2])}</h${level}>`);continue;}
      const ul=line.match(/^\s*[-*+]\s+(.+)$/),ol=line.match(/^\s*\d+\.\s+(.+)$/);
      if(ul||ol){const type=ul?'ul':'ol';if(list!==type){closeList();out.push(`<${type}>`);list=type;}out.push(`<li>${inline((ul||ol)[1])}</li>`);continue;}
      closeList();
      if(/^>\s*/.test(line)){out.push(`<blockquote>${inline(line.replace(/^>\s*/,''))}</blockquote>`);continue;}
      if(/^\s*---+\s*$/.test(line)){out.push('<hr>');continue;}
      out.push(`<p>${inline(line)}</p>`);
    }
    flush();if(inCode)out.push(`<pre><code>${escapeHTML(code.join('\n'))}</code></pre>`);
    return out.join('\n');
  }
  fetch(encodeURI(file.url)).then(response=>{if(!response.ok)throw new Error(`HTTP ${response.status}`);return response.text();}).then(source=>{
    raw=source;
    if(file.kind==='MD')body.innerHTML=`<article class="markdown-preview">${renderMarkdown(source)}</article>`;
    else{const pre=document.createElement('pre');pre.className='code-preview';pre.textContent=source;body.replaceChildren(pre);}
  }).catch(()=>{body.innerHTML='<p class="preview-error">Could not load this file preview. Open the raw file or download it instead.</p>';});
  const copyButton=document.getElementById('copy-source');
  copyButton.addEventListener('click',async()=>{
    try{await navigator.clipboard.writeText(raw);copyButton.textContent='Copied!';setTimeout(()=>{copyButton.textContent='Copy contents';},1800);}catch(e){copyButton.textContent='Copy unavailable';}
  });
})();
