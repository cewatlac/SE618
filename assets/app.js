(() => {
  'use strict';
  const lectures = window.COURSE_DATA || [];
  const grid = document.getElementById('lecture-grid');
  const input = document.getElementById('lecture-search');
  const results = document.getElementById('results-context');
  const count = document.getElementById('result-count');
  const empty = document.getElementById('empty-results');
  const buttons = [...document.querySelectorAll('.filter-button')];
  const escapeHTML = value => String(value ?? '').replace(/[&<>"']/g, ch => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[ch]));
  let group = 'All';

  const extLabel = kind => ({CPP:'C++',HPP:'HEADER',MD:'GUIDE',TXT:'TEXT'}[kind] || kind);
  function fileRow(file){
    const url = encodeURI(file.url);
    const view = `viewer.html?file=${encodeURIComponent(file.url)}`;
    return `<li class="file-row"><span class="file-icon file-${escapeHTML(file.kind.toLowerCase())}">${escapeHTML(extLabel(file.kind))}</span><span class="file-name-wrap"><a class="file-name" href="${view}">${escapeHTML(file.name)}</a>${file.description ? `<span class="file-description">${escapeHTML(file.description)}</span>` : ''}</span><span class="file-links"><a href="${view}" aria-label="Preview ${escapeHTML(file.name)}">Preview ↗</a><a href="${url}" download aria-label="Download ${escapeHTML(file.name)}">↓</a></span></li>`;
  }
  function createCard(l){
    const resources = l.files.length ? `<details class="resources-panel"><summary><span>View student resources <b>${l.files.length}</b></span><span class="accordion-symbol">+</span></summary><div class="resources-inner"><ul class="file-list">${l.files.map(fileRow).join('')}</ul><a class="download-pack" href="${encodeURI(l.zip)}" download>Download Lecture ${String(l.num).padStart(2,'0')} resources (ZIP) <span>↓</span></a></div></details>` : `<div class="resources-none">Presentation and assessment briefing only</div>`;
    return `<article id="lecture-${String(l.num).padStart(2,'0')}" class="lecture-card" data-number="${l.num}"><div class="card-intro"><div class="card-number">LECTURE <strong>${String(l.num).padStart(2,'0')}</strong></div><span class="card-category">${escapeHTML(l.tag)}</span></div><h3>${escapeHTML(l.title)}</h3><p class="card-summary">${escapeHTML(l.summary)}</p><div class="card-meta"><span><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" aria-hidden="true"><rect x="3" y="4" width="18" height="17" rx="2"/><path d="M7 2v4M17 2v4M3 9h18"/></svg>${escapeHTML(l.dateLabel)}</span><span><svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.8" aria-hidden="true"><rect x="3" y="3" width="18" height="18" rx="2"/><path d="M7 8h10M7 12h8M7 16h10"/></svg>${l.slides} slides</span><span>${l.files.length} files</span></div><a class="open-lecture" href="${l.folder}/index.html"><span>Open interactive slides</span><span class="open-arrow" aria-hidden="true">↗</span></a>${resources}</article>`;
  }
  function render(){
    const q = input.value.toLocaleLowerCase().trim();
    const active = lectures.filter(l => (group === 'All' || (group === 'Core structures' && (l.group === 'Core structures' || l.group === 'Foundations')) || l.group === group) && (!q || [l.num,l.title,l.tag,l.group,l.summary,...l.files.flatMap(f => [f.name,f.description])].join(' ').toLocaleLowerCase().includes(q)));
    grid.innerHTML = active.map(createCard).join('');
    count.textContent = active.length;
    empty.hidden = active.length !== 0;
    results.textContent = active.length === lectures.length && !q && group === 'All' ? `Showing all ${lectures.length} lectures in course order` : `Showing ${active.length} of ${lectures.length} lectures${q ? ` matching “${input.value.trim()}”` : ''}`;
  }
  buttons.forEach(btn => btn.addEventListener('click', () => {
    group = btn.dataset.group;
    buttons.forEach(b => {const selected = b === btn;b.classList.toggle('is-active',selected);b.setAttribute('aria-pressed', String(selected));});
    render();
  }));
  input.addEventListener('input', render);
  document.getElementById('reset-search').addEventListener('click',()=>{input.value='';buttons[0].click();input.focus();});
  document.addEventListener('keydown', e => {if(e.key === '/' && !['INPUT','TEXTAREA'].includes(document.activeElement.tagName) && !e.ctrlKey && !e.metaKey){e.preventDefault();input.focus();}});
  render();
  const hash=location.hash.match(/^#lecture-(\d{2})$/);
  if (hash) {
    const target=document.getElementById('lecture-'+hash[1]);
    if (target) {target.scrollIntoView();const detail=target.querySelector('details');if(detail)detail.open=true;}
  }
})();
